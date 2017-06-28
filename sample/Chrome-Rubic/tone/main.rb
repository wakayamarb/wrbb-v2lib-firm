#!mruby
pinMode(17,1)

tn = 440
10.times do
    tone(17,tn,2000)
    tn += 10
    delay 500
end
#delay 1000
tone(17,880,2000)
