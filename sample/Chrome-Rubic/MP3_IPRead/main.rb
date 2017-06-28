#!mruby
#GR-CITRUS Version 2.22

Usb = Serial.new(0,115200)

if(System.useWiFi() == 0)then
  Usb.println "WiFi Card can't use."
  System.exit() 
end
Usb.println "WiFi Ready"

if(System.useMP3(3,4) == 0)then
  Usb.println "MP3 can't use."
  System.exit() 
end
Usb.println "MP3 Ready"

Usb.println WiFi.disconnect
Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
#Usb.println WiFi.connect("TAROSAY","37000")
Usb.println WiFi.connect("000740DE0D79","")

res = WiFi.ipconfig
s = res.index("STAIP,") + 7
e = res.index("+CIFSR:STAMAC") - 4

MP3.led 1
1.times do
  MP3.play "voice/ip.mp3"
  for i in s..e do
    if(res.bytes[i].chr == ".")then
      MP3.play "voice/ten.mp3"
    else
      MP3.play "voice/" + res.bytes[i].chr + ".mp3"
    end
    delay 1
  end
  MP3.play "voice/desu.mp3"
  delay 300

    MP3.play "no1/00.mp3"

end

Usb.println WiFi.disconnect
