
  
puts '1'.to_s()
puts 1.to_s
puts nil.to_s
puts [true.to_s, false.to_s]
puts 10.to_s(16).upcase
puts '123'

s = 'HELLO'
n = 123 * 2
puts s
puts n

x = nil

puts (_special_price = 200)
puts _special_price_2 = 300

puts 特別価格 =200
puts 特別価格 * 2

puts x = 'HELLO'
puts x = 123

puts a = b = 2
puts 'HEE表\nLO'
puts "HEL\nLO"
name = 'Yamamoto'
puts "Hello, #{name}!"
i=10
puts "#{i.to_s(16).upcase}"
puts 'Hello,' + name + '!'
puts "Hello, \#{name}!"
puts 'He said, "Don\'t spaek."'
puts "He said, \"Don't spaek.\""








System.exit



def foo()
  "sgsg"
end

puts foo

puts 10.to_s\
(16)

=begin
puts 'hdhdhdhdh'
=end

puts 'abc'.class
puts 'abc'

puts %q!abc!
puts %Q!abc!
puts %!abc!

puts "line 1,
line 2"

a = 'Ruby'
a.prepend(<<TEXT)
Java
PHP
TEXT
puts a

b = <<TEXT.upcase
Hello,
Good-bye
TEXT

puts b

puts sprintf('%0.3f', 1.2)
puts '%0.3f' % 1.2
puts '%0.3f + %0.3f' % [1.2, 0.48]
puts [10,20,30].join
puts 'Hi!' * 10
puts "\u3042\u3044\u3046"
puts 2e-3
puts 10.class

n = 10
puts n > 10 ? '10より大きい' : '10以下'

def foo(mes=bar)
  puts bar
end

def bar
  'BAR'
end

foo

puts ''.empty?.to_s
puts 'ss'.empty?.to_s
puts 'watch'.include?('at').to_s
puts 'watch'.include?('in').to_s
puts nil.nil?.to_s
a = 'ruby'
puts a.upcase
puts a
puts a.upcase!
puts a
puts __FILE__
puts __LINE__

a = 'abcd'
b = 'abcd'

puts a.object_id
puts b.object_id

alias print puts
Count = 1
while Count <= 3 do
  print('Hello World!')
  Count = Count + 1
end

a = {'japan' => [10,2,3,4,5]}
puts a['japan'][0]


