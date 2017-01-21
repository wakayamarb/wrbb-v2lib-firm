/*
 * Adafruit TFT 2.8'' Touchpanel Capacitive  Shield関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <sTFTc.h>

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 *ctp=0;		// ポインタのみ生成しておく

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 *tft=0;	// ポインタのみ生成しておく
int TFTc_setuped = 0;		// TFTが初期化されると1を設定
unsigned int lastx=0;		// 最後に描画したポイント
unsigned int lasty=0;		// 最後に描画したポイント
unsigned int lastc=0;		// 最後に描画した色
TS_Point TPoint;			// タッチされた座標変換後の座標を格納する構造体


/////////////////////
// add buttons begin
#define MAX_BUTTONS 32			// 各画面で描画することのできる最大ボタン数（これ以上は作れない）
volatile int Button_Max;		// 各画面を通して使用できる最大のボタン数(インスタンスを生成)
volatile int Cur_Button_Max;	// 現在の画面で使用する最大のボタン数
Adafruit_GFX_Button *buttons[MAX_BUTTONS];		// ポインタのみ生成
// add buttons end
/////////////////////

/////////////////////
// add bmpdraw begin
//

// 追加: #indlude <SD.h>
//
// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.
int bmpDraw(char *, uint8_t, uint16_t);
uint16_t read16(File &);
uint32_t read32(File &);
#define BUFFPIXEL 20

////////
// SDカード内にあるビットマップファイルを描画する
// 戻り値： 
//		0: 描画成功
//		1: 失敗 (ビットマップサイズが画面よりも大きい)
//		2: 失敗 (ファイルが存在しない)
//		3: 失敗（サポート外のファイル形式）
//
int bmpDraw(char *filename, uint8_t x, uint16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft->width()) || (y >= tft->height())) return 1;
#ifdef TFTC_DEBUG
  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');
#endif
  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
#ifdef TFTC_DEBUG
    Serial.print(F("File not found"));
#endif
    return 2;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
#ifdef TFTC_DEBUG
    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
#else
	read32(bmpFile);	// ? magic read
#endif
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
#ifdef TFTC_DEBUG
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
#else
	read32(bmpFile);	// ? magic read
#endif
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
#ifdef TFTC_DEBUG
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
#endif
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
#ifdef TFTC_DEBUG
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);
#endif
        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft->width())  w = tft->width()  - x;
        if((y+h-1) >= tft->height()) h = tft->height() - y;

        // Set TFT address window to clipped image bounds
        tft->setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft->pushColor(tft->color565(r,g,b));
          } // end pixel
        } // end scanline
#ifdef TFTC_DEBUG
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
#endif
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp){
#ifdef TFTC_DEBUG
    Serial.println(F("BMP format not recognized."));
#endif
    return 3;
  } else {
    return 0;  //描画成功
  }
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

//**************************************************
//	mRuby method :画面描画関数
//	void drawbmp(char *, uint8_t, uint16_t);
//	void drawChar(filename,  x,  y  )
//	ビットマップを、指定の座標に描画する。左上が原点。
//戻り値： 
//		0: 描画成功
//		1: 失敗 (ビットマップサイズが画面よりも大きい)
//		2: 失敗 (ファイルが存在しない)
//		3: 失敗（サポート外のファイル形式）
//**************************************************
mrb_value mrb_tftc_drawbmp(mrb_state *mrb, mrb_value self)
{
	mrb_value value;
	char *str;
	unsigned int x,y;
	int ret;

	int n = mrb_get_args(mrb,"Sii", &value, &x, &y );
	str = RSTRING_PTR(value);
	ret = bmpDraw( str, x, y );

	return mrb_fixnum_value( ret );
}
//	mrb_define_module_function(mrb, tftcModule, "drawBmp", mrb_tftc_drawbmp, MRB_ARGS_REQ(3));		//引数3

//
// add bmpdraw end
/////////////////////

/////////////////////
// add buttons begin
//

////////
// ボタンのインスタンスを生成する
int newButtons( int n )
{
	int created = 0;

	if(n>MAX_BUTTONS) n=MAX_BUTTONS;
	for(int i=0; i<n; i++){
		if(buttons[i]==0){
			buttons[i] = new Adafruit_GFX_Button;	// インスタンスを生成
			created++;
		}
	}
	return created;									// 確保した配列数を返す
}
//**************************************************
//	mRuby method :画面描画関数 newButtons
//	int newbuttons( int )
//	int newbuttons( num )
//**************************************************
mrb_value mrb_tftc_newbuttons(mrb_state *mrb, mrb_value self)
{
	int num,ret;
	
	mrb_get_args(mrb,"i",&num);
	ret = newButtons(num);
	return mrb_fixnum_value( ret );
}
//	mrb_define_module_function(mrb, tftcModule, "newButtons", mrb_tftc_newbuttons, MRB_ARGS_REQ(1));	//引数1

////////
// ボタンのインスタンスを消去する
int deleteButtons( int n )
{
	int deleted=0;
	
	if(n>MAX_BUTTONS) n=MAX_BUTTONS;
	for(int i=0; i<n; i++){
		if (buttons[i]!=0){
			delete(buttons[i]);						// インスタンスを削除
			buttons[i]=0;							// ポインタに0を代入
			deleted++;
		}
	}
	return deleted;									// 削除した配列数を返す
}
//**************************************************
//	mRuby method :画面描画関数 deleteButtons
//	int deletebuttons( void )
//	int deletebuttons(      )
//**************************************************
mrb_value mrb_tftc_deletebuttons(mrb_state *mrb, mrb_value self)
{
	int ret;
	//int num;
	//mrb_get_args(mrb,"i",&num);
	//ret = deleteButtons(num);
	ret = deleteButtons(Button_Max);
	return mrb_fixnum_value( ret );
}
//	mrb_define_module_function(mrb, tftcModule, "deleteButtons", mrb_tftc_deletebuttons, MRB_ARGS_REQ(1));	//引数1
//	mrb_define_module_function(mrb, tftcModule, "deleteButtons", mrb_tftc_deletebuttons, MRB_ARGS_NONE());	//引数0


////////
// 全ての画面で使用するボタンの最大数を定義（このボタンの数のインスタンスが生成される）
int setButtonMax( int n )
{
	Button_Max = n;
	if( Button_Max !=newButtons(Button_Max) ){		// newButtons()は、生成できたボタンの個数を返す。
		// MAX_BUTTONSを超えるボタンを生成しようとした場合はMAX_BUTTONS の値が返されるため trueとなる。
#ifdef TFTC_DEBUG
		Serial.print("OutOfRange;MAX_BUTTONS Button_Max=");		// Button_Max個のボタンのインスタンスを生成
		Serial.println(Button_Max);
#endif
		// while(1) ;
	}
	return Button_Max;
}
//**************************************************
//	mRuby method :画面描画関数 setButtonMax
//	int setbuttonmax( int )
//	int setbuttonmax( num )
//**************************************************
mrb_value mrb_tftc_setbuttonmax(mrb_state *mrb, mrb_value self)
{
	int num,ret;
	
	mrb_get_args(mrb,"i",&num);
	ret = setButtonMax(num);
	return mrb_fixnum_value( ret );
}
//	mrb_define_module_function(mrb, tftcModule, "setButtonMax", mrb_tftc_setbuttonmax, MRB_ARGS_REQ(1));	//引数1

////////
// 現在の画面で使用するボタンの最大数を定義
int setCurrentDisplayButtonMax( int n )
{
	Cur_Button_Max = n;
	if( Cur_Button_Max >Button_Max ){
		Cur_Button_Max = Button_Max;
#ifdef TFTC_DEBUG
		Serial.print("OutOfRange;Button_Max Cur_Button_Max=");		// Button_Max個のボタンが使用可能
		Serial.println(Cur_Button_Max);
#endif
		// while(1) ;
	}
	return Cur_Button_Max;
}
//**************************************************
//	mRuby method :画面描画関数 setDisplayButtonMax
//	int setcurrentdisplaybuttonmax( int )
//	int setcurrentdisplaybuttonmax( num )
//**************************************************
mrb_value mrb_tftc_setcurrentdisplaybuttonmax(mrb_state *mrb, mrb_value self)
{
	int num,ret;
	
	mrb_get_args(mrb,"i",&num);
	ret = setCurrentDisplayButtonMax(num);
	return mrb_fixnum_value( ret );
}
//	mrb_define_module_function(mrb, tftcModule, "setDisplayButtonMax", mrb_tftc_setcurrentdisplaybuttonmax, MRB_ARGS_REQ(1));	//引数1

////////
// 指定の座標にあるボタンを検索し、該当ボタン
// の押下ステータスを変更する。
// ボタンno 1～Cur_Buttonmax のボタンが検索対象になる。
// ボタンnoは、1～MAX_BUTTONSまで。
// 引数： タッチ検出された座標 (x,y)
// void searchTouchedButton( int n )
void searchTouchedButton( int x, int y )
{
	for (int j=0;j<Cur_Button_Max;j++){
		if(buttons[j]->contains(x,y)) {
			buttons[j]->press(true);
			//Serial.print("Inside of Button");Serial.println(j);
		} else {
			buttons[j]->press(false);
			//Serial.print("Out of Button");Serial.println(j);
		}
	}
}
//**************************************************
//	mRuby method :画面描画関数 searchTouchedButton
//	int searchtouchedbutton( int , int )
//	int searchtouchedbutton( x , y )
//**************************************************
mrb_value mrb_tftc_searchtouchedbutton(mrb_state *mrb, mrb_value self)
{
	int x,y;
	
	mrb_get_args(mrb,"ii",&x,&y);
	searchTouchedButton(x,y);
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "searchTouchedButton", mrb_tftc_searchtouchedbutton, MRB_ARGS_REQ(2));	//引数2

////////
// ボタンの状態が変化した場合に値を返す
// ボタンnoは、1～MAX_BUTTONSまで。（0スタートでないので注意)
// 押された直後：-1 、離した直後：1、変化なし：0
// ボタン状態がpressに変化した場合は該当ボタンの背景を反転する。
// ボタン状態がreleaseに変化した場合は、該当ボタンの背景をノーマルに戻す。
int searchButtonState( int n )
{
	if (n > Cur_Button_Max) return 0;
	if (buttons[n-1]->justReleased()) {
		//Serial.print("Released: "); Serial.println(n);
		buttons[n-1]->drawButton();  // draw normal
		return 1;		//Just Released
	}
	else if (buttons[n-1]->justPressed()) {
		buttons[n-1]->drawButton(true);  // draw invert!
		//Serial.print("Pressed: "); Serial.println(n);
		return -1;		//Just Pressed
	}
	else {
		return 0;		// 変化なし
	}
}
//**************************************************
//	mRuby method :画面描画関数 searchButtonState
//	int searchbuttonstate( int )
//	int searchbuttonstate( num )
//**************************************************
mrb_value mrb_tftc_searchbuttonstate(mrb_state *mrb, mrb_value self)
{
	int num,ret;
	
	mrb_get_args(mrb,"i",&num);
	ret = searchButtonState(num);
	return mrb_fixnum_value( ret );
}
//	mrb_define_module_function(mrb, tftcModule, "searchButtonState", mrb_tftc_searchbuttonstate, MRB_ARGS_REQ(1));	//引数1

////////
// 押下されたボタンを検出し、そのボタンnoを返す
// released → press 状態を検出
// ボタンnoは、1～MAX_BUTTONSまで。（0スタートでないので注意)
// 戻り値： 0: 押下されたボタンなし 1～MAX_BUTTONS : 押下されたボタンNO
// 
int searchPressedButton( void )
{
	int ret=0;

	for (int j=1;j<=Cur_Button_Max;j++){
		if ( ( -1 ) == searchButtonState( j )){
			ret=j;
		}
	}
	return ret;
}
//**************************************************
//	mRuby method :画面描画関数 searchPressedButton
//	int searcpressedbutton( void )
//	int searcpressedbutton(      )
//**************************************************
mrb_value mrb_tftc_searchpressedbutton(mrb_state *mrb, mrb_value self)
{
	int ret;
	
	ret = searchPressedButton();
	return mrb_fixnum_value( ret );
}
//	mrb_define_module_function(mrb, tftcModule, "searchPressedButton", mrb_tftc_searchpressedbutton, MRB_ARGS_NONE());	//引数0

////////
// リリースされたボタンを検出し、そのボタンnoを返す
// pressed → release 状態を検出
// ボタンnoは、1～MAX_BUTTONSまで。（0スタートでないので注意)
// 戻り値： 0: リリースされたボタンなし 1～MAX_BUTTONS : リリースされたボタンNO
// 
int searchReleasedButton( void )
{
	int ret=0;

	for (int j=1;j<=Cur_Button_Max;j++){
		if ( ( 1 ) == searchButtonState( j )){
			ret=j;
		}
	}
	return ret;
}
//**************************************************
//	mRuby method :画面描画関数 searchReleasedButton
//	int searcreleasedbutton( void )
//	int searcreleasedbutton(      )
//**************************************************
mrb_value mrb_tftc_searcreleasedbutton(mrb_state *mrb, mrb_value self)
{
	int ret;
	
	ret = searchReleasedButton();
	return mrb_fixnum_value( ret );
}
//	mrb_define_module_function(mrb, tftcModule, "searchReleasedButton", mrb_tftc_searcreleasedbutton, MRB_ARGS_NONE());	//引数0

////////
// ボタンを初期化する
// ボタンnoは、1～MAX_BUTTONSまで。（0スタートでないので注意)
// 予め確保したボタンnoより大きな値が指定された場合は、0を返す。
int initButton(int buttonNo,int x, int y, int w, int h, int edgec, int backc, int textc, char *text, int size)
{
	if(buttonNo>Cur_Button_Max){ return 0; }
	else {
		buttons[buttonNo-1]->initButton(
		    tft,
		    x,      //中心位置x
		    y,      //中心位置y
		    w,      //幅w
		    h,      //高さh
		    edgec,  //枠線色
		    backc,  //背景色
		    textc,  //文字色
		    text,   //文字列
		    size    //文字サイズ 1(最小)..
		);
		return 1;
	}
}
//**************************************************
//	mRuby method :画面描画関数 (テキスト)
//	int initbutton( int,int,int,int,int, int  ,  int  , int   , char *, int  );
//	int initbutton( no , x , y , w , h , edge , backc , textc , text  , size )
//**************************************************
mrb_value mrb_tftc_initbutton(mrb_state *mrb, mrb_value self)
{
	mrb_value value;
	char *text;
	int ret,no,x,y,w,h,edgec,backc,textc,size;

	mrb_get_args(mrb,"iiiiiiiiSi", &no, &x, &y, &w, &h, &edgec, &backc, &textc, &value, &size  );
	text = RSTRING_PTR(value);
	ret = initButton( no, x, y, w, h, edgec, backc, textc, text, size);
	return mrb_fixnum_value( ret );
}
//	mrb_define_module_function(mrb, tftcModule, "initButton", mrb_tftc_initbutton, MRB_ARGS_REQ(10));	//引数10


////////
// 現在の画面の全てのボタン（ボタン数 Cur_Button_Maxを描画する
void drawButtons( void ){
	for(int i=0;i<Cur_Button_Max;i++) buttons[i]->drawButton();	//ボタンを描画
}
//**************************************************
//	mRuby method :画面描画関数 drawButtons
//	void drawbuttons( void )
//**************************************************
mrb_value mrb_tftc_drawbuttons(mrb_state *mrb, mrb_value self)
{
	drawButtons();
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "drawButtons", mrb_tftc_drawbuttons, MRB_ARGS_NONE());	//引数0


////////
// ボタンを個別に描画する
// flag  0(=false) : 通常描画  1(=true) : 反転描画
void drawButton( int btno, int flag ){

	if (flag) buttons[btno-1]->drawButton(true);	//btnoボタンを、flagの状態で描画
	else      buttons[btno-1]->drawButton();
}
//**************************************************
//	mRuby method :画面描画関数 drawButton
//	void drawbutton( int , int )
//	void drawbutton( btno, flag)
//**************************************************
mrb_value mrb_tftc_drawbutton(mrb_state *mrb, mrb_value self)
{
	int btno,flag;
	mrb_get_args(mrb,"ii",&btno,&flag);
	drawButton(btno,flag);
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "drawButton", mrb_tftc_drawbutton, MRB_ARGS_REQ(2));	//引数2


// add buttons end
/////////////////////


//**************************************************
//	mRuby method :画面描画関数
//	void fillscreen(int)
//**************************************************
mrb_value mrb_tftc_fillscreen(mrb_state *mrb, mrb_value self)
{
	unsigned int color;
	mrb_get_args(mrb,"i",&color);
	tft->fillScreen(color);
	
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "fillScreen", mrb_tftc_fillscreen, MRB_ARGS_REQ(1));	//引数1


//**************************************************
//	mRuby method :画面描画関数
//	void rotation(int)
//**************************************************
mrb_value mrb_tftc_rotation(mrb_state *mrb, mrb_value self)
{
	unsigned int rotation;
	mrb_get_args(mrb,"i",&rotation);
	tft->setRotation( (uint8_t)(rotation) );
	
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "rotation", mrb_tftc_rotation, MRB_ARGS_REQ(1));	//引数1


//**************************************************
//	mRuby method :画面描画関数:
//	int width( void )   画面の幅を返す
//**************************************************
mrb_value mrb_tftc_width(mrb_state *mrb, mrb_value self)
{
	unsigned int value=0;
	value = tft->width();
	return mrb_fixnum_value( value );
}
//	mrb_define_module_function(mrb, tftcModule, "width", mrb_tftc_width, MRB_ARGS_NONE());


//**************************************************
//	mRuby method :画面描画関数:
//	int height( void )   画面の高さを返す
//**************************************************
mrb_value mrb_tftc_height(mrb_state *mrb, mrb_value self)
{
	unsigned int value=0;
	value = tft->height();
	return mrb_fixnum_value( value );
}
//	mrb_define_module_function(mrb, tftcModule, "height", mrb_tftc_height, MRB_ARGS_NONE());


//**************************************************
//	mRuby method :画面描画関数
//	int color(int)
//**************************************************
mrb_value mrb_tftc_selectcolor(mrb_state *mrb, mrb_value self)
{
	int colorcode=0;
	unsigned int color;
	mrb_get_args(mrb,"i",&colorcode);
	switch(colorcode) {
		case  0:	color= C_BLACK;	break;
		case  1:	color= C_BROWN;	break;
		case  2:	color= C_RED;	break;
		case  3:	color= C_ORANGE;break;
		case  4:	color= C_YELLOW;break;
		case  5:	color= C_LIME;	break;
		case  6:	color= C_BLUE;	break;
		case  7:	color= C_VIOLET;break;
		case  8:	color= C_GRAY;	break;
		case  9:	color= C_WHITE;	break;
		case 10:	color= C_CYAN;	break;
		case 11:	color= C_MAGENTA;break;
		default:	color= C_WHITE;	break;
	}

	return mrb_fixnum_value( color );
}
//	mrb_define_module_function(mrb, tftcModule, "color", mrb_tftc_selectcolor, MRB_ARGS_REQ(1));	//引数1


//**************************************************
//	mRuby method :画面描画関数
//	void drawpixel(int,int,(int))
//	void drawpixel( x , y ,(color))
//**************************************************
mrb_value mrb_tftc_drawpixel(mrb_state *mrb, mrb_value self)
{
	unsigned int x,y,color;
	int n = mrb_get_args(mrb,"ii|i",&x,&y,&color);
	if( n<3 ) {
		tft->drawPixel(x,y,lastc);
	} else {
		tft->drawPixel(x,y,color);
		lastc=color;
	}
	lastx=x; lasty=y;
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "drawPixel", mrb_tftc_drawpixel, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(1));	//引数3or2


//**************************************************
//	mRuby method :画面描画関数
//	void drawline(int,int,int,int,(int))
//	void drawline( x0, y0, x1, y1,(color))
//**************************************************
mrb_value mrb_tftc_drawline(mrb_state *mrb, mrb_value self)
{
	unsigned int x0,y0,x1,y1,color;
	int n = mrb_get_args(mrb,"iiii|i",&x0,&y0,&x1,&y1,&color);
	if( n<5 ) {
		tft->drawLine(x0,y0,x1,y1,lastc);
	} else {
		tft->drawLine(x0,y0,x1,y1,color);
		lastc=color;
	}
	lastx=x1; lasty=y1;
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "drawLine", mrb_tftc_drawline, MRB_ARGS_REQ(4)|MRB_ARGS_OPT(1));	//引数5or4


//**************************************************
//	mRuby method :画面描画関数
//	void drawlineto(int,int,(int))
//	void drawlineto( x , y ,(color))
//**************************************************
mrb_value mrb_tftc_drawlineto(mrb_state *mrb, mrb_value self)
{
	unsigned int x,y,color;
	
	int n = mrb_get_args(mrb,"ii|i",&x,&y,&color);
	if( n<3 ) {
		tft->drawLine(lastx,lasty,x,y,lastc);
	} else {
		tft->drawLine(lastx,lasty,x,y,color);
		lastc=color;
	}
	lastx=x; lasty=y;
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "drawLineto", mrb_tftc_drawlineto, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(1));	//引数3or2


//**************************************************
//	mRuby method :画面描画関数
//	void drawcircle(int,int,int,(int))
//	void drawcircle( x , y , r ,(color))
//**************************************************
mrb_value mrb_tftc_drawcircle(mrb_state *mrb, mrb_value self)
{
	unsigned int x,y,r,color;
	
	int n = mrb_get_args(mrb,"iii|i",&x,&y,&r,&color);
	if( n<4 ) {
		tft->drawCircle(x,y,r,lastc);
	} else {
		tft->drawCircle(x,y,r,color);
		lastc=color;
	}
	lastx=x; lasty=y;
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "drawCircle", mrb_tftc_drawcircle, MRB_ARGS_REQ(3)|MRB_ARGS_OPT(1));	//引数4or3


//**************************************************
//	mRuby method :画面描画関数
//	void fillcircle(int,int,int,(int))
//	void fillcircle( x , y , r ,(color))
//**************************************************
mrb_value mrb_tftc_fillcircle(mrb_state *mrb, mrb_value self)
{
	unsigned int x,y,r,color;
	
	int n = mrb_get_args(mrb,"iii|i",&x,&y,&r,&color);
	if( n<4 ) {
		tft->fillCircle(x,y,r,lastc);
	} else {
		tft->fillCircle(x,y,r,color);
		lastc=color;
	}
	lastx=x; lasty=y;
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "fillCircle", mrb_tftc_fillcircle, MRB_ARGS_REQ(3)|MRB_ARGS_OPT(1));	//引数4or3


//**************************************************
//	mRuby method :画面描画関数
//	void drawrect(int,int,int,int,(int))
//	void drawrect( x, y, w, h,(color))
//**************************************************
mrb_value mrb_tftc_drawrect(mrb_state *mrb, mrb_value self)
{
	unsigned int x,y,w,h,color;
	int n = mrb_get_args(mrb,"iiii|i",&x,&y,&w,&h,&color);
	if( n<5 ) {
		tft->drawRect(x,y,w,h,lastc);
	} else {
		tft->drawRect(x,y,w,h,color);
		lastc=color;
	}
	lastx=x; lasty=y;
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "drawRect", mrb_tftc_drawrect, MRB_ARGS_REQ(4)|MRB_ARGS_OPT(1));	//引数5or4


//**************************************************
//	mRuby method :画面描画関数
//	void fillrect(int,int,int,int,(int))
//	void fillrect( x , y , w , h ,(color))
//**************************************************
mrb_value mrb_tftc_fillrect(mrb_state *mrb, mrb_value self)
{
	unsigned int x,y,w,h,color;
	int n = mrb_get_args(mrb,"iiii|i",&x,&y,&w,&h,&color);
	if( n<5 ) {
		tft->fillRect(x,y,w,h,lastc);
	} else {
		tft->fillRect(x,y,w,h,color);
		lastc=color;
	}
	lastx=x; lasty=y;
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "fillRect", mrb_tftc_fillrect, MRB_ARGS_REQ(4)|MRB_ARGS_OPT(1));	//引数5or4

//**************************************************
//	mRuby method :画面描画関数
//	void drawroundrect(int,int,int,int,int,(int))
//	void drawroundrect( x, y, w, h, r ,(color))
//**************************************************
mrb_value mrb_tftc_drawroundrect(mrb_state *mrb, mrb_value self)
{
	unsigned int x,y,w,h,r,color;
	int n = mrb_get_args(mrb,"iiiii|i",&x,&y,&w,&h,&r,&color);
	if( n<6 ) {
		tft->drawRoundRect(x,y,w,h,r,lastc);
	} else {
		tft->drawRoundRect(x,y,w,h,r,color);
		lastc=color;
	}
	lastx=x; lasty=y;
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "drawRoundRect", mrb_tftc_drawroundrect, MRB_ARGS_REQ(5)|MRB_ARGS_OPT(1));	//引数6or5


//**************************************************
//	mRuby method :画面描画関数
//	void fillroundrect(int,int,int,int,int,(int))
//	void fillroundrect( x , y , w , h , r ,(color))
//**************************************************
mrb_value mrb_tftc_fillroundrect(mrb_state *mrb, mrb_value self)
{
	unsigned int x,y,w,h,r,color;
	int n = mrb_get_args(mrb,"iiiii|i",&x,&y,&w,&h,&r,&color);
	if( n<6 ) {
		tft->fillRoundRect(x,y,w,h,r,lastc);
	} else {
		tft->fillRoundRect(x,y,w,h,r,color);
		lastc=color;
	}
	lastx=x; lasty=y;
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "fillRoundRect", mrb_tftc_fillroundrect, MRB_ARGS_REQ(5)|MRB_ARGS_OPT(1));	//引数6or5


//**************************************************
//	mRuby method :画面描画関数
//	void drawtriangle(int,int,int,int,int,int,(int))
//	void drawtriangle( x0, y0, x1, y1, x2, y2,(color))
//**************************************************
mrb_value mrb_tftc_drawtriangle(mrb_state *mrb, mrb_value self)
{
	unsigned int x0,y0,x1,y1,x2,y2,color;
	int n = mrb_get_args(mrb,"iiiiii|i",&x0,&y0,&x1,&y1,&x2,&y2,&color);
	if( n<7 ) {
		tft->drawTriangle(x0,y0,x1,y1,x2,y2,lastc);
	} else {
		tft->drawTriangle(x0,y0,x1,y1,x2,y2,color);
		lastc=color;
	}
	lastx=x0; lasty=y0;
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "drawTriangle", mrb_tftc_drawtriangle, MRB_ARGS_REQ(6)|MRB_ARGS_OPT(1));	//引数7or6


//**************************************************
//	mRuby method :画面描画関数
//	void filltriangle(int,int,int,int,int,int,(int))
//	void filltriangle( x0, y0, x1, y1, x2, y2,(color))
//**************************************************
mrb_value mrb_tftc_filltriangle(mrb_state *mrb, mrb_value self)
{
	unsigned int x0,y0,x1,y1,x2,y2,color;
	int n = mrb_get_args(mrb,"iiiiii|i",&x0,&y0,&x1,&y1,&x2,&y2,&color);
	if( n<7 ) {
		tft->fillTriangle(x0,y0,x1,y1,x2,y2,lastc);
	} else {
		tft->fillTriangle(x0,y0,x1,y1,x2,y2,color);
		lastc=color;
	}
	lastx=x0; lasty=y0;
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "fillTriangle", mrb_tftc_filltriangle, MRB_ARGS_REQ(6)|MRB_ARGS_OPT(1));	//引数7or6


//**************************************************
//	mRuby method :画面描画関数 (テキスト)
//	void textcursor(int,int)
//	void textcursor( x , y )
//**************************************************
mrb_value mrb_tftc_textcursor(mrb_state *mrb, mrb_value self)
{
	unsigned int x,y;
	mrb_get_args(mrb,"ii",&x,&y);
	tft->setCursor(x,y);

	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "textCursor", mrb_tftc_textcursor, MRB_ARGS_REQ(2));	//引数2


//**************************************************
//	mRuby method :画面描画関数 (テキスト)
//	void textcolor(int    , (int))
//	void textcolor( color , backgroundcolor )
//**************************************************
mrb_value mrb_tftc_textcolor(mrb_state *mrb, mrb_value self)
{
	unsigned int color,backgroundcolor;
	int n = mrb_get_args(mrb,"i|i",&color,&backgroundcolor);
	if ( n<2 ) {
		tft->setTextColor(color);	//バックグラウンドカラーの記憶は行わない
	} else {
		tft->setTextColor(color,backgroundcolor);
	}

	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "textColor", mrb_tftc_textcolor, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));	//引数2or1


//**************************************************
//	mRuby method :画面描画関数 (テキスト)
//	void print( char )
//	void print( str  )
//**************************************************
mrb_value mrb_tftc_print(mrb_state *mrb, mrb_value self)
{
	mrb_value value;
	char *str;

	int n = mrb_get_args(mrb,"S", &value);
	str = RSTRING_PTR(value);
	tft->print(str);

	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "print", mrb_tftc_print, MRB_ARGS_REQ(1));//|MRB_ARGS_OPT(1));	//引数2or1


//**************************************************
//	mRuby method :画面描画関数 (テキスト)
//	void println( char )
//	void println( str  )
//**************************************************
mrb_value mrb_tftc_println(mrb_state *mrb, mrb_value self)
{
	mrb_value value;
	char *str;

	int n = mrb_get_args(mrb,"S", &value  );
	str = RSTRING_PTR(value);
	tft->println(str);

	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "println", mrb_tftc_println, MRB_ARGS_REQ(1));//|MRB_ARGS_OPT(1));	//引数2or1


//**************************************************
//	mRuby method :画面描画関数 (テキスト)
//	void drawChar(uint16_t, uint16_t, char, uint16_t, uint16_t, uint8_t);
//	void drawChar(  x, y, c, color, backgroundcolor, size  )
//	キャラクタ1文字のみを、指定の座標に描画する。左上が原点。
//**************************************************
mrb_value mrb_tftc_drawchar(mrb_state *mrb, mrb_value self)
{
	mrb_value value;
	char *str;
	unsigned int x,y,color,bgcolor,size;

	int n = mrb_get_args(mrb,"iiSiii", &x, &y, &value, &color, &bgcolor, &size  );
	str = RSTRING_PTR(value);
	tft->drawChar( x, y, str[0], color, bgcolor, size );

	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "drawChar", mrb_tftc_drawchar, MRB_ARGS_REQ(6));	//引数6


//**************************************************
//	mRuby method :画面描画関数 (テキスト)
//	void textsize( int  )
//	void textsize( size )
//**************************************************
mrb_value mrb_tftc_textsize(mrb_state *mrb, mrb_value self)
{
	unsigned int size;
	int n = mrb_get_args(mrb,"i", &size);
	tft->setTextSize(size);

	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "textSize", mrb_tftc_textsize, MRB_ARGS_REQ(1));	//引数1


//**************************************************
//	mRuby method :画面描画関数 (テキスト)
//	void textwrap( int     )
//	void textwrap( boolean )
//	デフォルトでは true:スクリーン端でテキストを折り返す
//	注）falseにして折り返しを向こうにした場合、色が
//      全画面で反転してしまうバグあり
//**************************************************
mrb_value mrb_tftc_textwrap(mrb_state *mrb, mrb_value self)
{
	unsigned int bl;
	mrb_get_args(mrb,"i", &bl);
	if (bl==0) tft->setTextWrap(false);
	else tft->setTextWrap(true);

	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "textWrap", mrb_tftc_textwrap, MRB_ARGS_REQ(1));	//引数1


//**************************************************
//	mRuby method :画面描画関数 (テキスト)
//	void textfont( int     )
//	void textfont( fontno )
//	使いたいフォントのコメントを外すこと
//**************************************************
mrb_value mrb_tftc_textfont(mrb_state *mrb, mrb_value self)
{
	unsigned int fontno;
	mrb_get_args(mrb,"i", &fontno);
	switch (fontno) {
		case 0	: tft->setFont(); break;	// デフォルトフォント
//		case 1	: tft->setFont(&FreeMono9pt7b); break;
//		case 2	: tft->setFont(&FreeMono12pt7b); break;
//		case 3	: tft->setFont(&FreeMono18pt7b); break;
//		case 4	: tft->setFont(&FreeMono24pt7b); break;
//		case 5	: tft->setFont(&FreeMonoBold9pt7b); break;
//		case 6	: tft->setFont(&FreeMonoBold12pt7b); break;
//		case 7	: tft->setFont(&FreeMonoBold18pt7b); break;
//		case 8	: tft->setFont(&FreeMonoBold24pt7b); break;
//		case 9	: tft->setFont(&FreeMonoBoldOblique9pt7b); break;
//		case 10	: tft->setFont(&FreeMonoBoldOblique12pt7b); break;
//		case 11	: tft->setFont(&FreeMonoBoldOblique18pt7b); break;
//		case 12	: tft->setFont(&FreeMonoBoldOblique24pt7b); break;
//		case 13	: tft->setFont(&FreeMonoOblique9pt7b); break;
//		case 14	: tft->setFont(&FreeMonoOblique12pt7b); break;
//		case 15	: tft->setFont(&FreeMonoOblique18pt7b); break;
//		case 16	: tft->setFont(&FreeMonoOblique24pt7b); break;
//		case 17	: tft->setFont(&FreeSans9pt7b); break;
//		case 18	: tft->setFont(&FreeSans12pt7b); break;
//		case 19	: tft->setFont(&FreeSans18pt7b); break;
//		case 20	: tft->setFont(&FreeSans24pt7b); break;
//		case 21	: tft->setFont(&FreeSansBold9pt7b); break;
//		case 22	: tft->setFont(&FreeSansBold12pt7b); break;
//		case 23	: tft->setFont(&FreeSansBold18pt7b); break;
//		case 24	: tft->setFont(&FreeSansBold24pt7b); break;
//		case 25	: tft->setFont(&FreeSansBoldOblique9pt7b); break;
//		case 26	: tft->setFont(&FreeSansBoldOblique12pt7b); break;
//		case 27	: tft->setFont(&FreeSansBoldOblique18pt7b); break;
//		case 28	: tft->setFont(&FreeSansBoldOblique24pt7b); break;
//		case 29	: tft->setFont(&FreeSansOblique9pt7b); break;
//		case 30	: tft->setFont(&FreeSansOblique12pt7b); break;
//		case 31	: tft->setFont(&FreeSansOblique18pt7b); break;
//		case 32	: tft->setFont(&FreeSansOblique24pt7b); break;
//		case 33	: tft->setFont(&FreeSerif9pt7b); break;
//		case 34	: tft->setFont(&FreeSerif12pt7b); break;
//		case 35	: tft->setFont(&FreeSerif18pt7b); break;
//		case 36	: tft->setFont(&FreeSerif24pt7b); break;
//		case 37	: tft->setFont(&FreeSerifBold9pt7b); break;
//		case 38	: tft->setFont(&FreeSerifBold12pt7b); break;
//		case 39	: tft->setFont(&FreeSerifBold18pt7b); break;
//		case 40	: tft->setFont(&FreeSerifBold24pt7b); break;
//		case 41	: tft->setFont(&FreeSerifBoldItalic9pt7b); break;
//		case 42	: tft->setFont(&FreeSerifBoldItalic12pt7b); break;
//		case 43	: tft->setFont(&FreeSerifBoldItalic18pt7b); break;
//		case 44	: tft->setFont(&FreeSerifBoldItalic24pt7b); break;
//		case 45	: tft->setFont(&FreeSerifItalic9pt7b); break;
//		case 46	: tft->setFont(&FreeSerifItalic12pt7b); break;
//		case 47	: tft->setFont(&FreeSerifItalic18pt7b); break;
//		case 48	: tft->setFont(&FreeSerifItalic24pt7b); break;
		default	: tft->setFont(); break;	// デフォルトフォント
	}
	return mrb_nil_value();			//戻り値は無しですよ。
}
//	mrb_define_module_function(mrb, tftcModule, "textFont", mrb_tftc_textfont, MRB_ARGS_REQ(1));	//引数1


//**************************************************
//	mRuby method :タッチパネル関数
//	int paneltouched( void )
//	戻り値：タッチされていれば 1,タッチされていなければ 0
//	タッチされている場合は、グローバル変数 TPointに
//	座標変換後のタッチ座標を格納する。
//**************************************************
mrb_value mrb_tftc_paneltouched(mrb_state *mrb, mrb_value self)
{
	unsigned int tft_addr;
	int16_t temp;

	tft_addr = (unsigned int)(&tft);


	// Wait for a touch
	if (! ctp->touched()) {
			return mrb_fixnum_value( 0 );	// タッチされていないと0を返す
	}
	// Retrieve a point  
	TS_Point p = ctp->getPoint();
	
	// Print out raw data from screen touch controller
	//Serial.print("X = "); Serial.print(p.x);
	//Serial.print("\tY = "); Serial.print(p.y);
	//Serial.print(" -> ");

	// flip it around to match the screen.
	//p.x = map(p.x, 0, 240, 240, 0);
	//p.y = map(p.y, 0, 320, 320, 0);
	switch(tft->getRotation()){
		case 0 :	TPoint.x = map(p.x, 0, 240, 240, 0);
					TPoint.y = map(p.y, 0, 320, 320, 0);
					break;
		case 1 :	TPoint.x = map(p.y, 0, 320, 320, 0);
					TPoint.y = 240-map(p.x, 0, 240, 240,0);
					break;
		case 2 :	TPoint.x = 240-map(p.x, 0, 240, 240, 0);
					TPoint.y = 320-map(p.y, 0, 320, 320, 0);
					break;
		default :	TPoint.x = 320-map(p.y, 0, 320, 320, 0);
					TPoint.y = map(p.x, 0, 240, 240, 0);
					break;
	}
	// Print out the remapped (rotated) coordinates
	//Serial.print("("); Serial.print(p.x);
	//Serial.print(", "); Serial.print(p.y);
	//Serial.println(")");

	return mrb_fixnum_value( 1 );
}
//	mrb_define_module_function(mrb, tftcModule, "panelTouched", mrb_tftc_paneltouched, MRB_ARGS_NONE());

//**************************************************
//	mRuby method :タッチパネル関数
//	int tuched_x( void )
//	座標変換後のタッチ座標のx座標を返す。
//**************************************************
mrb_value mrb_tftc_touched_x(mrb_state *mrb, mrb_value self)
{
	return mrb_fixnum_value(TPoint.x);
}
//	mrb_define_module_function(mrb, tftcModule, "touchedX", mrb_tftc_touched_x, MRB_ARGS_NONE());


//**************************************************
//	mRuby method :タッチパネル関数
//	int tuched_y( void )
//	座標変換後のタッチ座標のx座標を返す。
//**************************************************
mrb_value mrb_tftc_touched_y(mrb_state *mrb, mrb_value self)
{
	return mrb_fixnum_value(TPoint.y);
}
//	mrb_define_module_function(mrb, tftcModule, "touchedY", mrb_tftc_touched_y, MRB_ARGS_NONE());


//**************************************************
// ライブラリのインスタンスを削除します
// TFTc_clear(mrb_state `mrb)
// 戻り値 0 : 削除不可 , 1 : 削除済
//**************************************************
int TFTc_Clear(mrb_state *mrb)
{
	if(TFTc_setuped==0) return 1;	//セットアップされていない場合は何もせず削除済を返す
	//タッチパネル、およびTFT液晶パネルのインスタンスを生成する。

	if(ctp!=0){
		delete(ctp);
		ctp=0;
	} else {
		return 0;	// 削除不可を返す （2回目の呼び出しが想定される）
	}
	if(tft!=0){
		delete(tft);
		tft=0;
	} else {
		return 0;	// 削除不可を返す （2回目の呼び出しが想定される）
	}
	TFTc_setuped=0;
	
	return 1;
}



//**************************************************
// ライブラリを定義します
// TFTc_Init(mrb_state `mrb)
// 戻り値 0 : 使用不可 , 1 : 使用可
//**************************************************
int TFTc_Init(mrb_state *mrb)
{

	if(TFTc_setuped) return 1;	//セットアップされている場合は何もせず使用可のみ返す
	//タッチパネル、およびTFT液晶パネルのインスタンスを生成する。

	if(ctp==0){
		ctp = new Adafruit_FT6206();
	} else {
		return 0;	// 使用不可を返す （2回目の呼び出しが想定される）
	}
	if(tft==0){
		tft = new Adafruit_ILI9341(TFT_CS, TFT_DC);
	} else {
		return 0;	// 使用不可を返す （2回目の呼び出しが想定される）
	}

	delay(100);

#ifdef TFTC_DEBUG
	Serial.println(F("Cap Touch Paint!"));	//for debug
#endif //TFTC_DEBUG

	tft->begin();		// エラー判定はされていないので、そのまま進める

	TPoint.x=0;	//タッチ座標初期化
	TPoint.y=0;	//タッチ座標初期化
	if (! ctp->begin(40)) {  // pass in 'sensitivity' coefficient

#ifdef TFTC_DEBUG
		Serial.println("Couldn't start FT6206 touchscreen controller");
#endif //TFTC_DEBUG

		//while (1);
		return 0;		// 使用不可を返す
	}
#ifdef TFTC_DEBUG
	Serial.println("Capacitive touchscreen started");	//for debug
#endif //TFTC_DEBUG

	////////////////////
	// 背景画面を初期化
	tft->fillScreen(ILI9341_BLACK);		// 背景画面を初期化

	////////////////////
	// クラスの定義
	struct RClass *tftcModule = mrb_define_module(mrb, "TFTc");

	////////////////////
	// クラス内関数の定義
	mrb_define_module_function(mrb, tftcModule, "drawBmp", mrb_tftc_drawbmp, MRB_ARGS_REQ(3));		//引数3
	mrb_define_module_function(mrb, tftcModule, "newButtons", mrb_tftc_newbuttons, MRB_ARGS_REQ(1));	//引数1
	mrb_define_module_function(mrb, tftcModule, "deleteButtons", mrb_tftc_deletebuttons, MRB_ARGS_NONE());	//引数0
	mrb_define_module_function(mrb, tftcModule, "setButtonMax", mrb_tftc_setbuttonmax, MRB_ARGS_REQ(1));	//引数1
	mrb_define_module_function(mrb, tftcModule, "setDisplayButtonMax", mrb_tftc_setcurrentdisplaybuttonmax, MRB_ARGS_REQ(1));	//引数1
	mrb_define_module_function(mrb, tftcModule, "searchTouchedButton", mrb_tftc_searchtouchedbutton, MRB_ARGS_REQ(2));	//引数2
	mrb_define_module_function(mrb, tftcModule, "searchButtonState", mrb_tftc_searchbuttonstate, MRB_ARGS_REQ(1));	//引数1
	mrb_define_module_function(mrb, tftcModule, "searchPressedButton", mrb_tftc_searchpressedbutton, MRB_ARGS_NONE());	//引数0
	mrb_define_module_function(mrb, tftcModule, "searchReleasedButton", mrb_tftc_searcreleasedbutton, MRB_ARGS_NONE());	//引数0
	mrb_define_module_function(mrb, tftcModule, "initButton", mrb_tftc_initbutton, MRB_ARGS_REQ(10));	//引数10
	mrb_define_module_function(mrb, tftcModule, "drawButtons", mrb_tftc_drawbuttons, MRB_ARGS_NONE());	//引数0
	mrb_define_module_function(mrb, tftcModule, "drawButton", mrb_tftc_drawbutton, MRB_ARGS_REQ(2));	//引数2
	mrb_define_module_function(mrb, tftcModule, "fillScreen", mrb_tftc_fillscreen, MRB_ARGS_REQ(1));	//引数1
	mrb_define_module_function(mrb, tftcModule, "rotation", mrb_tftc_rotation, MRB_ARGS_REQ(1));	//引数1
	mrb_define_module_function(mrb, tftcModule, "width", mrb_tftc_width, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, tftcModule, "height", mrb_tftc_height, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, tftcModule, "color", mrb_tftc_selectcolor, MRB_ARGS_REQ(1));	//引数1
	mrb_define_module_function(mrb, tftcModule, "drawPixel", mrb_tftc_drawpixel, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(1));	//引数3or2
	mrb_define_module_function(mrb, tftcModule, "drawLine", mrb_tftc_drawline, MRB_ARGS_REQ(4)|MRB_ARGS_OPT(1));	//引数5or4
	mrb_define_module_function(mrb, tftcModule, "drawLineto", mrb_tftc_drawlineto, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(1));	//引数3or2
	mrb_define_module_function(mrb, tftcModule, "drawCircle", mrb_tftc_drawcircle, MRB_ARGS_REQ(3)|MRB_ARGS_OPT(1));	//引数4or3
	mrb_define_module_function(mrb, tftcModule, "fillCircle", mrb_tftc_fillcircle, MRB_ARGS_REQ(3)|MRB_ARGS_OPT(1));	//引数4or3
	mrb_define_module_function(mrb, tftcModule, "drawRect", mrb_tftc_drawrect, MRB_ARGS_REQ(4)|MRB_ARGS_OPT(1));	//引数5or4
	mrb_define_module_function(mrb, tftcModule, "fillRect", mrb_tftc_fillrect, MRB_ARGS_REQ(4)|MRB_ARGS_OPT(1));	//引数5or4
	mrb_define_module_function(mrb, tftcModule, "drawRoundRect", mrb_tftc_drawroundrect, MRB_ARGS_REQ(5)|MRB_ARGS_OPT(1));	//引数6or5
	mrb_define_module_function(mrb, tftcModule, "fillRoundRect", mrb_tftc_fillroundrect, MRB_ARGS_REQ(5)|MRB_ARGS_OPT(1));	//引数6or5
	mrb_define_module_function(mrb, tftcModule, "drawTriangle", mrb_tftc_drawtriangle, MRB_ARGS_REQ(6)|MRB_ARGS_OPT(1));	//引数7or6
	mrb_define_module_function(mrb, tftcModule, "fillTriangle", mrb_tftc_filltriangle, MRB_ARGS_REQ(6)|MRB_ARGS_OPT(1));	//引数7or6
	mrb_define_module_function(mrb, tftcModule, "textCursor", mrb_tftc_textcursor, MRB_ARGS_REQ(2));	//引数2
	mrb_define_module_function(mrb, tftcModule, "textColor", mrb_tftc_textcolor, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));	//引数2or1
	mrb_define_module_function(mrb, tftcModule, "print", mrb_tftc_print, MRB_ARGS_REQ(1));	//引数2or1
	mrb_define_module_function(mrb, tftcModule, "println", mrb_tftc_println, MRB_ARGS_REQ(1));	////引数2or1
	mrb_define_module_function(mrb, tftcModule, "drawChar", mrb_tftc_drawchar, MRB_ARGS_REQ(6));	//引数6
	mrb_define_module_function(mrb, tftcModule, "textSize", mrb_tftc_textsize, MRB_ARGS_REQ(1));	//引数1
	mrb_define_module_function(mrb, tftcModule, "textWrap", mrb_tftc_textwrap, MRB_ARGS_REQ(1));	//引数1
	mrb_define_module_function(mrb, tftcModule, "textFont", mrb_tftc_textfont, MRB_ARGS_REQ(1));	//引数1
	mrb_define_module_function(mrb, tftcModule, "panelTouched", mrb_tftc_paneltouched, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, tftcModule, "touchedX", mrb_tftc_touched_x, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, tftcModule, "touchedY", mrb_tftc_touched_y, MRB_ARGS_NONE());

	TFTc_setuped=1;	//初期化済みフラグをセット
	return 1;	// 使用可を返す

}
