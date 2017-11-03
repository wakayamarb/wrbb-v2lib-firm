#!mruby
#v2.35

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

Nt = NextTime.new(24, [1, 0.5, 0.25, 0.125, 0.063])
while(true) do
  if(Nt.update)then
    break
  end
end