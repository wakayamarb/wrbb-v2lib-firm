#!mruby
#################################
# ラムネ発射器
# GWS S03N 2BBMG
# オレンジ・・・信号線
# 赤・・・・・・3.3V
# 茶・・・・・・GND
#################################
#GR-CITRUS Version 2.26

Usb = Serial.new(0,115200)

if(System.use?("WiFi") == false)then
  Usb.println "WiFi Card can't use."
  System.exit() 
end
Usb.println "WiFi Ready"

if(System.use?("MP3",[3,4]) == false)then
  Usb.println "MP3 can't use."
  System.exit() 
end
Usb.println "MP3 Ready"
MP3.led ON
MP3.play "voice/title.mp3"

#18番ピンをサーボ制御ピンに設定
@SrvPin = 18
pinMode(@SrvPin, OUTPUT)
Servo.attach(0, @SrvPin)
Servo.write(0, 4) #4度にする

Usb.println WiFi.disconnect
Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
#Usb.println WiFi.connect("TAROSAY","37000")
Usb.println WiFi.connect("000740DE0D79","")

def speakip()
    res = WiFi.ipconfig
    s = res.index("STAIP,") + 7
    e = res.index("+CIFSR:STAMAC") - 4

    MP3.play "voice/ip.mp3"
    for i in s..e do
        if(res.bytes[i].chr == ".")then
            MP3.play "voice/ten.mp3"
        else
          MP3.play "voice/" + res.bytes[i].chr + ".mp3"
        end
        delay 1
    end
    MP3.play "voice/desu.mp3"
end

2.times do
    speakip()
end

led 1
Usb.println WiFi.multiConnect 1

MP3.play "voice/access.mp3"


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

k = 0
while(true)do
    res = WiFi.httpServer
    #Usb.println res.to_s
    
    if(res == "/ramune")then
        Usb.println res
	    led 1

        body1 = '<body><h1 align="center">ラムネを発射します。</h1></body>'
        body1 += "\r\n\r\n"
        
        header1 = "Content-Length: " + (body0 + body1).length.to_s + "\r\n\r\n"
        
        WiFi.send(0, header0)
        WiFi.send(0, header1)
        WiFi.send(0, body0)
        WiFi.send(0, body1)

        MP3.play "voice/ramune.mp3"

		Servo.write(0, 0)
		delay(1000)
				
		16.times {|i|
			Servo.write(0, i*10)
			delay(300)
		}
        k = 0
    elsif(res == "/ramune?exit")
        Usb.println res
        body1 = '<body><h1 align="center">終了いたします。</h1></body>'
        body1 += "\r\n\r\n"
        
        header1 = "Content-Length: " + (body0 + body1).length.to_s + "\r\n\r\n"
        
        WiFi.send(0, header0)
        WiFi.send(0, header1)
        WiFi.send(0, body0)
        WiFi.send(0, body1)

        MP3.play "voice/owari.mp3"
        break
    elsif(res == "/ramune?ip")
        Usb.println res
        
        ipadd = WiFi.ipconfig
        s = ipadd.index("STAIP,") + 7
        e = ipadd.index("+CIFSR:STAMAC") - 4
        str = ""
        for i in s..e do
            str += ipadd.bytes[i].chr
        end
        body1 = '<body><h1 align="center">IPアドレスは、' + str + 'です。</h1></body>'
        body1 += "\r\n\r\n"
        
        header1 = "Content-Length: " + (body0 + body1).length.to_s + "\r\n\r\n"
        
        WiFi.send(0, header0)
        WiFi.send(0, header1)
        WiFi.send(0, body0)
        WiFi.send(0, body1)
        speakip()
        k = 0
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

        MP3.play "voice/error.mp3"
        k = 0
    end
    delay 0
    k += 1
    MP3.play 'ppap/' + k.to_s + '.mp3'
    if(k == 8)then
        k = 0
    end
end

Servo.write(0, 4) #4度にする
Servo.detach(0)
pinMode(@SrvPin, INPUT)

WiFi.httpServer(-1)
Usb.println WiFi.disconnect
