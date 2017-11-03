#!mruby
@Usb = Serial.new(0)

#--------------------
# UTF-8の半角カナ半角カナをASCIIの半角カナに変換する
#--------------------
def hkana(utf8)
    kana = ""
    for i in 0..utf8.length - 1 do
        if((i%3) == 2)then
            if(utf8.bytes[i] < 0xA0)then
                kana += (utf8.bytes[i] + 0x40).chr
                #@Usb.println i.to_s + ";" + (utf8.bytes[i] + 0x40).to_s(16)
            else
                kana += utf8.bytes[i].chr
                #@Usb.println i.to_s + ":" + utf8.bytes[i].to_s(16)
            end
        end
    end
    kana
end


k = 1
10.times do |n|
    led k
    k = 1 - k
    @Usb.println "#{k.to_s}:Hello World! at #{System.getMrbPath}"
    @Usb.println hkana "ﾊﾛｰ､ﾜｶﾔﾏｱｰﾙﾋﾞｰ"

    delay 500
end
led 0