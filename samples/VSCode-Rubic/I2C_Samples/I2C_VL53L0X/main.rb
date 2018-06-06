#!mruby
#Ver2.49

System.exit "VL53L0X can't use." if(!System.use?("VL53L0X"))

VL53L0X.init(1)
VL53L0X.startContinuous

while(true)
  puts VL53L0X.readContinuous
  #puts VL53L0X.readSingle
  if (VL53L0X.isTimeout)
    puts "TIMEOUT"
  end
  delay 0
end
