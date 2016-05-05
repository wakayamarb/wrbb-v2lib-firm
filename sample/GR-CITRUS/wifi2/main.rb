#!mruby
digitalWrite(4,1)
digitalWrite(5,1)
digitalWrite(17,1)

pinMode(4,1)
pinMode(5,1)
pinMode(17,1)

System.useWiFi()

usb = Serial.new(0,115200)

if( System.useWiFi() == 0)then
    usb.println "WiFi Card can't use."
   System.exit() 
end
usb.println "WiFi Ready"


usb.println "GR-CITRUS[bypass]"

WiFi.bypass()

