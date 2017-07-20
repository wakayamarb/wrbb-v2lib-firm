#!mruby
#v2.32

Usb = Serial.new(0)
3.times do |n|
    led
    Usb.println "#{n.to_s}:Hello World! at #{System.getMrbPath}"
    delay 500
end
led 0