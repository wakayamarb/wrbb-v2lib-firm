#!mruby
#Ver.2.33
Usb = Serial.new(0,115200)
Serial1 = Serial.new(1,115200)

Usb.println "Serial1 Test"
d = [0xaa,0x0d,0x00,0x00,0x00,0x00]

while(Usb.available() > 0)do	#何か受信があった
  Usb.read()
end
while(Serial1.available() > 0)do	#何か受信があった
  Serial1.read()
end

while true do
  while(Usb.available() > 0)do
    c = Usb.read()
    #Usb.println c.length.to_s
    Serial1.write(c,c.length)
  end

  while(Serial1.available() > 0)do
    c = Serial1.read()
    Usb.write(c,1)
  end
  delay 50
end
