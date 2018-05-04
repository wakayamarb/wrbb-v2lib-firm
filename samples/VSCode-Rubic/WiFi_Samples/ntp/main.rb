#!mruby
#GR-CITRUS Version 2.46
#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,OUTPUT)
digitalWrite(5,LOW)   # LOW:Disable
delay 500
digitalWrite(5,HIGH)   # HIGH:Enable
delay 500

if(!System.use?("WiFi"))
  puts "WA-MIKAN can't use."
  System.exit
end

puts WiFi.connect("TAROSAY","37000")
tm = WiFi.ntp("ntp.jst.mfeed.ad.jp", +9)
System.exit if(!tm.instance_of?(Array))

#puts tm
Rtc.init
Rtc.setTime(tm)

def zeroAdd(num)
  str = "00" + num.to_s
  str[str.length-2..str.length]
end

loop do
#60.times do
  tm = Rtc.getTime
  #puts tm
  body = tm[0].to_s + "/" + zeroAdd(tm[1]) + "/" + zeroAdd(tm[2])
  body += " " + zeroAdd(tm[3]) + ":" +zeroAdd(tm[4]) + ":" +zeroAdd(tm[5])
  puts body
  delay 1000
end
