#!mruby
#Ver.2.42
MaxVero = 100
RotVero = 100
Rottime = 300
RotPm = [RotVero, Rottime]

#-------
# タンク前進させます
#-------
def mstart(g)
  p = 0
  if(g == 0)then
    Motor.ccw
  else
    Motor.cw
  end
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


puts "System Start"

#mstart 0
#delay 2000
#Motor.brake

#mstart 1
#delay 2000
#Motor.brake

4.times do
  rot(0, RotPm)   #Left Rotation
  rot(1, RotPm)   #Right Rotation
end

rot(0, [250, 10000])   #Left Rotation

Motor.speed(1, 0)
Motor.speed(2, 0)
Motor.free