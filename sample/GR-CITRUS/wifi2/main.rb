#!mruby
digitalWrite(5,1)   # LOW:Disable
pinMode(5,1)

#System.useWiFi()

Usb = Serial.new(0,115200)

if( System.useWiFi() == 0)then
    Usb.println "WiFi Card can't use."
   System.exit() 
end
Usb.println "WiFi Ready"
Usb.println "GR-CITRUS[bypass]"

WiFi.bypass()
