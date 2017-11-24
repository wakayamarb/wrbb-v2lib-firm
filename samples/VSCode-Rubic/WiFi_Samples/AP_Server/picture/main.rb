#!mruby
#GR-CITRUS Version 2.36

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,1)
digitalWrite(5,0)   # LOW:Disable
delay 500
digitalWrite(5,1)   # LOW:Disable
delay 500

if(!System.use?('SD'))then
	puts "SD Card can't use."
	System.exit() 
end
puts "SD Card Ready"
if(!System.use?('WiFi'))then
  puts "WiFi can't use."
  System.exit() 
end
puts "WiFi Ready"

puts "WiFi disconnect"
puts WiFi.disconnect

puts "WiFi Mode Setting"
puts WiFi.setMode 3  #Station-Mode & SoftAPI-Mode

puts "WiFi access point"
#puts WiFi.softAP "GR-CITRUS","37003700",2,3
puts WiFi.softAP "192.168.4.1","",1,0

puts "WiFi dhcp enable"
puts WiFi.dhcp 0,1

puts WiFi.ipconfig

puts "WiFi multiConnect Set"
puts WiFi.multiConnect 1

puts WiFi.httpServer(-1).to_s

puts "BASE64 Start"
puts WiFi.base64("SAMPLE.jpg", "SAMPLE.b64")
puts "BASE64 Finish"

fn = SD.open(0, "SAMPLE.b64") #新規オープン
if(fn >= 0)then
	b64size = SD.size(fn)
	SD.close(fn)
end

body0 = '<html><img src="data:image/jpeg;base64,'
body1 = '" /></html>'

#ヒアドキュメント書式を知りませんでした
header = <<EOS
HTTP/1.1 200 OK
Server: GR-CITRUS
Content-Type: text/html
Accept-Ranges: bytes
Connection: keep-alive
EOS
header += "Content-Length: " + (body0.length + body1.length + b64size).to_s + "\r\n\r\n"


puts WiFi.httpServer(80).to_s
while(true)do
	res, sesnum = WiFi.httpServer
	#Usb.println res.to_s
	if(res == "/")then
		puts res

		fname = "SAMPLE.b64"

		puts header
		WiFi.send(sesnum, header)
		
		fn = SD.open(0, fname) #新規オープン
		if(fn >= 0)then
			WiFi.send(sesnum, body0)
			ch = 0
			tsize = 0
			while(ch >= 0)do
			 	jpgdata = ""
			 	for i in 1..1024 do
			 		ch = SD.read(fn)
			 		if(ch < 0)then
			 			break
			 		end
			 		jpgdata += ch.chr
			 		if((i % 32) == 0)then
						 GC.start
						 #led
			 		end
			 	end
				if(jpgdata.length > 0)then
					WiFi.send(0, jpgdata)
				end
				GC.start
				led
				tsize += jpgdata.length
				#puts tsize.to_s
			end
			WiFi.send(sesnum, body1)
			SD.close(fn)
		end
  elsif(res == "0,CLOSED\r\n")
    puts res
  
  elsif(res.to_s.length > 2 && ((res.bytes[0].to_s + res.bytes[1].to_s  == "0,") || (res.bytes[0].to_s + res.bytes[1].to_s  == "1,")))
    puts "Else(*,:" + res
    
  elsif(res != 0)
    puts "Else:" + res.to_s
        
    #header = header0
    #header += "Content-Length: " + body.length.to_s + "\r\n\r\n"
    #WiFi.send(0, header)
    #WiFi.send(0, body)
  end
  delay 0
end
  
WiFi.httpServer(-1)
puts WiFi.disconnect