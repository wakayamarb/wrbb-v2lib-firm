#!mruby
#GR-CITRUS Version 2.46
#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,OUTPUT)
digitalWrite(5,LOW)   # LOW:Disable
delay 500
digitalWrite(5,HIGH)   # HIGH:Enable
delay 500

if(!System.use?("WiFi"))
  puts "WiFi Card can't use."
  System.exit() 
end

puts WiFi.connect("TAROSAY","37000")
tm = WiFi.ntp("ntp.jst.mfeed.ad.jp", +9)
System.exit if(!tm.instance_of?(Array))

puts tm
Rtc.init
Rtc.setTime(tm)

loop do
#60.times do
  tm = Rtc.getTime
  #puts tm
  year = tm[0].to_s
  month = "00" + tm[1].to_s
  day = "00" + tm[2].to_s
  hour = "00" + tm[3].to_s
  min = "00" + tm[4].to_s
  sec = "00" + tm[5].to_s
  body = year + "/"
  body += month[month.length-2..month.length] + "/"
  body += day[day.length-2..day.length] + " "
  body += hour[hour.length-2..hour.length] + ":"
  body += min[min.length-2..min.length] + ":"
  body += sec[sec.length-2..sec.length]
  puts body
  delay 1000
end
