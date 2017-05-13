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
    
    #Font test

    if(1==System.useTFTc()) then
        TFTc.fillScreen(TFTc.color(0))
        TFTc.textFont(0)    # デフォルトフォントに設定
        TFTc.textSize(0)    # 倍率を1に設定
        TFTc.textCursor(10,0)
        TFTc.textColor(TFTc.color(2))
        TFTc.print("Size:0,RED ")
        TFTc.textColor(TFTc.color(4))
        TFTc.println("YELLOW")
        TFTc.textColor(TFTc.color(3))
        TFTc.textWrap(0)
        TFTc.println("Text Wrap 0,ORANGE:")
        TFTc.println("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789.!?#")
        TFTc.textWrap(1)
        TFTc.textColor(TFTc.color(5))
        TFTc.println("Text Wrap 1,GREEN :");
        TFTc.println("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789.!?#")
        TFTc.textSize(1)
        TFTc.textColor(TFTc.color(6))
        TFTc.println("Size:1,BLUE: ABC")
        TFTc.textSize(2)
        TFTc.textColor(TFTc.color(7))
        TFTc.println("Size:2,PURPLE: ABC")
        TFTc.textSize(3)
        TFTc.textColor(TFTc.color(8))
        TFTc.println("Size:3,GRAY: ABC")
        TFTc.textSize(1)
        TFTc.textCursor(50,130)
        TFTc.println("Size:1,cursor(50,100)[CR]")
        TFTc.textColor(TFTc.color(9))
        TFTc.print("White:ABCD")
        TFTc.textColor(TFTc.color(10))
        TFTc.println("CYAN:EFGHI")
        TFTc.textSize(2)
        TFTc.textColor(TFTc.color(0),TFTc.color(2))
        TFTc.print("                   ")
        TFTc.print("     BLACK/RED     ")
        TFTc.print("                   ")
        TFTc.print("                   ")
        TFTc.drawChar(100,200,"*",TFTc.color(4),0,1)
        TFTc.drawChar(108,192,"*",TFTc.color(4),0,1)
        TFTc.drawChar(116,200,"*",TFTc.color(4),0,1)
        TFTc.textCursor(0,210)
        TFTc.textSize(1)
        TFTc.textColor(11)
        for i in 1..8 do
            TFTc.print("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789.!?#")
        end
        TFTc.drawChar(100,250,"A",TFTc.color(4),0,5)
        # 以下はフリーフォントを組み込んでいる場合に実行可能
        # TFTc.textCursor(0,100)
        # TFTc.textFont(5)
        # TFTc.print("ab")
        # TFTc.textFont(6)
        # TFTc.print("cd")
        # TFTc.textFont(7)
        # TFTc.print("ef")
        # TFTc.textFont(8)
        # TFTc.println("g")
        # TFTc.textFont(21)
        # TFTc.print("ab")
        # TFTc.textFont(22)
        # TFTc.print("cd")
        # TFTc.textFont(23)
        # TFTc.print("ef")
        # TFTc.textFont(24)
        # TFTc.println("g")
        TFTc.textFont(0)
        TFTc.textSize(0)
        System.clrTFTc()
    end #if

    System.exit()