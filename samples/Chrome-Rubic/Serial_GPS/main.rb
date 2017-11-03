#!mruby
# GPS module AE-GYSFDMAXB (http://akizukidenshi.com/catalog/g/gK-09991/)
# moduele::5V  -> 5V
# moduele::GND -> GND
# moduele::RXD -> TX1
# moduele::TXD -> RX1

debug = Serial.new(0, 115200)
gps = Serial.new(1, 9600)

debug.println("start")

sw = 0
buf = ""

loop do
    while (gps.available > 0) do
        c = gps.read
        buf = buf + c
    end

    lines = buf.to_s.split("\r\n")
    lines.each do |line|
        messages = line.split(',')

        # sample parse of NMEA0183 V3.01 format
        if messages[0] == '$GPGGA' && messages.length == 15
            # convert dddmm.mmmm format (NMEA0183) to ddd.dddd format (Google Map etc..)
            lat = messages[2].to_f / 100
            lon = messages[4].to_f / 100
            lat_d = lat.to_i
            lon_d = lon.to_i
            lat = lat_d + (lat - lat_d) * 100 / 60
            lon = lon_d + (lon - lon_d) * 100 / 60
            
            debug.println("lat,lon= #{lat},#{lon}")
            sw = 1 - sw
            led sw
        end

        buf = line
    end

    delay 1000

end
