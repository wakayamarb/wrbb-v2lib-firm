Wakayama.rb Ruby Board V2 library Firmware
------
　Wakayama.rb のV2ライブラリを使ったRubyボードのソースです。
　V2ライブラリとは、ルネサスさんが提供しているRX63NのV2ライブラリを示します。

  バージョンのARIDAやUmeJamの後の数字は動作する基板種類を表します。GR-CITRUSはARIDA5に相当します。
  バージョン最後のf2やf3はバイトコードフォーマットの番号です。mruby1.0.0ベースの場合はf2となります。

  mruby ver1.0.0 -> ByteCode Format Ver.0002  
  mruby ver1.1.0 -> ByteCode Format Ver.0003  
  mruby ver1.2.0 -> ByteCode Format Ver.0003  

    WAKAYAMA.RB Board Ver.ARIDA5-1.52(2016/3/19)f3(256KB), mruby 1.2.0
                            |      |      |      |   |
                            |      |      |      |   |
                            |      |      |      |  RAM Size
                            |      |      |      |
                            |      |      |   ByteCode Format Number
                            |      |      |
                            |      |    作成日
                            |      |
                            | Firmware Version
                            |
                       回路基板種類


How to use Wakayama.rb board
------
https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/description/Ruby%20Firmware%20on%20GR-CITRUS.pdf
http://www.slideshare.net/MinaoYamamoto/grcitrusruby

Link
------


License
------
 Wakayama.rb-Ruby-Board-V2-library-Firmware is released under the [MIT License](MITL).
