#!mruby
#GR-CITRUS Version 2.20

Usb = Serial.new(0,115200)

if( System.useMP3(3,4) == 0)then
    Usb.println "MP3 can't use."
   System.exit() 
end
Usb.println "MP3 Ready"

Usb.print MP3.play "/decrain.mp3"
Usb.print MP3.play "/koidance.mp3"

randomSeed(micros())

100.times do
    n = random(1, 10 + 1)
    f = "/00" + n.to_s
    f = f[f.length-2..f.length] + ".mp3"
    
    Usb.println f.to_s
    Usb.print MP3.play f
end

