@DATA_PIN  = 15
@RATCH_PIN = 14
@CLOCK_PIN = 13

class SN74HC595N
    attr_accessor :data_pin, :ratch_pin, :clock_pin, :mode
    def initialize(data_pin, ratch_pin, clock_pin)
        @data_pin = data_pin
        @ratch_pin = ratch_pin
        @clock_pin = clock_pin
        @mode = MSBFIRST

        # ピンを初期化
        [@data_pin, @ratch_pin, @clock_pin].each do |p|
            pinMode(p, OUTPUT)
            digitalWrite(p, 0)
        end
    end

    def output(value)
        # ラッチをHIGHにすると書き込める
        digitalWrite(@ratch_pin, 1)

        # 書き込み
        shiftOut(@data_pin, @clock_pin, @mode, value)

        # ラッチをLOWに戻す
        digitalWrite(@ratch_pin, 0)
    end

    def clear
        output(0x00)
    end
end

sr = SN74HC595N.new(@DATA_PIN, @RATCH_PIN, @CLOCK_PIN)
sr.clear

256.times do |i|
    sr.output(i)
    delay(25)
end

# シフトレジスタなので次のデータを入れないと最後のデータが出力されない
sr.clear()