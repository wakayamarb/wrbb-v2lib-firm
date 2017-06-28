#!mruby
#GR-CITRUS v2.31

Usb = Serial.new(0)
k = 0
10.times do |n|
    led k
    k = 1 - k
    Usb.println "Hello World! at #{System.getMrbPath}"
    delay 500
end

10.times do |n|
    led
    Usb.println "Hello World! at #{System.getMrbPath}"
    delay 500
end