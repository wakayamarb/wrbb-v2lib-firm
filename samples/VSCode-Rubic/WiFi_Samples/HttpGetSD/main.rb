#!mruby
#GR-CITRUS Version 2.45

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,OUTPUT)
digitalWrite(5,LOW)   # LOW:Disable
delay 500
digitalWrite(5,HIGH)   # HIGH:Enable
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
puts "WiFi Get Version: " + WiFi.version
puts "WiFi disconnect: "+ WiFi.disconnect
puts "WiFi Mode Setting: " + WiFi.setMode(3)  #Station-Mode & SoftAPI-Mode
puts "WiFi ipconfig: " + WiFi.ipconfig
puts "WiFi connecting: " + WiFi.connect("TAROSAY","37000")
puts "WiFi ipconfig: " + WiFi.ipconfig
puts "WiFi multiConnect Set: " + WiFi.multiConnect(1)

heds=["User-Agent: curl"]
puts "HTTP GET Start"
res = WiFi.httpGetSD("wakayama.HTM","wttr.in/wakayama", heds)
puts "wttr.in/wakayama: " + res.to_s; System.exit if res==0
res = WiFi.httpGetSD("osaka.HTM","wttr.in/osaka", heds)
puts "wttr.in/osaka: " + res.to_s; System.exit if res==0
res = WiFi.httpGetSD("kobe.HTM","wttr.in/kobe", heds)
puts "wttr.in/kobe: " + res.to_s; System.exit if res==0
res = WiFi.httpGetSD("nara.HTM","wttr.in/nara", heds)
puts "wttr.in/nara: " + res.to_s; System.exit if res==0
res = WiFi.httpGetSD("kyoto.HTM","wttr.in/kyoto", heds)
puts "wttr.in/kyoto: " + res.to_s; System.exit if res==0

puts "WiFi disconnect"
puts WiFi.disconnect

Usb = Serial.new(0,115200)

def Filedisp(filename)
  puts "\r\n====== SD File Open:" + filename + " ======\r\n"
  #保存したhtmlファイルの読込み
  fn = SD.open(0,filename)
  if(fn < 0)then
    puts "File Open Error."
    return
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
end

Filedisp("wakayama.HTM")
Filedisp("osaka.HTM")
Filedisp("kobe.HTM")
Filedisp("nara.HTM")
Filedisp("kyoto.HTM")

puts "Exit"
