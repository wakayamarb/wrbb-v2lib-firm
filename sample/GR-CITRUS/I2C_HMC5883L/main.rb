#!mruby
# Digital Compass Module HMC5883L
# SCL -> 1
# SDA -> 0
# I2C Address(8bit): 0x3C(W)/0x3D(R)
# I2C Address(7bit): 0x1E = (0x3C >> 1) || (0x3D >> 1)

@ADDRESS   = 0x1E
Usb = Serial.new(0)
Hmc = I2c.new(1)

#連続測定モードに設定
#Hmc.begin(@ADDRESS)
#Hmc.lwrite(0x02)
#Hmc.lwrite(0x00)
#Hmc.end()
# ↓ 1行で書くと ↓
Hmc.write(@ADDRESS,0x02,0x00)


100.times do
  Hmc.begin(@ADDRESS);
  Hmc.lwrite(0x03)
  Hmc.end()
  Hmc.request(@ADDRESS, 6)

  xH = Hmc.lread()
  xL = Hmc.lread()
  zH = Hmc.lread()
  zL = Hmc.lread()
  yH = Hmc.lread()
  yL = Hmc.lread()

  x = ((xH << 8) + xL) 
  x -= 0x10000 if (x >> 15) == 1
  y = ((yH << 8) + yL)
  y -= 0x10000 if (y >> 15) == 1
  z = ((zH << 8) + zL)
  z -= 0x10000 if (z >> 15) == 1

  Usb.println "x:"+x.to_s+" y:"+y.to_s+" z:"+z.to_s
   
  delay(100)
end
