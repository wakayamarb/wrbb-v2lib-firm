#!mruby
#################################
# フードシュータ
# GWS S03N 2BBMG
# オレンジ・・・信号線
# 赤・・・・・・3.3V
# 茶・・・・・・GND
#################################
@SrvPin = 0
Usb = Serial.new(0)
led 1

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,1)
digitalWrite(5,0)   # LOW:Disable
delay 500
digitalWrite(5,1)   # LOW:Disable
if( System.useWiFi() == 0)then
    Usb.println "WiFi Card can't use."
   System.exit() 
end

Usb.println WiFi.disconnect
Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
#WiFi.connect "TAROSAY", "37000"
Usb.println WiFi.connect("000740DE0D79","")
Usb.println WiFi.ipconfig
Usb.println WiFi.multiConnect 1

#0番ピンをサーボ制御ピンに設定
pinMode(0, 1)
Servo.attach(0, @SrvPin)
Servo.write(0, 0) #0度にする

Usb.println WiFi.httpServer(-1).to_s
delay 100

body0 = '<html><head><meta http-equiv="Content-Type" content="text/html; charset=utf-8">'
body0 += '<title>RAMUNE SHOOTER</title></head>'
#body1 = '<body><h1 align="center">ラムネ撃ちます。</h1></body>'
#body1 += "\r\n\r\n"

#Usb.print body
#Usb.println "body.length = " + body.length.to_s

header0 = "HTTP/1.1 200 OK\r\n"
header0 += "Server: GR-CITRUS\r\n"
header0 += "Content-Type: text/html\r\n"
header0 += "Date: Sun, 13 Nov 2016 12:00:00 GMT\r\n"
header0 += "Connection: close\r\n"
#header1 += "Content-Length: " + (body0 + body1).length.to_s + "\r\n\r\n"


Usb.println WiFi.httpServer(80).to_s

led 0
k = 0
while(true)do
    res = WiFi.httpServer
    #Usb.println res.to_s
    
    if(res == "/ramune")then
        Usb.println res
	    led 1

        body1 = '<body><h1 align="center">ラムネ撃ちます。</h1></body>'
        body1 += "\r\n\r\n"
        
        header1 = "Content-Length: " + (body0 + body1).length.to_s + "\r\n\r\n"
        
        WiFi.send(0, header0)
        WiFi.send(0, header1)
        WiFi.send(0, body0)
        WiFi.send(0, body1)

		Servo.write(0, 0)
		delay(1000)
				
		16.times {|i|
			Servo.write(0, i*10)
			delay(300)
		}				
    elsif(res == "/ramune?exit")
        Usb.println res
        body1 = '<body><h1 align="center">終了します。</h1></body>'
        body1 += "\r\n\r\n"
        
        header1 = "Content-Length: " + (body0 + body1).length.to_s + "\r\n\r\n"
        
        WiFi.send(0, header0)
        WiFi.send(0, header1)
        WiFi.send(0, body0)
        WiFi.send(0, body1)
        break
    elsif(res == "0,CLOSED\r\n")
        Usb.println res

    elsif(res.to_s.length > 2 && ((res.bytes[0].to_s + res.bytes[1].to_s  == "0,") || (res.bytes[0].to_s + res.bytes[1].to_s  == "1,")))
        Usb.println "Else(*,:" + res
    
    elsif(res != 0)
        Usb.println "Else:" + res.to_s
        
        body1 = '<body><h1 align="center">エラーです。</h1></body>'
        body1 += "\r\n\r\n"
        
        header1 = "Content-Length: " + (body0 + body1).length.to_s + "\r\n\r\n"
        
        WiFi.send(0, header0)
        WiFi.send(0, header1)
        WiFi.send(0, body0)
        WiFi.send(0, body1)
    end
    delay 100
    k = 1 - k
    led k
end

WiFi.httpServer(-1)
WiFi.disconnect
digitalWrite(5,0)   # LOW:Disable

