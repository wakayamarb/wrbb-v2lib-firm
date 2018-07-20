#!mruby
#Ver2.48
#VL53LOX
Dev = I2c.new(1)
Add = 0x29    # 0b0101001x アドレス
delay 300

#初期化
Dev.begin(Add)
Dev.lwrite(0x88)
Dev.lwrite(0x00)
Dev.end()
delay 300

#who am i
Dev.begin(Add)
Dev.lwrite(0xc0)
Dev.end()

Dev.request(Add,1)
datL = Dev.lread()

puts "WHOAMI: 0x" + datL.to_s(16)



