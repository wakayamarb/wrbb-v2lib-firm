#!mruby
#GR-CITRUS Version 2.15

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,1)
digitalWrite(5,0)   # LOW:Disable
delay 500
digitalWrite(5,1)   # LOW:Disable
delay 500

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
#Usb.println WiFi.connect("TAROSAY","37000")
Usb.println WiFi.connect("000740DE0D79","")

Usb.println "WiFi ipconfig"
Usb.println WiFi.ipconfig

Usb.println "Set wifi mode"
Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode

Usb.println "Enable multiple connection"
Usb.println WiFi.multiConnect 1

Usb.println "Create a UDP transmission, for example, id is 4"
#UDP通信,WAMIKAN受信:5555, 送信: 5556
Usb.println WiFi.udpOpen(4,"192.168.1.44",5555,5556)

100.times do
  WiFi.send 4,"hoho01111122222\r\n"
  delay 25
end
Usb.println WiFi.send(4, 0x02.chr + "bcdefghijklmn" + 0x03.chr + "ddd\r\n").to_s

Usb.println WiFi.cClose 4
Usb.println WiFi.disconnect

