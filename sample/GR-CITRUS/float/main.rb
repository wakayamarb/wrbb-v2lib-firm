#!mruby
serial = Serial.new(0)

serial.println("NG") if 0.5 == 0

serial.println("2.0 => #{2.0}")
