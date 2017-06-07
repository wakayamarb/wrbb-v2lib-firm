#!mruby
usb = Serial.new(0)
#Rtc.init
#Rtc.setTime([2017,05,27,0,51,20])

if( System.useSD() == 0)then
    usb.println "SD Card can't use."
   System.exit() 
end

usb.println SD.rmdir('abcdefg').to_s
usb.println SD.exists("test1.txt").to_s
usb.println SD.open(0,"test1.txt",2).to_s
usb.println SD.open(1,"test2.txt",1).to_s

10.times do
    SD.write(0,"1234567890",10)
    SD.write(0,0xD.chr + 0xA.chr,2)
    SD.write(1,"12345678",8)
    SD.write(1,0xD.chr + 0xA.chr,2)
    dt = Rtc.getTime()

    usb.print "#{dt[0].to_s}/#{dt[1].to_s}/#{dt[2].to_s}"
    usb.println " #{dt[3].to_s}:#{dt[4].to_s}:#{dt[5].to_s}"
    
    delay 1000
end
SD.close(0)
SD.close(1)

usb.println SD.open(0,"test2.txt").to_s
size = SD.size(0)
size.times do
    c = SD.read(0)
    usb.write(c.chr, 1)
end
SD.close(0)

SD.rename('test1.txt', 'test3.txt')
