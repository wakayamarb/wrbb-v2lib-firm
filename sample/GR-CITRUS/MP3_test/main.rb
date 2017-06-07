#!mruby
#GR-CITRUS Version 2.22

Usb = Serial.new(0,115200)

if( System.useMP3(3,4) == 0)then
    Usb.println "MP3 can't use."
   System.exit() 
end
Usb.println "MP3 Ready"

MP3.led 0
Usb.print MP3.play "/decrain.mp3"

MP3.led 1
Usb.print MP3.play "koidance.mp3"

