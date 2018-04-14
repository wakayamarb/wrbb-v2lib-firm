#!mruby
#GR-CITRUS Version 2.36

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,OUTPUT)
digitalWrite(5,LOW)   # LOW:Disable
delay 500
digitalWrite(5,HIGH)   # LOW:Disable
delay 500

Usb = Serial.new(0,115200)

if(!System.use?('Jpeg'))then
  Usb.println "Jpeg can't use."
  System.exit() 
end

res = Jpeg.decode("4.jpg")
Usb.println "4.jpg"
Usb.println res[0].to_s
Usb.println res[1].to_s
Usb.println res[2].to_s

res = Jpeg.stl("4.raw")
Usb.println "4.raw"
Usb.println res[0].to_s
Usb.println res[1].to_s
Usb.println res[2].to_s
