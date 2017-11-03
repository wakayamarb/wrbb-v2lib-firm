#!mruby

# main01.mrb    2016/11/27  モジュール化。
#      システムクラス System.useTFTc() にて初期化
#      その後、 TFTc.drawsketch() にてたタッチドローを開始
#       makefile に sTFTc.h と sTFTc.h を追加
#
# main02.mrb    2016/11/28  WA-MIKAN 対応
#       WiFi クラスの動作確認コマンド "w" を追加。
#
# main03.mrb    2016/12/12-14  クラスライブラリ整理
#       Adafruit 2.8'' TFT 用のグラフィック制御関数追加
#       同上のタッチパネル制御関数追加
#
# 注意： cコマンドでSDカードのチェックを行っておかないと、
#        グラフィックが何かの拍子に反転表示になってもとに戻らない。
#        再現方法:c コマンドを行わずに,tコマンド(lineto)や9コマンド(textWrapをfalse)
#        を実施すると実施すると、必ず反転表示になる。
#        とりあえず、cコマンドの内容は事前に行っておくことで対処とする。
#
# main04.mrb    2016/12/27 drawBmp()手続き追加、SDアクセス時のゴミ回避策検討
#       初期化手続きで回避できるようにしたがSDカードが挿されていない場合には
#       出続けることになるなんとも厄介な状態。ファームのバグが取れるまでは
#       これで対処するしかなさそう。なお、SDカードとグラリックライブラリの共存
#       の初期化処理は省略しても、コマンド入力 p c t 1 c t なのでも対処できる。
#


#@Usb = Serial.new(0)        # @UsbをSerial0(USB)に設定
@Usb = Serial.new(0,115200)
@VIEWMSG = 1                # デバッグ用メッセージ表示:1, 非表示:0
@Rot = 0                    # TFT Screen rotation

#// debug用プリント
def dbprintln(ch)
    if @VIEWMSG == 1 then
        @Usb.println(ch)
    end
end

def dbprint(ch)
    if @VIEWMSG == 1 then
        @Usb.print(ch)
    end
end

    @Usb.println("setup");
    @Usb.println("RUBY_VERSION"+RUBY_VERSION); #ruby version = 1.9

    ############################################################
    # 初期化開始　(SDカードとグラフィックライブラリの共存)
    #
    dbprintln("pinMode(10,1) and digitalWrite(10,1)")
    pinMode(10,1)       # pin10 output
    digitalWrite(10,1)  # pin10をHighに

    #############
    # 同じSPIバスを使用するため、SDカードの初期化を行っておく
    # これを行わないと、正しく描画出来ない事がある。
    dbprintln("Checking SD")
    if (0 == System.useSD()) then   # 0:SDカード使用不可 1: 使用可 0ならSDカードが挿入されていない
        dbprintln("SD card is not inserted.")
    else
        dbprintln("SD card is ready.")
        if (0==SD.open(0,"sample.txt",0)) then
            while(true) do
                c = SD.read(0)
                if(c < 0) then
                    break
                end
                #@Usb.write(c.chr,1)
                dbprint(c.chr)
            end
            SD.close(0)
        end
    end
    #
    # SDカード初期化終わり
    ##############
    dbprintln("tone(10,1000,100)")
    tone(10,1000,100)       #pin10 4000Hz 100ms #SD初期化後のssポートの動作不安定応急対処
    digitalWrite(10,1)  # pin10をHighに (beep off)

    delay(100)

    if(1==System.useTFTc()) then
        TFTc.drawPixel(1,1,TFTc.color(1))
    end
    #############
    # 同じSPIバスを使用するため、SDカードの初期化を行っておく
    # これを行わないと、正しく描画出来ない事がある。
    dbprintln("Checking SD")
    if (0 == System.useSD()) then   # 0:SDカード使用不可 1: 使用可 0ならSDカードが挿入されていない
        dbprintln("SD card is not inserted.")
    else
        dbprintln("SD card is ready.")
        if (0==SD.open(0,"sample.txt",0)) then
            while(true) do
                c = SD.read(0)
                if(c < 0) then
                    break
                end
                #@Usb.write(c.chr,1)
                dbprint(c.chr)
            end
            SD.close(0)
        end
    end
    #
    # SDカード初期化終わり
    ##############
    dbprintln("tone(10,1000,100)")
    tone(10,1000,100)       #pin10 4000Hz 100ms #SD初期化後のssポートの動作不安定応急対処
    digitalWrite(10,1)  # pin10をHighに (beep off)
    dbprintln("Clear screen")
    if(1==System.useTFTc()) then
        TFTc.fillScreen(TFTc.color(0))
    end #then

    #
    # 初期化終了　(SDカードとグラフィックライブラリの共存)
    ############################################################




    while(true) do
        while (@Usb.available()>0) do

            @GCH = @Usb.read()
            #@Usb.println @GCH
            ####コマンド解析####
            @ARG=""
            @CMD = @GCH.bytes[0].chr
            for i in 1..@GCH.length-1 do
                if @GCH.bytes[i] != 0xA then
                    @ARG += @GCH.bytes[i].chr
                else
                    break;          # R[LF]10 が送られてきた場合は Rのみ受け取り、10は破棄される
                end
            end
            dbprintln("CMD="+@CMD)
            dbprintln("ARG="+@ARG)
            ####################
            
            #######
            # CMD       リモートコマンド処理
            #######
            
            case @CMD
            when "Q" then
                System.exit()
            when "b" then
                dbprintln("Checking SD")
                if (0 == System.useSD()) then   # 0:SDカード使用不可 1: 使用可 0ならSDカードが挿入されていない
                    dbprintln("SD card is not inserted.")
                else
                    dbprintln("SD card is ready.")
                    if(1==System.useTFTc()) then
                        dbprintln("TFT is ready.")
                        delay(100)
                        dbprintln("Draw 24bit color Bitmap.")
                        TFTc.drawBmp("purple.bmp",0,0)
                    else
                        dbprintln("TFT is not ready.")
                    end #then
                    SD.close(0)
                end
            when "c" then
                dbprintln("Checking SD")
                if (0 == System.useSD()) then   # 0:SDカード使用不可 1: 使用可 0ならSDカードが挿入されていない
                    dbprintln("SD card is not inserted.")
                else
                    dbprintln("SD card is ready.")
                    if (0==SD.open(0,"sample.txt",0)) then
                        while(true) do
                            c = SD.read(0)
                            if(c < 0) then
                                break
                            end
                            #@Usb.write(c.chr,1)
                            dbprint(c.chr)
                        end
                        SD.close(0)
                    end
                end
            when "p" then
                dbprintln("pinMode(10,1) and digitalWrite(10,1)")
                pinMode(10,1)       # pin10 output
                digitalWrite(10,1)  # pin10をHighに
            when "t" then
                dbprintln("tone(10,1000,100)")
                tone(10,1000,100)       #pin10 4000Hz 100ms #SD初期化後のssポートの動作不安定応急対処
                digitalWrite(10,1)  # pin10をHighに (beep off)
            when "w" then
                dbprintln("WiFi test.")
                #ESP8266を一度停止させる(リセットと同じ)
                pinMode(5,1)
                digitalWrite(5,0) # LOW:Disable
                delay 500
                digitalWrite(5,1) # LOW:Disable
                #Usb = Serial.new(0,115200)
                if( System.useWiFi() == 0)then
                    #@Usb.println "WiFi Card can't use."
                    dbprintln("WiFi Card can't use.")
                    #System.exit()
                else
                    dbprintln("WiFi Card can use!")
                    #Usb.print WiFi.version
                    dbprintln("Wifi[bypass]")
                    WiFi.bypass()
                end
            when "r" then
                dbprintln("TFT Draw test.")
                #setupTFTandTP();
                System.useTFTc();
                delay(100)
                while(true) do
                    #flag=0;
            #        delay(10)
                    #flag=drawsketch();
                    flag=TFTc.drawsketch();
                    #flag = drawsketch();
                    if (flag==2) then
            #            dbprintln("Box is touched.")
                        ;
                    elsif (flag==1) then
            #            dbprintln("Screen is touched.")
                    else
                        ;
                        #flag==0 どこもタッチされていない
                    end
                    #break; #とりあえず無限ループから抜ける
                end #while
            when "1" then
                dbprintln("drawPixel test.")
                if(1==System.useTFTc()) then
                    #delay(100)
                    TFTc.drawPixel(100,50,TFTc.color(1))
                    TFTc.drawPixel(100,60)
                    TFTc.drawPixel(110,60)
                    TFTc.drawPixel(110,50,0xcb43)
                    TFTc.drawPixel(105,55,TFTc.color(4))
                end #then
            when "2" then
                dbprintln("drawline test.")
                if(1==System.useTFTc()) then
                    #delay(100)
                    TFTc.drawLine(150,50,150,60,TFTc.color(2))
                    TFTc.drawLine(150,60,160,60)
                    TFTc.drawLine(160,60,160,50)
                    TFTc.drawLine(160,50,150,50,TFTc.color(2))
                    TFTc.drawLine(150,50,155,55,TFTc.color(4))
                end #then
            when "3" then
                dbprintln("drawlineto test.")
                if(1==System.useTFTc()) then
                    #delay(100)
                    TFTc.drawLine(  200,50,200,60,TFTc.color(5))
                    TFTc.drawLineto(210,60)
                    TFTc.drawLineto(210,50)
                    TFTc.drawLineto(200,50)
                    TFTc.drawLineto(205,55,TFTc.color(6))
                end #then
            when "4" then
                dbprintln("drawCircle & fillCircle test.")
                if(1==System.useTFTc()) then
                    #delay(100)
                    TFTc.drawCircle(50,50,15,TFTc.color(6))
                    TFTc.drawCircle(50,55,12,TFTc.color(7))
                    TFTc.fillCircle(50,60,10)
                    TFTc.fillCircle(50,50,5,TFTc.color(8))
                end #then
            when "5" then
                dbprintln("drawRect & fillRect test.")
                if(1==System.useTFTc()) then
                    #delay(100)
                    TFTc.drawRect(50,100,10,4,TFTc.color(8))
                    TFTc.drawRect(55,105,20,8)
                    TFTc.fillRect(60,110,15,20)
                    TFTc.fillRect(70,115,5,10,TFTc.color(9))
                end #then
            when "6" then
                dbprintln("drawRoundRect & fillRoundRect test.")
                if(1==System.useTFTc()) then
                    #delay(100)
                    TFTc.drawRoundRect(100,100,20,14,5,TFTc.color(8))
                    TFTc.drawRoundRect(105,105,25,20,4)
                    TFTc.fillRoundRect(110,110,15,30,3)
                    TFTc.fillRoundRect(120,115,5,10,2,TFTc.color(9))
                end #then
            when "7" then
                dbprintln("drawTriangle & fillTriangle test.")
                if(1==System.useTFTc()) then
                    #delay(100)
                    TFTc.drawTriangle(150,100,155,105,145,105,TFTc.color(2))
                    TFTc.drawTriangle(160,110,165,115,155,115)
                    TFTc.fillTriangle(165,120,175,130,155,130)
                    TFTc.fillTriangle(170,130,180,145,160,145,TFTc.color(10))
                end #then
            when "8" then
                dbprintln("Screen rotation test.")
                if(1==System.useTFTc()) then
                    #delay(100)
                    TFTc.fillScreen(TFTc.color(11))
                    TFTc.fillScreen(0x1111)
                    w=TFTc.width()
                    h=TFTc.height()
                    dbprint("w=");dbprint(w.to_s);dbprint("h=");dbprintln(h.to_s);
                    @Rot += 1
                    if(4==@Rot) then
                        @Rot = 0
                    end
                    TFTc.rotation(@Rot)
                    dbprintln("rotation(1)")
                    w=TFTc.width()
                    h=TFTc.height()
                    dbprint("w=");dbprint(w.to_s);dbprint("h=");dbprintln(h.to_s);

                end #then
            when "9" then
                dbprintln("Font test.")
                if(1==System.useTFTc()) then
                    TFTc.textFont(0)
                    TFTc.textSize(0)
                    TFTc.textCursor(10,0)
                    TFTc.textColor(TFTc.color(2))
                    TFTc.print("abcd")
                    TFTc.textColor(TFTc.color(4))
                    TFTc.println("efghi")
                    TFTc.textColor(TFTc.color(3))
                TFTc.print("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789.!?#")
                TFTc.textWrap(0)
                TFTc.print("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789.!?#")
                TFTc.textWrap(1)
                    TFTc.textColor(TFTc.color(5))
                    TFTc.println("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789.!?#")
                    TFTc.textCursor(50,50)
                    TFTc.println("efghi")
                    TFTc.textColor(TFTc.color(3))
                    TFTc.print("ABCD")
                    TFTc.textColor(TFTc.color(5))
                    TFTc.println("EFGHI")
                    TFTc.textColor(TFTc.color(0),TFTc.color(2))
                    TFTc.print("ABCD")
                    TFTc.textSize(2)
                    TFTc.textColor(TFTc.color(6))
                    TFTc.println("EFGHI")
                    TFTc.drawChar(5,25,"a",TFTc.color(11),0,1)
                    TFTc.drawChar(100,20,"A",TFTc.color(4),0,5)

                    TFTc.textCursor(0,100)
                    TFTc.textFont(5)
                    TFTc.print("ab")
                    TFTc.textFont(6)
                    TFTc.print("cd")
                    TFTc.textFont(7)
                    TFTc.print("ef")
                    TFTc.textFont(8)
                    TFTc.println("g")
                    TFTc.textFont(21)
                    TFTc.print("ab")
                    TFTc.textFont(22)
                    TFTc.print("cd")
                    TFTc.textFont(23)
                    TFTc.print("ef")
                    TFTc.textFont(24)
                    TFTc.println("g")

                    TFTc.textFont(0)
                    TFTc.textSize(0)
                end #then
            when "0" then
                dbprintln("Clear screen")
                if(1==System.useTFTc()) then
                    TFTc.fillScreen(TFTc.color(0))
                end #then
            when "T" then
                dbprintln("Touch test")
                if(1==System.useTFTc()) then
                    TFTc.fillScreen(TFTc.color(0))
                    while(true) do
                        if (TFTc. panelTouched() == 1) then
                            x=TFTc.touchedX;
                            y=TFTc.touchedY;
                            TFTc.drawLineto(x,y,TFTc.color(3))
                        #    TFTc.fillCircle(x-1,y-1,2,TFTc.color(3))
                        
                        #    TFTc.textCursor(x,y)
                        #    TFTc.print("(")
                        #    TFTc.print(x.to_s)
                        #    TFTc.print(",")
                        #    TFTc.print(y.to_s)
                        #    TFTc.print(")")
                            if(x>220 and x<240 and y>220 and y<240) then
                                TFTc.fillScreen(TFTc.color(0))
                                break;
                            end
                        end
                    end
                end #then
            end #when
        end #while
    end #while


    System.exit()