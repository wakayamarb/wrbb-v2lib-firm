#!mruby
#Ver.2.32
MaxVero = 100
RotVero = 100
Rottime = 1500
RotPm = [RotVero, Rottime]

Usb = Serial.new(0)

#-------
# タンク前進させます
#-------
def mstart()
  p = 0
  Motor.ccw
  MaxVero.times do
    delay 5
    Motor.speed(1, p)
    Motor.speed(2, p)
    p += 1
  end
end
#-------
# タンクを、t ms回転させます
#-------
def rot(r, pm)
  led HIGH
  if(r == 0)then
    Motor.cw 1
    Motor.ccw 2
  else
    Motor.ccw 1
    Motor.cw 2
  end    
  p = 0
  ps = 1
  (2 * pm[0]).times do
    delay 5
    Motor.speed(1, p)
    Motor.speed(2, p)
    p += ps
    if(p == pm[0])then
      delay pm[1]
      ps = -1
    end
  end
end
#-----------------------------------------


Usb.println("System Start")

mstart
delay 5000
Motor.brake
rot(0, RotPm)   #Left Rotation
rot(1, RotPm)   #Right Rotation

Motor.speed(1, 0)
Motor.speed(2, 0)
Motor.free