#!mruby
pos = 0
inc = 10
Usb = Serial.new(0)
Servo.attach(0, 7)


delay 2000

Servo.write(0, 0)



Usb.println("Servo System Start.")

k = 1
50.times do
    delay 200
    pos += inc
    if pos >= 180 then 
        pos = 180
        inc = -10
    elsif pos < 0 then
        pos = 0
        inc = 10
    end
    Servo.write(0, pos)

    led k
    k=1-k
end

while true
    delay 500
    if digitalRead(10) == 0 then
        Servo.write(0, 0)
    else
        Servo.write(0, 90)
    end

    if digitalRead(11) == 1 then
        break
    end
    
    led k
    k = 1 - k
end

#Servo.write(0, 180)
Servo.write(0, 0)

delay 2000
Servo.detach(0)