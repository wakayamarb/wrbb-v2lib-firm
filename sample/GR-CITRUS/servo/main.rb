#!mruby
pos = 0
inc = 10
ubs = Serial.new(0)
Servo.attach(0, 1)
Servo.write(0, pos)
k=1
50.times do
    delay 100
    pos += inc
    Servo.write(0, pos)
    inc *= -1 if pos >= 180 or pos <= 0
    led k
    k=1-k
end

#Servo.write(0, 180)
Servo.write(0, 0)

Servo.detach(0)