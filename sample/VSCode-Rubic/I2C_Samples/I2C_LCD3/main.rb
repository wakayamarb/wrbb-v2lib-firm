#!mruby
@ID = 0x3E
@Usb = Serial.new(0)
@Lcd = I2c.new(1)
@RST = 6

pinMode(@RST,OUTPUT)

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
    digitalWrite(@RST, LOW)
    delay(1)
    digitalWrite(@RST, HIGH)
    delay(40)
    lcd_cmd(0x38)   #// 8bit 2line Normal mode
    lcd_cmd(0x39)   #// 8bit 2line Extend mode
    lcd_cmd(0x14)   #// OSC 183Hz BIAS 1/5

    #/* コントラスト設定 */
    contrast = 0x5F
    lcd_cmd(0x70 + (contrast & 0x0F))   #//下位4bit
    lcd_cmd(0x5C + ((contrast >> 4)& 0x3))     #//上位2bit
    #lcd_cmd(0x6B)                   #// Follwer for 3.3V
    lcd_cmd(0x6C)                   #// Follwer for 3.3V
    delay(300)

    lcd_cmd(0x38)       #// Set Normal mode
    lcd_cmd(0x0C)       #// Display On
    lcd_cmd(0x01)       #// Clear Display
    delay(2)
    
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
    lcd_print("GR-CITRUS")     #//文字列表示
    lcd_setCursor(2,1)      #//カーソルを1行、2文字目に位置設定
    lcd_print("WAKAYAMA")           #//数字を表示

    #lcd_setCursor(0,1)      #//カーソルを0行に位置設定
    #"GRCITRUS".each_byte{|c| @Lcd.write(@ID,0x40,c)}
    
