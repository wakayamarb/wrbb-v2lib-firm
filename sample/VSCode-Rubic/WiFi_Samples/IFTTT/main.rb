#!mruby
#GR-CITRUS Version 2.22

Usb = Serial.new(0,115200)

if(System.useWiFi() == 0)then
  Usb.println "WiFi Card can't use."
  System.exit() 
end
Usb.println "WiFi Ready"

Usb.println WiFi.disconnect
Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
#Usb.println WiFi.connect("TAROSAY","37000")
Usb.println WiFi.connect("000740DE0D79","")

Usb.println WiFi.ipconfig

Usb.println "WiFi multiConnect Set"
Usb.println WiFi.multiConnect 1


# Digital Compass Module HMC5883L
# SCL -> 1
# SDA -> 0
# I2C Address(8bit): 0x3C(W)/0x3D(R)
# I2C Address(7bit): 0x1E = (0x3C >> 1) || (0x3D >> 1)
@ADDRESS   = 0x1E
Hmc = I2c.new(1)

#連続測定モードに設定
Hmc.write(@ADDRESS,0x02,0x00)
x = 0
y = 0
z = 0
while(true) do
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
   
  delay 50
  if(x < 0)then
      break
  end
end


#header=["User-Agent: gr-citrus", "Connection: close"]
#Usb.println WiFi.httpGet("maker.ifttt.com/trigger/test/with/key/cNk1NkNlkZHNCavIFbupmy").to_s
#Usb.println WiFi.httpGet("maker.ifttt.com/trigger/grcitrus1/with/key/cNk1NkNlkZHNCavIFbupmy").to_s


header=["User-Agent: gr-citrus", "Accept: application/json", "Content-type: application/json"]
body = '{"value1":"' + x.to_s + '","value2":"' + y.to_s + '","value3":"' + z.to_s + '"}'
Usb.println body
Usb.println WiFi.httpPost("maker.ifttt.com/trigger/grcitrus1/with/key/cNk1NkNlkZHNCavIFbupmy", header, body).to_s


Usb.println "WiFi disconnect"
Usb.println WiFi.disconnect
