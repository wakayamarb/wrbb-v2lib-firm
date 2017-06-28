#!mruby
# Ondo Sensor Unit KP-ADT7410
# SCL -> 1
# SDA -> 0
# I2C Address(8bit): 0x90(W)
# I2C Address(7bit): 0x48 = (0x90 >> 1)


@ADDRESS   = 0x48
Usb = Serial.new(0)
Hmc = I2c.new(1)

Usb.println "NEW"
Usb.println "NEW"
Usb.println "NEW"
Usb.println "NEW"

1.times do

Hmc.begin(@ADDRESS)
Hmc.lwrite(0x03)
Hmc.lwrite(0x80)
#Hmc.end()


Hmc.request(@ADDRESS,2)
Usb.println "Hmc.request"

datL = Hmc.lread()
datH = Hmc.lread()
Hmc.end()
Usb.println "dat:"+datL.to_s
 delay(250)
end
