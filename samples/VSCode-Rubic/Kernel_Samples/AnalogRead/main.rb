#!mruby
#Ver.2.35
Sens = 17            #アナログ距離センサ
Usb = Serial.new(0,115200)

500.times do
  Usb.println analogRead(Sens).to_s
  delay 500
end
