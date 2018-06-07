#!mruby
#Ver2.50

ADD = 0x3E
Lcd = I2c.new(1)
RST = 6

pinMode(RST,OUTPUT)
Num = 0

#液晶へ１コマンド出力
def lcd_cmd(cmd)
    #puts millis
    Lcd.write(ADD,0x00,cmd)
    #puts millis
    if((cmd == 0x01)||(cmd == 0x02))then
        delay(2)
    else
        delay 0
    end
end

#データを送る
def lcd_data(dat)
    Lcd.write(ADD,0x40,dat)
    delay 0
end

#カーソルのセット
def lcd_setCursor(clm,row)
    if(row==0)then
        lcd_cmd(0x80+clm)
    end
    
    if(row==1)then
        lcd_cmd(0xc0+clm)
    end
end

#LCDの初期化
def lcd_begin()
    puts "lcd_begin"

    digitalWrite(RST, LOW)
    delay(1)
    digitalWrite(RST, HIGH)
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
    
    puts "end of lcd_begin"
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

#"GRCITRUS".each_byte{|c| Lcd.write(ADD,0x40,c)}

 lcd_clear()             #全消去
 lcd_setCursor(0,0)      #カーソルを0行に位置設定
 lcd_print("192 168")    #文字列表示
 lcd_setCursor(0,1)      #カーソルを1行、2文字目に位置設定
 lcd_print("1 130")      #数字を表示

 System.exit "VL53L0X can't use." if(!System.use?("VL53L0X"))
VL53L0X.setI2C(1)
#VL53L0X.begin
puts VL53L0X.init()
VL53L0X.startContinuous

lcd_setCursor(0,0)       #カーソルを1行、2文字目に位置設定
lcd_print("        ")    #文字列表示
while(true)
  #lcd_clear()             #全消去
  lcd_setCursor(0,0)      #カーソルを0行に位置設定
  lcd_print(VL53L0X.readContinuous.to_s + " ")    #文字列表示
  #puts VL53L0X.readContinuous
  #puts VL53L0X.readSingle
  if (VL53L0X.isTimeout)
    lcd_setCursor(0,1)       #カーソルを1行、2文字目に位置設定
    lcd_print("TIMEOUT ")    #文字列表示
    #puts "TIMEOUT"
  end
  delay 0
end
