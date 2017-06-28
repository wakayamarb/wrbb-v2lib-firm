#!mruby
Usb = Serial.new(0)
a = 123
s = "00000"+a.to_s
Usb.println s[s.length-4..s.length]

