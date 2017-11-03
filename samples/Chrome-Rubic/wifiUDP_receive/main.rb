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

#UDP通信,WAMIKAN受信:5555, 送信: 5556
Usb.println WiFi.udpOpen(4,"192.168.1.44",5555,5556)

Usb.println "UDP受信した分がarray配列で返ります"
1000.times do
  array = WiFi.recv 4   #受信データがない場合は array[0]に -1 が返ります
  #Usb.println array.length.to_s
  if(array[0] >= 0)then
    for var in array do
      Usb.println var.to_s
    end
  end
  delay 10
end

Usb.println WiFi.cClose 4
Usb.println WiFi.disconnect

