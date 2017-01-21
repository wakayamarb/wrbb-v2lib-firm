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
    
    # Draw Bitmap test (Only 24bit color BMP file)
    
    Usb.println("Checking SD")
    if (0 == System.useSD()) then   # 0:SDカード使用不可 1: 使用可 0ならSDカードが挿入されていない
        Usb.println("SD card is not inserted.")
    else
        Usb.println("SD card is ready.")
        if(1==System.useTFTc()) then
            Usb.println("TFT is ready.")
            delay(100)
            Usb.println("Draw 24bit color Bitmap.")
            a=TFTc.drawBmp("purple.bmp",0,0)
            Usb.print("drawBmp return code is :"); Usb.println(a.to_s)
            System.clrTFTc()
        else
            Usb.println("TFT is not ready.")
        end #if
        SD.close(0)
    end

    System.exit()