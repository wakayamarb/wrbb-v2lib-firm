#!mruby
#Ver.2.50
Lev = [9,16,6]
for i in Lev do
  pinMode(i, INPUT_PULLUP) #プルアップ  入力設定
end

System.exit("SD Card can't use.") if(!System.use?'SD')
System.exit("MP3 can't use.") if(!System.use?('MP3', Lev))

MP3.play "mp3/001.mp3"
