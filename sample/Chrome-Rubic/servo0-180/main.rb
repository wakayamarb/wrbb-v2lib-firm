#!mruby
Usb = Serial.new(0)
pin = 7
pinMode(pin, 1)
Servo.attach(0, pin)

#Usb.println("Servo System Start.")

k = 1
10.times do
    Servo.write(0, k * 177)
    led k
    k = 1 - k
    delay 1000
end

Servo.write(0, 0)
Servo.detach(0)
