#!mruby
#Ver.2.24
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
pos = 0
vero = [4,10]
num = [18,3,15,14]
Usb = Serial.new(0)
pinMode(18, OUTPUT)
pinMode(3, OUTPUT)
pinMode(15, OUTPUT)
pinMode(14, OUTPUT)

Usb.println("Moter System Start.")

k=0
p=0
Usb.println("CW")
#CW
digitalWrite(num[0],HIGH) #A1
digitalWrite(num[1],LOW)  #A2
digitalWrite(num[2],HIGH) #B1
digitalWrite(num[3],LOW)  #B2
25.times do
    delay 350
    pwm(vero[0], p)
    pwm(vero[1], p)
    p += 10
    led k
    k=1-k
end
25.times do
    delay 350
    pwm(vero[0], p)
    pwm(vero[1], p)
    p -= 10
    led k
    k=1-k
end

p=0
Usb.println("CCW")
#CCW
digitalWrite(num[0],LOW)  #A1
digitalWrite(num[1],HIGH) #A2
digitalWrite(num[2],LOW)  #B1
digitalWrite(num[3],HIGH) #B2
25.times do
    delay 350
    pwm(vero[0], p)
    pwm(vero[1], p)
    p += 10
    led k
    k=1-k
end
25.times do
    delay 350
    pwm(vero[0], p)
    pwm(vero[1], p)
    p -= 10
    led k
    k=1-k
end

pwm(vero[0], 0)
pwm(vero[1], 0)
digitalWrite(num[0],HIGH)
digitalWrite(num[1],HIGH)
digitalWrite(num[2],HIGH)
digitalWrite(num[3],HIGH)

