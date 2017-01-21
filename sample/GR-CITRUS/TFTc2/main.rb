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
    
    #Screen rotation test
    
    if(1==System.useTFTc()) then
        for @Rot in 0..4 do
            TFTc.fillScreen(TFTc.color(11))
            TFTc.fillScreen(0x1111)
            TFTc.textCursor(0,0)
            TFTc.print("abcd")
            w=TFTc.width()
            h=TFTc.height()
            Usb.print("w=");Usb.print(w.to_s);Usb.print("h=");Usb.println(h.to_s);
            @Rot += 1
            if(4==@Rot) then
                @Rot = 0
            end
            TFTc.rotation(@Rot)
            Usb.print("rotation");Usb.println(@Rot.to_s);
            w=TFTc.width()
            h=TFTc.height()
            Usb.print("w=");Usb.print(w.to_s);Usb.print("h=");Usb.println(h.to_s);
            delay(1000)
        end #for
        System.clrTFTc()
    end #if

    System.exit()