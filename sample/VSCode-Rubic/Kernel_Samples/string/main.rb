#!mruby
#GR-CITRUS v2.31 mrbgem-String, mrbgem-sprintf

Usb = Serial.new(0)

Usb.println 'String#strip'
Usb.println " \t\r\n\f\v".strip
Usb.println "\0a\0".strip
Usb.println "abc".strip
Usb.println "  abc".strip
Usb.println "abc  ".strip
Usb.println "  abc  ".strip

Usb.println 'String#swapcase'
Usb.println "Hello".swapcase
Usb.println "cYbEr_PuNk11".swapcase

Usb.println "a0".succ
