#!mruby
#GR-CITRUS Version 2.46

tm1 = Rtc.getTime
delay 1100
tm2 = Rtc.getTime

if(tm1[5] == tm2[5] || tm1[0] < 2010)then
  puts 'RTC Initialized'
  Rtc.init
  Rtc.setTime([2018,5,1,16,15,0])
end

def zeroAdd(num)
  str = "00" + num.to_s
  str[str.length-2..str.length]
end

loop do
#60.times do
  tm = Rtc.getTime
  body = tm[0].to_s + "/" + zeroAdd(tm[1]) + "/" + zeroAdd(tm[2])
  body += " " + zeroAdd(tm[3]) + ":" +zeroAdd(tm[4]) + ":" +zeroAdd(tm[5])
  puts body
  delay 1000
end