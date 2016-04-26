#!mruby
Usb = Serial.new(0,115200)
Se1 = Serial.new(3,115200)

Usb.println "GR-CITRUS1"
d = [0x30,0x31,0x32,0x33,0x34]

while(Usb.available() > 0)do	#何か受信があった
  Usb.read()
end

Usb.println "GR-CITRUS2"

while true do
  while(Usb.available() > 0)do
    c = Usb.read()
    #Usb.println c.length.to_s
    Se1.write(c,c.length)
  end

  while(Se1.available() > 0)do
    c = Se1.read()
    Usb.write(c,1)
  end
  delay 100
end
