#!mruby

Usb = Serial.new(0,115200)
S3 = Serial.new(3,115200)

Usb.println "Serial3 Ready"

while(true) do
	delay(0)
		
    while(Usb.available() > 0)do
        S3.print Usb.read()
    end
	
    while(S3.available() > 0)do
        Usb.print S3.read()
    end
end
