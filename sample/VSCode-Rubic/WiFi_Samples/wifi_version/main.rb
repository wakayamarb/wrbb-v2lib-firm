#!mruby
#GR-CITRUS Version 2.15

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,1)
digitalWrite(5,0)   # LOW:Disable
delay 500
digitalWrite(5,1)   # LOW:Disable

Usb = Serial.new(0,115200)

if( System.useWiFi() == 0)then
    Usb.println "WiFi Card can't use."
   System.exit() 
end

Usb.print WiFi.version

