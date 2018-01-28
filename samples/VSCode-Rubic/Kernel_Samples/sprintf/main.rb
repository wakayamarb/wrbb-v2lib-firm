#!mruby
#GR-CITRUS v2.31 mrbgem-sprintf

Usb = Serial.new(0)

Usb.println sprintf("price:%5d(tax:%d)",4000,200)
