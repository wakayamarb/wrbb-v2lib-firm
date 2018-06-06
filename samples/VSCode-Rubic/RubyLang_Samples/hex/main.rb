#!mruby
#Ver2.48
#mruby-string-ext - String class extension
a = "0020FF\r"
x1 = a[0..1].hex
x2 = a[2..3].hex
x3 = a[4..5].hex.to_s(2)
puts x1
puts x2
puts x3

puts "FF".hex
