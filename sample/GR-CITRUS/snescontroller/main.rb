
#!mruby


#黒電源
#ない　GND
#アカDAT
#市白　clk
#緑　PS
DAT = 3
PS = 4
CLK = 5
Spe = 0
Da = 261
Re = 293
Mi = 329
Fa = 349
So = 392
Ra = 440
Ti = 493
Uda = 523
Ida = 523
Ire = 587
Imi = 659
Ifa = 698
Iso = 783
Ira = 879
Iti = 987
Iuda = 1046
Ihda = 554
Ihre = 622
Ihmi = 670
Ihfa = 739
Ihso = 830
Ihra = 932
Ihti = 987
Hda = 277
Hre = 293
Hmi = 330
Hfa = 369
Hso = 415
Hra = 466
Hti = 510
Nhda = 1108
Ma =60
LOW = 0
HUGH = 1
INPUT=0
OUTPUT=1
pinMode(Spe,1)
pinMode(DAT,0)
pinMode(CLK,1)
pinMode(PS,1)
tone(Spe,Uda,900)
delay(900)
po=0    
pi=0
 loop{
    
    digitalWrite(PS,1)
    digitalWrite(CLK,1)
    digitalWrite(PS,0)
    digitalWrite(CLK,0)
    if (digitalRead(DAT) === 0) then

          
        if(po === 1)then
                        
            tone(Spe,Iuda)
            delay(Ma)
            noTone(Spe)
        else
               
            tone(Spe,Uda)
            delay(Ma)
            noTone(Spe)
        end
    end
    for h in 2..16 
        noTone(Spe)
        digitalWrite(CLK,1)
        digitalWrite(CLK,0) 
        if(h == 11)
          po=0
          pi=0
        end
            
        if(digitalRead(DAT)==0 && po==0 && pi==0)

            case h
            when 11
                pi=1
                       
            when 12
                po=1             #一オクターブあげるフラグ
                       
            when 8
                tone(Spe,Da)#40
                delay(Ma)
                       
            when 5
                tone(Spe,Re)
                delay(Ma)
                       
            when 7
                tone(Spe,Mi)
                delay(Ma)
                       
            when 6
                tone(Spe,Fa)                    
                delay(Ma)
                       #50
            when 2
                tone(Spe,So)
                delay(Ma)
                       
            when 10
                tone(Spe,Ra)
                delay(Ma)
                       
            when 9
               tone(Spe,Ti)
               delay(Ma)
                       
            when 4
                tone(Spe,900)
                delay(Ma)
                       
            end
          
               
                    
                   
        noTone(Spe)   
        elsif (digitalRead(DAT)==0 && po==1 &&pi==0)
                    
                    
            case h
                when 11
                pi=1
                       
                when 12
                    po=1             #一オクターブあげるフラグ
                       
                when 8
                    tone(Spe,Ida)#40
                    delay(Ma)
                       
                when 5
                    tone(Spe,Ire)
                    delay(Ma)
                       
                when 7
                    tone(Spe,Imi)
                    delay(Ma)
                       
                when 6
                    tone(Spe,Ifa)                    
                    delay(Ma)
                       #50
                when 2
                    tone(Spe,Iso)
                    delay(Ma)
                       
                when 10
                    tone(Spe,Ira)
                    delay(Ma)
                       
                when 9
                    tone(Spe,Iti)
                    delay(Ma)
                       
                when 4
                    tone(Spe,1900)
                    delay(Ma)
                       
                end
                noTone(Spe)    
                                 
                   
            elsif(digitalRead(DAT)== 0 && po==1 &&pi==1 )
                    
                    
                case h 
                when 11
                    pi=1
                       
                when 12
                    po=1             #一オクターブあげるフラグ
                       
                when 8
                    tone(Spe,Ihda)#40
                    delay(Ma)
                       
                when 5
                    tone(Spe,Ire)
                    delay(Ma)
                       
                when 7
                    tone(Spe,Ihmi)
                    delay(Ma)
                       
                when 6
                    tone(Spe,Ihfa)                    
                    delay(Ma)
                       #50
                when 2
                    tone(Spe,Ihso)
                    delay(Ma)
                       
                when 10
                    tone(Spe,Ihra)
                    delay(Ma)
                       
                when 9
                    tone(Spe,Ihti)
                    delay(Ma)
                       
                when 4
                    tone(Spe,19000)
                    delay(Ma)
                       
                end
                noTone(Spe)
    
            
                 
     
            elsif(digitalRead(DAT) == 0 && po==0 && pi==1)
                    

            case h
                when 11
                    pi=1
                       
                when 12 
                    po=1             #一オクターブあげるフラグ
                       
                when 8
                    tone(Spe,Hda)#40
                    delay(Ma)
                       
                when 5
                    tone(Spe,Hre)
                    delay(Ma)
                       
                when 7
                    tone(Spe,Hmi)
                    delay(Ma)
                       
                when 6
                    tone(Spe,Hfa)                    
                    delay(Ma)
                       #50
                when 2
                    tone(Spe,Hso)
                    delay(Ma)
                       
                when 10
                    tone(Spe,Hra)
                    delay(Ma)
                       
                when 9
                    tone(Spe,Hti)
                    delay(Ma)
                      
                when 4
                    tone(Spe,4339)
                    delay(Ma)
                

            end
            noTone(Spe)
        
        end
    end
}
 
 
 