#!mruby
#GR-CITRUS Version 2.41
#Usb = Serial.new 0

tm1 = Rtc.getTime
delay 1100
tm2 = Rtc.getTime
if(tm1[5] == tm2[5] || tm1[0] < 2010)then
  puts 'RTC Initialized'
  Rtc.init
  Rtc.setTime([2018,2,13,20,39,00])
end

15.times do
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
