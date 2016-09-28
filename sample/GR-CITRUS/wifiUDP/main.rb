#!mruby
#GR-CITRUS Version 2.12

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
Usb.println "WiFi Ready"

Usb.println "WiFi Get Version"
Usb.println WiFi.version

Usb.println "WiFi disconnect"
Usb.println WiFi.disconnect

Usb.println "WiFi connecting"
Usb.println WiFi.connect("TAROSAY","37000") 
#Usb.println WiFi.connect("000740DE0D79","")


Usb.println "WiFi ipconfig"
Usb.println WiFi.ipconfig

Usb.println "Set wifi mode"
Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode

Usb.println "..."
Usb.println "Enable multiple connection"
Usb.println WiFi.multiConnect 1
Usb.println "..."

Usb.println "Create a UDP transmission, for example, id is 4"
#UDP通信,WAMIKAN受信:5555, 送信: 5556
Usb.println WiFi.at('CIPSTART=4,"UDP","192.168.1.44",5555,5556,0')
Usb.println "Send 5 bytes to transmission NO.4"

100.times do
  WiFi.at('CIPSEND=4,17')
  WiFi.at("hoho01111122222\r\n",1)
  delay 25
end

WiFi.at('CIPSEND=4,16')
WiFi.at(0x02.chr + "bcdefghijklmn\r\n",1)

Usb.println WiFi.ipconfig
Usb.println "Delete transmission NO.4"
Usb.println WiFi.cClose 4
Usb.println "CWQAP"
Usb.println WiFi.disconnect

Usb.println "end"
