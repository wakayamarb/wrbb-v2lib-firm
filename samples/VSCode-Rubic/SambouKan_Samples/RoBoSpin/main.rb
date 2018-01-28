#!mruby
#Ver.2.33
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
RotVero = 95
Rottime = 600
RotPm = [RotVero, Rottime]
Vero = [4,10]       #モータの速度を決定するGR-CITRUSのピンが4番と10番です。     
Num = [18,3,15,14]  #モータの回転方向などを制御するビット、1モータ2ビットです。18,3番、15と14番がペアです

Usb = Serial.new(0)
for i in Num do
 pinMode(i,OUTPUT)
end

class Rotation
  attr_accessor :pm # インスタンス変数@pmに対応するゲッタとセッタを定義
  def initialize(powers, controls)   # コンストラクタ
    @powers = powers
    @controls = controls   # @がつくのがインスタンス変数（メンバ変数）
  end

  #-------
  # 停止
  #-------
  def stop()
    pwm(@powers[0], 0)
    pwm(@powers[1], 0)
    digitalWrite(@controls[0],LOW)  #A1
    digitalWrite(@controls[1],LOW)  #A2
    digitalWrite(@controls[2],LOW)  #B1
    digitalWrite(@controls[3],LOW)  #B2
  end

  #-------
  # 回転
  # dir 0:右回転, 1:左回転
  #-------
  def rot(dir)
    led HIGH
    r0 = HIGH
    r1 = LOW
    if(dir == 1)then
      r0 = LOW
      r1 = HIGH
    end

    digitalWrite(@controls[0],r1)  #A1
    digitalWrite(@controls[1],r0)  #A2
    digitalWrite(@controls[2],r0)  #B1
    digitalWrite(@controls[3],r1)  #B2
    p = 0
    ps = 1
    (2 * @pm[0]).times do
      delay 5
      pwm(@powers[0], p)
      pwm(@powers[1], p)
      p += ps
      if(p == @pm[0])then
        delay @pm[1]
        ps = -1
      end
    end
  end
end

class NextTime
  attr_accessor :ntim # インスタンス変数に対応するゲッタとセッタを定義
  def initialize(n, span)   # コンストラクタ
    @Span = span
    @n = n
    @i = 0; @cnt = 0
    @ntim = millis + @Span[@i] * 1000
  end

  def update()
    if(@ntim < millis)then
      led
      @cnt += 1
      if(@cnt >= @n)then
        @cnt = 0
        @i += 1
        if(@i >= @Span.length)then
          @i = 0; @cnt = 0
          @ntim = millis + @Span[@i] * 1000
          return true
        end
      end
      @ntim = millis + @Span[@i] * 1000
    end
    return false
  end
end
#-----------------------------------------

Usb.println("System Start")
led HIGH
randomSeed(micros())  #乱数の初期化

Spin = Rotation.new(Vero, Num)
Spin.pm = RotPm
Spin.rot(random(2))
Spin.stop

Nt = NextTime.new(24, [1, 0.5, 0.25, 0.125, 0.063])

while(true) do
  if(Nt.update)then
    Spin.rot(random(2))
    Spin.stop
    led LOW
    #break
  end
end
