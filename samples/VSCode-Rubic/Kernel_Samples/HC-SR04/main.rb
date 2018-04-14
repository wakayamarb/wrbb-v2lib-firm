#!mruby
#v2.44

trigPin = 8
echoPin = 9

pinMode(trigPin, OUTPUT)
pinMode(echoPin, INPUT)

loop do
  digitalWrite(trigPin, LOW)
  delayMicroseconds 2
  digitalWrite(trigPin, HIGH)
  delayMicroseconds 10
  digitalWrite(trigPin, LOW)
  duration = pulseIn(echoPin, HIGH)
  if duration > 0
    distance = duration / 2
    distance = (distance * 340 * 100 / 1000000).floor
    puts "#{duration} us : #{distance} cm"
  end
  delay 500
end