Usb = Serial.new(0,115200)

readbuff = ""
last_sec = 0    #前の秒カウント値
command_get = 0 #コマンド取得フラグ

    Usb.println "Input Date and time. Example:"
                #0123456789012345678
    Usb.println "2018/ 2/ 9  2:52: 0;"
    Usb.println "-------------------"
    while(true) do
        while(true) do
            while(Usb.available()>0) do #何か受信があったらreadbuffに蓄える
                a = Usb.read()
                readbuff += a
                Usb.print a
                if a.to_s == ";" then
                    command_get = 1;                
                    break
                end
                delay 20
            end #while
            if command_get==1 then
                break
            end
        end #while
        if command_get==1 then
            command_get=0
            if(readbuff[4]=="/" and readbuff[7]=="/" and readbuff[10]==" " and readbuff[13]==":" and readbuff[16]==":") then
                if(readbuff.length >= 19 ) then
                    year = readbuff[0,4].to_i
                    mon	 = readbuff[5,2].to_i
                    da	 = readbuff[8,2].to_i
                    ho	 = readbuff[11,2].to_i
                    min	 = readbuff[14,2].to_i
                    sec	 = readbuff[17,2].to_i
                    Rtc.deinit()
                    #Rtc.init(-20)	# RTC補正：10 秒毎に 20/32768 秒遅らせる
                    Rtc.init()      # v2.83以降：デフォルト値(-20)で補正を行う
                    Rtc.setTime([year,mon,da,ho,min,sec])
                end
                year,mon,da,ho,min,sec = Rtc.getTime()
                Usb.println(" " + readbuff + " " + year.to_s + "/" + mon.to_s + "/" + da.to_s + " " + ho.to_s + ":" + min.to_s + ":" + sec.to_s)
                Usb.println "RTC setteing is done."
                break
            else
                Usb.println "Illegal command:"+readbuff
                readbuff = ""
            end #if
        end
    end #while
    while(true) do    
        year,mon,da,ho,min,sec = Rtc.getTime()
        if(last_sec != sec) then
            last_sec = sec
            Usb.println year.to_s + "/" + mon.to_s + "/" + da.to_s + " " + ho.to_s + ":" + min.to_s + ":" + sec.to_s
        end
        delay 20
    end #while

    System.exit()
