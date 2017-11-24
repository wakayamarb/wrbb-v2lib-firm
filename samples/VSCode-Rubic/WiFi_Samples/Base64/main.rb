#!mruby
#GR-CITRUS v2.36

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,OUTPUT)
digitalWrite(5,LOW)   # LOW:Disable
delay 500
digitalWrite(5,HIGH)   # HIGH:Enable
delay 500

if(!System.use?('WiFi'))then
  puts "WiFi can't use."
  System.exit() 
end

puts "BASE64 TEST"
puts WiFi.base64('photo.jpg', 'photo.b64')
puts "Encode Finish"
puts WiFi.base64('photo.b64', 'photo1.jpg', 1)
puts "Decode Finish"
