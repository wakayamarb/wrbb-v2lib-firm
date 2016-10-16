#!mruby
#GR-CITRUS Version 2.15
pinMode(5,1)
digitalWrite(5,1)   # LOW:Disable

Usb = Serial.new(0,115200)

if( System.useWiFi() == 0)then
    Usb.println "WiFi Card can't use."
   System.exit() 
end
Usb.println "WiFi Ready"
Usb.println "GR-CITRUS[bypass]"

WiFi.bypass()
