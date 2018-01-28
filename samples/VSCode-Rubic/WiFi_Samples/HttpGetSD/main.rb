#!mruby
#GR-CITRUS Version 2.15
Usb = Serial.new(0,115200)

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,1)
digitalWrite(5,0)   # LOW:Disable
delay 500
digitalWrite(5,1)   # LOW:Disable
delay 500

if(!System.use?('SD'))then
  puts "SD can't use."
  System.exit() 
end
if(!System.use?('WiFi'))then
  puts "WiFi can't use."
  System.exit() 
end
puts "WiFi Ready"

puts "WiFi Get Version"
puts WiFi.version

puts "WiFi disconnect"
puts WiFi.disconnect

puts "WiFi Mode Setting"
puts WiFi.setMode 3  #Station-Mode & SoftAPI-Mode

puts "WiFi ipconfig"
puts WiFi.ipconfig

puts "WiFi connecting"
puts WiFi.connect("TAROSAY","37000")
#puts WiFi.connect("000740DE0D79","")

puts "WiFi ipconfig"
puts WiFi.ipconfig

puts "WiFi multiConnect Set"
puts WiFi.multiConnect 1

heds=["User-Agent: curl"]
puts "HTTP GET Start"
puts WiFi.httpGetSD("WETHER1.HTM","wttr.in/wakayama")
#puts WiFi.httpGetSD("wether4.htm","wttr.in/wakayama", heds).to_s
#Usb.println WiFi.httpGetSD("yahoo1.htm","www.yahoo.co.jp").to_s
#Usb.println WiFi.httpGetSD("google1.htm","www.google.co.jp").to_s
#Usb.println WiFi.httpGetSD("yahoo2.htm","www.yahoo.co.jp").to_s
#Usb.println WiFi.httpGetSD("google2.htm","www.google.co.jp").to_s
#Usb.println WiFi.httpGetSD("yahoo3.htm","www.yahoo.co.jp").to_s
#Usb.println WiFi.httpGetSD("google3.htm","www.google.co.jp").to_s

puts "WiFi disconnect"
puts WiFi.disconnect

#保存したhtmlファイルの読込み
fn = SD.open(0, 'WETHER1.HTM')
if(fn < 0)then
  System.exit
end

while(true)do
  c = SD.read(fn)
  if(c < 0)then
    break
  end
  if(c == 0x0a)then
    Usb.println()
  else
    Usb.print(c.chr)
  end
end
SD.close(fn)
