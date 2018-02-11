#!mruby

pinMode(2,OUTPUT)
pinMode(3,INPUT)

digitalWrite(2,LOW) #LOWにする
delay 1
digitalWrite(2,HIGH)
delayMicroseconds  100  #100us 後にLOEにする
digitalWrite(2,LOW)
puts pulseIn(3,HIGH)    #HIGHになっていた時間を取得して表示する
