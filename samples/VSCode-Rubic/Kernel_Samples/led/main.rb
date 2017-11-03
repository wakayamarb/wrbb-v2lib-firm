#!mruby
#GR-CITRUS v2.33

Usb = Serial.new(0)
#k = 0
#10.times do |n|
#    led k
#    k = 1 - k
#    Usb.println "Hello World! at #{System.getMrbPath}"
#    delay 500
#end

20.times do |n|
    led
    Usb.println "Hello Ruby World!"
    delay 250
end