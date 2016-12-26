#!mruby
#GR-CITRUS Version 2.20

Usb = Serial.new(0,115200)

if( System.useMP3(3,4) == 0)then
    Usb.println "MP3 can't use."
   System.exit() 
end
Usb.println "MP3 Ready"
MP3.led 1

#MP3.play "no1/01.mp3"

randomSeed(micros())

100.times do
    a = random(1, 2 + 1)
    n = random(1, 20 + 1)
    f = "00" + n.to_s
    MP3.play "no" + a.to_s + "/" + f[f.length-2..f.length] + ".mp3"
end

