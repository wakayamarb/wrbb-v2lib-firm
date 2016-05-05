#!mruby
digitalWrite(4,1)   # HIGH:Enable, LOW:Disable
digitalWrite(5,1)   # LOW:RESET
digitalWrite(17,1)  # LOW + RESET ... Firmware Writing Mode
pinMode(4,1)
pinMode(5,1)
pinMode(17,1)

usb = Serial.new(0,115200)
esp = Serial.new(3,115200)

usb.println "GR-CITRUS & WA-MIKAN"
c = 0
while(true) do
	delay(0)
		
    while(usb.available() > 0)do
        esp.print usb.read()
        c += 1
        if(c > 20)then
            esp.bps 115200
            c = 0
        end
    end
	
    while(esp.available() > 0)do
        usb.print esp.read()
    end
end
