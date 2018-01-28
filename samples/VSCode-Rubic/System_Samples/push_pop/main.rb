#!mruby
#GR-CITRUS Version 2.36

#フラッシュメモリの 0x0000番から値を読みだします。
#2バイトのデータを読み込みます
ans = System.pop(0x0000, 2)
puts "0x0000～0x0001: " + ans
puts "0x0000: " + ans.bytes[0].to_s(16)
puts "0x0001: " + ans.bytes[1].to_s(16)

#0x0000から 2バイト分のデータを書きます
buf = "\x30\x31"
System.push( 0x0000, buf, 2 )

#2バイトのデータを読み込みます
ans = System.pop(0x0000, 2)
puts "0x0000～0x0001: " + ans
puts "0x0000: " + ans.bytes[0].to_s(16)
puts "0x0001: " + ans.bytes[1].to_s(16)

#3バイトのデータを読み込みます
ans = System.pop(0x0002, 3)
puts "0x0002: " + ans.bytes[0].to_s(16)
puts "0x0003: " + ans.bytes[1].to_s(16)
puts "0x0004: " + ans.bytes[2].to_s(16)

num = ans.bytes[0] + ans.bytes[1] * 256 + ans.bytes[2] * 65536 + 1
puts "Number= " + num.to_s

buf = (num & 0xff).chr + ((num >> 8) & 0xff).chr + ((num >> 16) & 0xff).chr

#buf = 0xff.chr + 0xff.chr + 0xff.chr
System.push( 0x0002, buf, 3 )
#電源を切っても覚えているはずです
