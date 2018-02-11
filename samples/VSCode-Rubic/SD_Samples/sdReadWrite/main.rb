#!mruby
#GR-CITRUS Version 2.41

if(!System.use?('SD'))then
  puts "SD can't use."
  System.exit() 
end

doc = <<EOS
あなたの背中のテレビ
中央フリーウェイへ行こうって言うあなた
CITRUSのテレビ

まぶしすぎて
泣いていた

忘れられない
憧れの中央フリーウェイ
切ない気持ちはCITRUS

あなたの背中のCITRUS
EOS

fn = SD.open(0, 'song.txt',2)
if(fn < 0)then
  System.exit
end

SD.write(fn, doc, doc.length)
SD.close(fn)

#保存したhtmlファイルの読込み
fn = SD.open(0, 'song.txt')
if(fn < 0)then
  System.exit
end

Usb = Serial.new(0)

while(true)do
  c = SD.read(fn)
  if(c < 0)then
    break
  end
  Usb.print c.chr
end
SD.close(fn)
