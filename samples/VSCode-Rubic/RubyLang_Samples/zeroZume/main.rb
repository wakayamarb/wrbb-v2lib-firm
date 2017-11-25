#!mruby
#GR-CITRUS v2.36
a = 123
s = "00000" + a.to_s
puts s[s.length-4..s.length]

