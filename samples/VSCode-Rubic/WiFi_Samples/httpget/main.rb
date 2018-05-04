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

puts WiFi.disconnect
# WiFi Mode Setting
puts WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
puts WiFi.connect("TAROSAY","37000")
# WiFi multiConnect Set
puts WiFi.multiConnect 1

value = 2
res = WiFi.httpGet("192.168.1.58:3000/?value1=" + value.to_s + "&value2=" + (value*value).to_s)
puts 'Flase' if(res == 0)
