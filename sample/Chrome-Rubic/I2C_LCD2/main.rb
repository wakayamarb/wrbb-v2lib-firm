#!mruby
@ID = 0x3E
@Usb = Serial.new(0)
@Lcd = I2c.new(1)

#// 液晶へ１コマンド出力
def lcd_cmd(cmd)
    @Lcd.write(@ID,0x00,cmd)

    if((cmd == 0x01)||(cmd == 0x02))then
        delay(2)
    else
        delay 0
    end
end

#//データを送る
def lcd_data(dat)
    @Lcd.write(@ID,0x40,dat)
    delay 0
end

def lcd_setCursor(clm,row)
    if(row==0)then
        lcd_cmd(0x80+clm)
    end
    
    if(row==1)then
        lcd_cmd(0xc0+clm)
    end
end

def lcd_begin()
    @Usb.println("lcd_begin")
    delay(10)
    lcd_cmd(0x38)   #// 8bit 2line Normal mode
    lcd_cmd(0x39)   #// 8bit 2line Extend mode
    lcd_cmd(0x14)   #// OSC 183Hz BIAS 1/5

    #/* コントラスト設定 */
    lcd_cmd(0x70 + (0x20 & 0x0F))   #//下位4bit
    lcd_cmd(0x5C + (0x20 >> 4))     #//上位2bit
    lcd_cmd(0x6B)                   #// Follwer for 3.3V
    delay(1)

    lcd_cmd(0x38)       #// Set Normal mode
    lcd_cmd(0x0C)       #// Display On
    lcd_cmd(0x01)       #// Clear Display
    delay(1)
    
    @Usb.println("end of lcd_begin");
end

#//全消去関数
def lcd_clear()
    lcd_cmd(0x01)   #//初期化コマンド出力
    delay(2)
end

#//文字列表示関数
def lcd_print(cs)
    cs.each_byte{|c| lcd_data(c)}
end


    lcd_begin() #//初期設定
    @Usb.println("setup");

    "GRCITRUS".each_byte{|c| @Lcd.write(@ID,0x40,c)}

    lcd_setCursor(0,0)      #//カーソルを0行に位置設定
    lcd_print("GRCITRUS")     #//文字列表示
    lcd_setCursor(0,1)      #//カーソルを0行に位置設定
    lcd_print("WAKAYAMA")           #//数字を表示

    lcd_setCursor(0,1)      #//カーソルを0行に位置設定
    "GRCITRUS".each_byte{|c| @Lcd.write(@ID,0x40,c)}
    
