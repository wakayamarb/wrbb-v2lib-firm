#!mruby
Usb = Serial.new(0)
Servo.attach(0, 3)

Usb.println("Servo Attached.")
Servo.write(0, 0)

led 1
pos = 0
inc = 15

10.times do
	delay 500
	pos += inc
	Servo.write(0, pos)
end

Servo.write(0,0)

led 0
Usb.println("Servo End.")
delay 100
	
Servo.detach(0)
Usb.println("Servo Detached.")

