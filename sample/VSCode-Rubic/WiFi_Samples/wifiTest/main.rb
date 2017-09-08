#!mruby
#GR-CITRUS Version 2.15

#pinMode(5,1)
#digitalWrite(5,1)   # LOW:Disable

Usb = Serial.new(0,115200)
Esp = Serial.new(3,115200)

Usb.println "GR-CITRUS & WA-MIKAN"
c = 0
while(true) do
	delay(0)
		
    while(Usb.available() > 0)do
        Esp.print Usb.read()
        c += 1
        if(c > 20)then
            Esp.bps 115200
            c = 0
        end
    end
	
    while(Esp.available() > 0)do
        Usb.print Esp.read()
    end
end
