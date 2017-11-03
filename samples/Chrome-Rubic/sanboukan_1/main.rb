#!mruby
#Ver.2.24

pos = 0
num=[0,1,6,9,17,16]
Usb = Serial.new(0)
for i in 1..6 do
    Servo.attach(i, num[i-1])
end
delay 2000

for i in 1..6 do
    Servo.write(i, pos)
end
Usb.println("Servo System Start.")

inc = 5
k = 1
50.times do
    delay 200
    pos += inc
    if pos >= 135 then 
        pos = 135
        inc = -10
    elsif pos < 45 then
        pos = 45
        inc = 10
    end
    for i in 1..6 do
        Servo.write(i, pos)
    end

    led k
    k=1-k
end

for i in 1..6 do
    Servo.write(i, 90)
end
delay 2000

for i in 1..6 do
    Servo.detach(i)
end
delay 2000
for i in 0..5 do
    pinMode(num[i], INPUT)
end
