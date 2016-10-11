#!mruby

Usb = Serial.new(0,115200)
S1 = Serial.new(1,115200)

Usb.println "Serial1 Ready"

while(true) do
	delay(0)
		
    while(Usb.available() > 0)do
        S1.print Usb.read()
    end
	
    while(S1.available() > 0)do
        Usb.print S1.read()
    end
end
