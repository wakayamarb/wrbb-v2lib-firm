#!mruby
#GR-CITRUS Version 2.41

if(!System.use?("SD"))then
  puts "SD Card can't use."
  System.exit 
end
puts "SD Ready"

if(!System.use?("WiFi"))then
  puts "WiFi can't use."
  System.exit 
end
puts "WiFi Ready"

if(!System.use?("MP3",[3,4]))then
  puts "MP3 can't use."
  System.exit() 
end
puts "MP3 Ready"


