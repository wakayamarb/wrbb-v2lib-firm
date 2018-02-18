#!mruby
#GR-CITRUS v2.41
puts Math::PI
puts Math::sqrt(2)
deg = 30
rad = ( deg * Math::PI/ 180.0 ) # 度->ラジアン変換
puts Math.sin(rad).to_s
puts Math.cos(rad).to_s
puts Math.tan(rad)
puts Math.asin(rad)
puts Math.acos(rad)
puts Math.atan(rad)
puts Math.exp(1)
puts Math.log(10)
