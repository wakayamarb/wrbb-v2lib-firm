#!mruby
#GR-CITRUS Version 2.15

Usb = Serial.new(0,115200)

if( System.useWiFi() == 0)then
    Usb.println "WiFi Card can't use."
   System.exit() 
end
Usb.println "WiFi Ready"

Usb.println "WiFi Get Version"
Usb.println WiFi.version

Usb.println "WiFi disconnect"
Usb.println WiFi.disconnect

Usb.println "WiFi Mode Setting"
Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode

Usb.println "WiFi ipconfig"
Usb.println WiFi.ipconfig

Usb.println "WiFi connecting"
Usb.println WiFi.connect("TAROSAY","37000")

Usb.println "WiFi ipconfig"
Usb.println WiFi.ipconfig

Usb.println "WiFi disconnect"
Usb.println WiFi.disconnect

