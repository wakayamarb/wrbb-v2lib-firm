#!mruby
#Ver.2.27
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
pos = 0
maxVero = 100
vero = [4,10]
num = [18,3,15,14]
r0 = [HIGH, LOW]
r1 = [LOW, HIGH]
str = ["CW", "CCW"]
Usb = Serial.new(0)
for i in num do
    pinMode(i, OUTPUT)
end
Usb.println("Moter System Start")

digitalWrite(num[0],HIGH)  #A1
digitalWrite(num[1],LOW)  #A2
digitalWrite(num[2],LOW)  #B1
digitalWrite(num[3],HIGH)  #B2
p = 0
k = 1
while(true) do
    maxVero.times do
        delay 10
        pwm(vero[0], p)
        pwm(vero[1], p)
        p += 1
        led k
        k=1-k
    end
    5.times do
        delay 100
        led k
        k=1-k
    end
    p = 0
    pwm(vero[0], p)
    pwm(vero[1], p)

    60.times do
        delay 1000
        led k
        k=1-k
    end
end

pwm(vero[0], 0)
pwm(vero[1], 0)
digitalWrite(num[0],HIGH)
digitalWrite(num[1],HIGH)
digitalWrite(num[2],HIGH)
digitalWrite(num[3],HIGH)

