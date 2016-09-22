#!mruby
Usb = Serial.new(0)
Servo.attach(0, 10)

Usb.println("Servo Attached.")
Servo.write(0, 180)

led 1
ang = 0

1000.times do
	delay 250
	Servo.write(0, 180-ang * 90)
	ang = 1 - ang
end

Servo.write(0,180)

led 0
Usb.println("Servo End.")
delay 100



