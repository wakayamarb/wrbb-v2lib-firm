#!mruby
serial = Serial.new(0)

a=4095

serial.println((a % 256).to_s)

serial.println((a/256).floor)

