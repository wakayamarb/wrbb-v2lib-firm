#!mruby
for num in 0..6 do
    pinMode(num,0)
end
for num in 9..19 do
    pinMode(num,0)
end
pinMode(4,1)
pinMode(17,1)
digitalWrite(4,1)
digitalWrite(17,1)
#digitalWrite(3,1)
#digitalWrite(4,0)
#delay 500
#digitalWrite(4,1)
#delay 500


usb = Serial.new(0,115200)
esp = Serial.new(3,115200)

while(usb.available() > 0)do
    usb.read()
end

usb.println "GR-CITRUS"



WiFi.bypass()

5000.times do
	delay(0)
		
   while(usb.available() > 0)do
        #usb.print usb.read()
        esp.print usb.read()
    end
	
    while(esp.available() > 0)do
        usb.print esp.read()
    end
end
