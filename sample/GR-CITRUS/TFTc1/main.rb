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
    
    # 基本的なグラフィック描画メソッドの動作

    if(1==System.useTFTc()) then
    
        #drawCircle & fillCircle test
        x=50;y=50;
        TFTc.drawCircle( x ,y   ,15,TFTc.color(6))
        TFTc.drawCircle( x ,y+5 ,12,TFTc.color(7))
        TFTc.fillCircle( x ,y+10,10,C_YELLOW)
        TFTc.fillCircle( x ,y   ,5 ,C_GRAY)
    
        #drawPixel test
        x=100;y=50;a=20;
        TFTc.drawPixel(x  ,y  ,TFTc.color(1))
        TFTc.drawPixel(x  ,y+a)
        TFTc.drawPixel(x+a,y+a)
        TFTc.drawPixel(x+a,y   ,0xcb43)
        TFTc.drawPixel(x+a/2 ,y+a/2 ,TFTc.color(4))
    
        #drawLine ,drawLineto test
        x=150;y=40;
        10.step(35,5){ |a|
            TFTc.drawLine(x ,y ,x, y+a,TFTc.color(5))
            TFTc.drawLineto(x+a,y+a)
            TFTc.drawLineto(x+a,y   )
            TFTc.drawLineto(x  ,y   )
            TFTc.drawLineto(x+a/2 ,y+a/2 ,TFTc.color(7))
        }
        
        #drawRect & fillRect test
        x=50;y=100;
        TFTc.drawRect(x   ,y   ,10,4,TFTc.color(8))
        TFTc.drawRect(x+5 ,y+5 ,20,8)
        TFTc.fillRect(x+10,y+10,15,20)
        TFTc.fillRect(x+20,y+15,5 ,10,TFTc.color(9))
    
        #drawRoundRect & fillRoundRect test
        x=100;y=100;
        TFTc.drawRoundRect(x   ,y   ,20,14,5,TFTc.color(8))
        TFTc.drawRoundRect(x+5 ,y+5 ,25,20,4)
        TFTc.fillRoundRect(x+10,y+10,15,30,3)
        TFTc.fillRoundRect(x+20,y+15,5,10,2,TFTc.color(9))
    
        #drawTriangle & fillTriangle test
        x=180;y=110;
        TFTc.fillTriangle(x   ,y   ,x+5 ,y+10,x-5 ,y+10,TFTc.color(2))
        x=150;y=100;
        TFTc.drawTriangle(x+10,y+10,x+15,y+30,x+5 ,y+30,C_ORANGE)
        TFTc.fillTriangle(x+15,y+20,x+23,y+35,x+7 ,y+35,C_BROWN)
        TFTc.drawTriangle(x+25,y+30,x+35,y+45,x+15,y+45,C_YELLOW)
    
        TFTc.drawRoundRect(20,20,200,150,5,TFTc.color(8))
        x=120;y=240;
        for i in 1..5 do
            for j in 1..5 do
                TFTc.drawCircle(x+j*2,y+j*2,i*10,TFTc.color(6)-j*3-i*2)
            end
        end
        
        System.clrTFTc()
    end #if

    System.exit()