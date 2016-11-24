#!mruby
# v2.18
usb = Serial.new(0)
usb.println(Math::PI.to_s)
usb.println(Math::sqrt(2).to_s)
deg = 30
rad = ( deg * Math::PI/ 180.0 ) # 度->ラジアン変換
usb.println(Math.sin(rad).to_s)
usb.println(Math.cos(rad).to_s)
usb.println(Math.tan(rad).to_s)
usb.println(Math.asin(rad).to_s)
usb.println(Math.acos(rad).to_s)
usb.println(Math.atan(rad).to_s)
usb.println(Math.exp(1).to_s)
usb.println(Math.log(10).to_s)
