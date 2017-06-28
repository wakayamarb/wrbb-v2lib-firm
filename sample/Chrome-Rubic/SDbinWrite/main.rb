#!mruby
Usb = Serial.new(0)
Rtc.init
Rtc.setTime([2016,11,25,1,0,0])

if( System.useSD() == 0)then
    Usb.println "SD Card can't use."
   System.exit() 
end

SD.open(0,"test010.bin",2)

l = 0
a = ""
for i in 0..4095

    #a += (i/256).floor.chr + (i%256).chr #Big Endian
    a += (i >> 8).chr + (i & 0xFF).chr #Big Endian
    l += 2
    
    #256バイトずつ書き込むようにした
    if(l == 256)then
        SD.write(0,a,l)
        a = ""
        l = 0
        Usb.println i.to_s
        #GC.start
    end
    
    #delay 0
end

SD.write(0,a,l)

SD.close(0)
