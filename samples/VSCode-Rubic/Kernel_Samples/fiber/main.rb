#!mruby
#GR-CITRUS v2.31 mrbgem-Fiber
Usb = Serial.new(0)

fiber = Fiber.new do
	Usb.println "First"
	Fiber.yield
	Usb.println "Second"
end

fiber.resume # "Fisrt"
fiber.resume # "Second"

fiber = Fiber.new do |num|
	num.times do |i|
		Fiber.yield i
	end
end

Usb.println (fiber.resume 3).to_s # 0
Usb.println fiber.resume.to_s   # 1
Usb.println fiber.resume.to_s   # 2
Usb.println fiber.resume.to_s   # 3
Usb.println fiber.resume.to_s   # FiberError
