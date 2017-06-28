#!mruby
#GR-CITRUS Version 2.30

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,1)
digitalWrite(5,LOW)   # LOW:Disable

Usb = Serial.new(0,115200)

if( !System.use?('SD') )then
    Usb.println "SD Card can't use."
   System.exit() 
end
Usb.println "SD Ready"


#-----------------------------------------
Usb.println("System Start")

SD.cpmem( 'test.rb', 'test1.rb' )
