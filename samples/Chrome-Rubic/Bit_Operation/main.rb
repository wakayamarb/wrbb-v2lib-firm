#!mruby
Usb = Serial.new(0)


Usb.println (1 << 1).to_s
Usb.println (0x01 << 1).to_s
Usb.println (0b0000_0001 << 1).to_s

Usb.println (8 >> 1).to_s
Usb.println (0x08 >> 1).to_s
Usb.println ((0b0000_1000) >> 1).to_s

