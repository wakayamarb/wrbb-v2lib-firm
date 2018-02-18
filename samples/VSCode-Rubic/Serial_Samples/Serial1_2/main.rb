#!mruby
#Ver.2.41
Usb = Serial.new(0,9600)
S1 = Serial.new(1,9600)
#S2 = Serial.new(2,9600)

Usb.println "Serial1 Test"

while(Usb.available > 0)do	#何か受信があった
  Usb.read
end
while(S1.available > 0)do	  #何か受信があった
  S1.read
end
#while(S2.available > 0)do	  #何か受信があった
#  S2.read
#end

50.times do
  S1.println "Serial_1"
  #S2.println "Serial_2"
  Usb.println "Usb"
  delay 1000
end

System.exit



# while true do
#   while(Usb.available() > 0)do
#     c = Usb.read()
#     Usb.write c, c.length
#     #Serial1.write(c,c.length)
#   end
#   delay 50
# end



while true do
  while(Usb.available() > 0)do
    c = Usb.read()
    #Usb.println c.length.to_s
    S1.write(c,c.length)
  end

  while(S1.available() > 0)do
    c = S1.read()
    Usb.write(c,c.length)
  end
  delay 50
end
