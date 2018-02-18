#!mruby
#v2.42

Usb = Serial.new(0)
8.times do |n|
    led
    Usb.println "#{n.to_s}:Hello World! at #{System.getMrbPath}"
    delay 500
end
led 0