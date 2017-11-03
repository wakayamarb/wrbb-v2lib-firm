#!mruby
#GR-CITRUS Version 2.28
Lev = [1,16]        #ロボホンロボホンのレバー
WiFiEN = 5          #WiFiのEN:LOWでDisableです

for i in Lev do
    pinMode(i, 2) #INPUT & プルアップ
end
pinMode(WiFiEN, OUTPUT)

#ESP8266をDisableにする
digitalWrite(WiFiEN, LOW)   #LOW:Disable

Usb = Serial.new(0,115200)

if(!System.use?('MP3', Lev))then
    Usb.println "MP3 can't use."
   System.exit() 
end
Usb.println "MP3 Ready"

#-------
# レバー状態状態を取得します
#-------
def lever()
  3 - digitalRead(Lev[0]) - digitalRead(Lev[1]) - digitalRead(Lev[1])
end

MP3.led 1
i = 0
while(true)do
  n = '0'+i.to_s
  MP3.play "music/" + n[n.size-2..n.size] + '.mp3'
  i += 1
  if(i > 20)then
      i = 0
  end

  if(lever() == 3)then
    while(lever() != 0)do end
    System.setrun 'wifiload.mrb'
    System.exit
  end 
end
