#!mruby
#GR-CITRUS Version 2.35

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,1)
digitalWrite(5,0)   # LOW:Disable
delay 500
digitalWrite(5,1)   # LOW:Disable
delay 500

Usb = Serial.new(0,115200)

if(!System.use?('WiFi'))then
  Usb.println "WiFi Card can't use."
  System.exit() 
end

Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
Usb.println WiFi.softAP "GR-CITRUS","37003700",2,3
Usb.println WiFi.dhcp 0,1
Usb.println WiFi.multiConnect 1
60.times do
  Usb.println "Connected IP= " + WiFi.connectedIP
  delay 1000
end
