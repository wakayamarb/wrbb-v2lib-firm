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
  
puts "GR-CITRUS[bypass]"

WiFi.bypass
