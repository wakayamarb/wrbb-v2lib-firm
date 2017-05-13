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
    
    # Button test 2
    # ボタン使用例（詳細）：押下とリリースを区別する場合。同時押し検出をする場合。
    if(1==System.useTFTc()) then
        TFTc.fillScreen(TFTc.color(0))
        #button初期化
        TFTc.setButtonMax(5)
        TFTc.setDisplayButtonMax(3)
        TFTc.initButton(1,50,100,50,50,TFTc.color(9),TFTc.color(0),TFTc.color(4),"On",2)
        TFTc.initButton(2,120,100,50,50,TFTc.color(9),TFTc.color(0),TFTc.color(2),"Off",2)
        TFTc.initButton(3,195,100,60,50,TFTc.color(9),TFTc.color(0),TFTc.color(6),"Exit",2)
        TFTc.drawButtons()
        TFTc.searchTouchedButton(-1,-1);    # buttonのタッチ履歴をクリア
        exit_flag=0;
        while(true) do
            x=(-1); y=(-1); # release トラッキング用に座標をクリアする
            if (TFTc. panelTouched() == 1) then
                x=TFTc.touchedX;
                y=TFTc.touchedY;
            end #if
            
            #TFTc.drawLineto(x,y,TFTc.color(3))
            TFTc.searchTouchedButton(x,y)
            for i in 1..3 do
                ret = TFTc.searchButtonState(i)
                case ret
                when -1 then
                    Usb.print("Pressed Btn no:");Usb.println(i.to_s)
                when 1 then
                    Usb.print("Released Btn no:");Usb.println(i.to_s)
                    if (3==i) then
                        exit_flag=1
                    end
                end #case
            end #for
            if exit_flag == 1 then
                TFTc.deleteButtons()   # 生成したボタンのインスタンスを全て消去(ここでは5個分)
                TFTc.fillScreen(TFTc.color(0))
                break   #exit this mode
            end # then
        end #while
        System.clrTFTc()
    end #if

    System.exit()