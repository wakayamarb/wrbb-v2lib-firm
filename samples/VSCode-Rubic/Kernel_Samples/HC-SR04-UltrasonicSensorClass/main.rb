#!mruby
#v2.44

class UltrasonicSensor
  def initialize(trig_pin=8, echo_pin=9)
    @trig_pin, @echo_pin = trig_pin, echo_pin
    pinMode(@trig_pin, OUTPUT)
    pinMode(@echo_pin, INPUT)
  end

  def measure
    digitalWrite(@trig_pin, LOW)
    delayMicroseconds 2
    digitalWrite(@trig_pin, HIGH)
    delayMicroseconds 10
    digitalWrite(@trig_pin, LOW)
    _calculate pulseIn(@echo_pin, HIGH)
  end

  def _calculate(duration)
    return 0 if duration == 0
    # unit is centimetre.
    distance = duration / 2
    distance = (distance * 340 * 100 / 1000000).floor
  end
end

sensor = UltrasonicSensor.new

loop do
  puts "#{sensor.measure} cm"
  delay 500
end