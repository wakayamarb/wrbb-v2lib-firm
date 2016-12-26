#!mruby
# Ver 2.23
Usb = Serial.new(0)
Usb.println(ON.to_s)    #-> 1
Usb.println(OFF.to_s)   #-> 0
Usb.println(HIGH.to_s)  #-> 1
Usb.println(LOW.to_s)   #-> 0
Usb.println(OUTPUT.to_s)   #-> 1
Usb.println(INPUT.to_s)  #-> 0

LOW = 3
Usb.println(LOW.to_s)   #-> 3
