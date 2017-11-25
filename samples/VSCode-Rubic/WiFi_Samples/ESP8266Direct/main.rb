#!mruby
#GR-CITRUS Version 2.36
#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,1)
digitalWrite(5,0)   # LOW:Disable
delay 500
digitalWrite(5,1)   # LOW:Disable
delay 500

if(!System.use?('WiFi'))then
  puts "WiFi can't use."
  System.exit() 
end
puts "WiFi Ready"
puts "GR-CITRUS[bypass]"

WiFi.bypass()
