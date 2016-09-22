#!mruby
digitalWrite(5,1)   # LOW:Disable
pinMode(5,1)

Usb = Serial.new(0,115200)

if( System.useWiFi() == 0)then
    Usb.println "WiFi Card can't use."
   System.exit() 
end
Usb.println "WiFi Ready"

Usb.println WiFi.version

Usb.println WiFi.disconnect

Usb.println WiFi.config
Usb.println WiFi.connect("TAROSAY","37000")

Usb.println WiFi.config

Usb.println WiFi.at "CWMODE=1"
Usb.println WiFi.at "CIPMUX=1"
#cip = "CIPSTART=4," + 0x22.chr + "TCP" + 0x22.chr + "," + 0x22.chr + "www.yahoo.co.jp" + 0x22.chr + ",80"
#Usb.println cip

#Usb.println WiFi.at cip
Usb.println WiFi.at 'CIPSTART=4,"TCP","wakayamarb.github.io",80'

Usb.println WiFi.at "CIPSEND=4,57"
Usb.println WiFi.at "GET /wa-mikan/ HTTP/1.1", 1
Usb.println WiFi.at "Host: wakayamarb.github.io", 1
Usb.println WiFi.at "", 1
Usb.println WiFi.at "", 1


Usb.println "WiFi disconnect"
Usb.println WiFi.disconnect

