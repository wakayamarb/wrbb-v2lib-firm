#!mruby
#GR-CITRUS v2.42 mrbgem-Fiber

fiber = Fiber.new do
	puts "First"
	Fiber.yield
	puts "Second"
end

fiber.resume # "Fisrt"
fiber.resume # "Second"

fiber = Fiber.new do |num|
	num.times do |i|
		Fiber.yield i
	end
end

puts (fiber.resume 3) # 0
puts fiber.resume   # 1
puts fiber.resume   # 2
puts fiber.resume   # 3
puts fiber.resume   # FiberError
