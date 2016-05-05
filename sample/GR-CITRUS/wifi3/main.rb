#!mruby
digitalWrite(4,1)   # HIGH:Enable, LOW:Disable
digitalWrite(5,1)   # LOW:RESET
digitalWrite(17,1)  # LOW + RESET ... Firmware Writing Mode
pinMode(4,1)
pinMode(5,1)
pinMode(17,1)

usb = Serial.new(0,115200)

if( System.useWiFi() == 0)then
    usb.println "WiFi Card can't use."
   System.exit() 
end
usb.println "WiFi Ready"

#usb.println WiFi.at("CWQAP")

usb.println WiFi.cifsr
usb.println WiFi.cwjap("000740DE0D79","hama1297noiti")
#usb.println WiFi.cwjap("TAROSAY","37000")

usb.println WiFi.cifsr

usb.println "WiFi disconnect"
usb.println WiFi.at("CWQAP")
