#!mruby
Usb = Serial.new(0,115200)

    # 初期化開始　(SDカードとグラフィックライブラリの共存)
    # 10pinからのノイズ出力の不具合対策
    # 予めSDカードにsample.txtを作り、カードを挿入しておく
    pinMode(10,1)       # pin10 output
    digitalWrite(10,1)  # pin10をHighに
    Usb.println("Checking SD")
    if (0 == System.useSD()) then
        Usb.println("SD card is not inserted.")
    else
        Usb.println("SD card is ready.")
        if (0==SD.open(0,"sample.txt",0)) then
            while(true) do
                c = SD.read(0)
                if(c < 0) then
                    break
                end
                Usb.print(c.chr)
            end
            SD.close(0)
        end
    end
    Usb.println("tone(10,1000,100)")
    tone(10,1000,100)
    digitalWrite(10,1)  # pin10をHighに (beep off)

    delay(100)

    if(1==System.useTFTc()) then
        TFTc.drawPixel(1,1,TFTc.color(1))
    end
    Usb.println("Checking SD")
    if (0 == System.useSD()) then
        Usb.println("SD card is not inserted.")
    else
        Usb.println("SD card is ready.")
        if (0==SD.open(0,"sample.txt",0)) then
            while(true) do
                c = SD.read(0)
                if(c < 0) then
                    break
                end
                Usb.print(c.chr)
            end
            SD.close(0)
        end
    end
    tone(10,1000,100)
    digitalWrite(10,1)  # pin10をHighに (beep off)
    Usb.println("Clear screen")
    if(1==System.useTFTc()) then
        TFTc.fillScreen(TFTc.color(0))
    end
    # 初期化終了
    
    # Draw Screen test
    # Rubyにてボタン生成、タッチ検出、グラフィック描画を制御
    if(1==System.useTFTc()) then
        TFTc.fillScreen(TFTc.color(0))
        w=TFTc.width()  #ディスプレイの幅を取得
        h=TFTc.height() #ディスプレイの高さを取得
        #button初期化
        TFTc.setButtonMax(10) #ボタンのインスタンスを生成
        TFTc.setDisplayButtonMax(8) # この画面で使用するボタン数を指定
        TFTc.initButton(1,20,20,40,40,TFTc.color(9),TFTc.color(0),TFTc.color(2),"RED",1)
        TFTc.initButton(2,60,20,40,40,TFTc.color(9),TFTc.color(0),TFTc.color(4),"YELLOW",1)
        TFTc.initButton(3,100,20,40,40,TFTc.color(9),TFTc.color(0),TFTc.color(5),"GREEN",1)
        TFTc.initButton(4,140,20,40,40,TFTc.color(9),TFTc.color(0),TFTc.color(10),"CYAN",1)
        TFTc.initButton(5,180,20,40,40,TFTc.color(9),TFTc.color(0),TFTc.color(6),"BLUE",1)
        TFTc.initButton(6,220,20,40,40,TFTc.color(9),TFTc.color(0),TFTc.color(11),"MAGEN",1)
        TFTc.initButton(7,220,60,40,40,TFTc.color(9),TFTc.color(0),TFTc.color(1),"EXIT",1)
        TFTc.initButton(8,20,60,40,40,TFTc.color(9),TFTc.color(0),TFTc.color(7),"CLEAR",1)
        TFTc.drawButtons()
        TFTc.searchTouchedButton(-1,-1);    # buttonのタッチ履歴をクリア
        currentcolor = 2   # RED
        old_bno = 1 # ハイライトのための処理:前回タッチしてハイライトされているボタンno
        bno = 1     # ハイライトのための処理:現在のタッチ検出ボタンno
        TFTc.drawButton(1,1);    # button 1 を反転させる
        exit_flag = 0
        while(true) do
            x=(-1); y=(-1); # releaseトラッキング用に座標をクリアする。
            if (TFTc. panelTouched() == 1) then # パネル全体のタッチ検出
                x=TFTc.touchedX;
                y=TFTc.touchedY;
            end #if
            if(x<w or y<h) then
                TFTc.searchTouchedButton(x,y) # 全てのボタンについてタッチされたか調査
                bno = TFTc.searchReleasedButton()  # リリースされたボタンを検出
                if(bno!=0) then # 何れかのボタンがタッチされていた場合
                    Usb.print("Released Btn no:");Usb.println(bno.to_s)
                    if(bno!=8) then     #no8のボタンは画面クリア用なので、反転させる必要なし
                        TFTc.drawButton(old_bno,0) # 前回タッチされたボタンの反転表示をクリア
                        TFTc.drawButton(bno,1)     # 今回タッチされたボタンを反転表示する
                        old_bno = bno              # 「前回タッチされたボタンno」を記憶する
                    end #if
                end #if
                case bno
                when 1 then
                    currentcolor = 2;
                when 2 then
                    currentcolor = 4;
                when 3 then
                    currentcolor = 5;
                when 4 then
                    currentcolor = 10;
                when 5 then
                    currentcolor = 6;
                when 6 then
                    currentcolor = 11;
                when 7 then # EXIT ボタンでこのモードを終了する
                    TFTc.deleteButtons()   # 生成したボタンのインスタンスを全て消去(ここでは10個分)
                    TFTc.fillScreen(TFTc.color(0)) # 画面を暗転する
                    exit_flag=1
                when 8 then # 画面をクリアする
                    TFTc.fillScreen(TFTc.color(0))
                    TFTc.drawButtons()
                    TFTc.drawButton(old_bno,1)     # 前回タッチされたボタンを反転表示する
                when 0 then
                    if x!=(-1) then #画面がタッチされており、かつボタンがタッチされていない場合
                        TFTc.fillCircle(x,y,3,TFTc.color(currentcolor))
                    end # then
                end #when
            end #if
            if exit_flag==1 then
                break   #exit while(true)
            end #if
        end #while
        System.clrTFTc()
    end #if

    System.exit()