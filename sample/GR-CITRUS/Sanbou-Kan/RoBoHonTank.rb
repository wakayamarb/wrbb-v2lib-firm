#!mruby
#Ver.2.27
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
pos = 0
maxVero = 165
vero = [4,10]
num = [18,3,15,14]
r0 = [HIGH, LOW]
r1 = [LOW, HIGH]
str = ["CW", "CCW"]
lev = [0,17]

Usb = Serial.new(0)
for i in num do
    pinMode(i, OUTPUT)
end
for i in lev do
    pinMode(i, 2) #プルアップ
end
Usb.println("Moter System Start")


def lever(lev)
  digitalRead(lev[0]) + 2 * digitalRead(lev[1])
end

cmd = lever
200.times do
  if(lever == 0)then
    if(cmd == 0)then
      cmd = 1
      Usb.println "Start"
    elif cmd == 1 then
      cmd = 0
      Usb.println "STOP"
    end
  end

  delay 250
end

System.exit


#digitalWrite(num[0],r0[0])  #A1
#digitalWrite(num[1],r1[0])  #A2
#digitalWrite(num[2],r1[0])  #B1
#digitalWrite(num[3],r0[0])  #B2
#for i in 1..maxVero do
#    delay 100
#    Usb.println(i.to_s)
#    pwm(vero[0], i)
#    pwm(vero[1], i)
#end
#p = maxVero
#200.times do
#    Usb.println(p.to_s)
#    pwm(vero[0], p)
#    pwm(vero[1], p)
#    p += 1
#    delay 250
#end
#Usb.println("Test End")
#System.exit

for i in 0..1 do
    k=0
    p=0
    Usb.println(str[i])
    digitalWrite(num[0],r0[i])  #A1
    digitalWrite(num[1],r1[i])  #A2
    digitalWrite(num[2],r0[i])  #B1
    digitalWrite(num[3],r1[i])  #B2
    maxVero.times do
        delay 10
        #Usb.println(p.to_s)
        pwm(vero[0], p)
        pwm(vero[1], p)
        p += 1
        led k
        k=1-k
    end
    5.times do
        delay 500
        led k
        k=1-k
    end
    maxVero.times do
        delay 10
        #Usb.println(p.to_s)
        pwm(vero[0], p)
        pwm(vero[1], p)
        p -= 1
        led k
        k=1-k
    end
end

for i in 0..1 do
    p = 0
    digitalWrite(num[0],r0[i])  #A1
    digitalWrite(num[1],r1[i])  #A2
    digitalWrite(num[2],r1[i])  #B1
    digitalWrite(num[3],r0[i])  #B2
    maxVero.times do
        delay 10
        pwm(vero[0], p)
        pwm(vero[1], p)
        p += 1
        led k
        k=1-k
    end
    10.times do
        delay 500
        led k
        k=1-k
    end
    maxVero.times do
        delay 10
        pwm(vero[0], p)
        pwm(vero[1], p)
        p -= 1
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

