# include "qsy.c"


int main(void){
	Initialize_All();
	int hour = 19, min = 15, sec = 2, msec = 2;
	int year = 2020, month = 6, day = 20;
	SutdentNum_flag = true;
	Set_Date(year, month, day);
	Set_Time(hour, min, sec, msec, apm);
	Display_StudentNum();
		
	while(1){	
		Pulse_Frequency = 10;
		if(systick_ms_status)	{systick_ms_status = 0; _msec = _msec + 1;}
		Set_Time(_hour, _minute, _second, _msec, apm);
		Set_Date(_year, _month, _day);
	
		Set_Arm1();
		Set_Arm2();	
		
		Update_Date();
		Update_Time();
		Update_APM();
		Update_DayNight();
		Update_LED();
		Update_SW();
				
		Update_Button_Status("USR_SW1",6);
		switch(Button_Status)
		{		
			case 0:		// 时间显示
				if(!nightflag) Display_Time();	
				break;
			
			
			case 1:		// 日期显示
				if(!nightflag) Display_Date();
				break;
			
			
			case 2:		// 闹钟1设置
				if(!nightflag) {
					Display_Arm1();
					Motor_Enable(); }
				break;
			
			
			case 3:		// 闹钟2设置
				if(!nightflag) {
					Display_Arm2();
					Motor_Enable(); }
				break;
			
			
			case 4:		// 时间设置
				if(!nightflag) {
					Twinkle_Minute();
					if(keySW_pressed[5] || keySW_pressed[6]) Twinkle_Hour(); }
				break;
			
			
			case 5:		// 日期设置
				if(!nightflag) {
					Twinkle_Day();
					if(keySW_pressed[3] || keySW_pressed[4]) Twinkle_Month(); }
				break;
		}
			
	}
}




void UART0_Handler(void) {
		int32_t uart0_int_status;
		uart0_int_status = UARTIntStatus(UART0_BASE, true);		// Get the interrrupt status.
		UARTIntClear(UART0_BASE, uart0_int_status);								//Clear the asserted interrupts
		
		// Loop while there are characters in the receive FIFO.
		while (UARTCharsAvail(UART0_BASE))	{
			char temp = UARTCharGetNonBlocking(UART0_BASE);
			if(temp != '\r') GetChar[i++] = temp;	
			else break;
		}
		GetChar[i++] = '\0';
		
		
		if (UARTCharGet(UART0_BASE) == '\n'){
			
			// Get Date
			DatePutChar = Int2String(10000*_year + 100*_month + _day);
			strcat(DatePutChar, "\n\n");
			if (isStringEqual(GetChar, "GET+DATE")) {UARTStringPut(DatePutChar); inputflag = true;}
			
			//Get Time
			TimePutChar = Int2String(100*_hour + _minute);
			strcat(TimePutChar, "\n\n");
			if (isStringEqual(GetChar, "GET+TIME")) {UARTStringPut(TimePutChar); inputflag = true;}
			
			// Set Date
			if (GetChar[0] == 'D'){
				DateRx = String2Int_ad(GetChar, 4);
				_month = DateRx / 100; _day = DateRx % 100;
				inputflag = true;
			}
			
			// Set Time
			if (GetChar[0] == 'T'){
				TimeRx = String2Int_ad(GetChar, 4);
				_hour = TimeRx / 100; _minute = TimeRx % 100;
				inputflag = true;
			}
			
			// Set ARM1
			if (GetChar[3] == 'P'){		// ARMPxxx
				ArmRx_1 = String2Int_ad(GetChar, 4);
				_hour1 = ArmRx_1 / 100; _minute1 = ArmRx_1 % 100;
				arm1flag = true;
				inputflag = true;
			}	
			
			// Set ARM2
			if (GetChar[3] == 'Q'){		// ARMPxxx
				ArmRx_2 = String2Int_ad(GetChar, 4);
				_hour2 = ArmRx_2 / 100; _minute2 = ArmRx_2 % 100;
				arm2flag = true;
				inputflag = true;
			}	
			
			// Reset
			if (GetChar[0] == 'R'){
				Display_StudentNum_ad();
				Button_Status = 0;
				inputflag = true;
			}
			
			if(inputflag == false) UARTStringPutNonBlocking("InputError\n\n");
			
			i = 0;		// A must
			Flash("D1"); Delay(20);
		}
		
		inputflag = false;
		UnFlash("D1");
}



void TIMER0A_Handler(void) { 
	uint32_t  intStatus; 
	intStatus = TimerIntStatus(TIMER0_BASE, true); 
	TimerIntClear(TIMER0_BASE, intStatus ); 

	if(motor_flag1 && !nightflag){
		GPIO_PORTF_AHB_DATA_R = AHB_DATA_array[7 - step];
		step = (step + 1) % 8;
	}
	
	if(motor_flag2 && !nightflag){
		GPIO_PORTF_AHB_DATA_R = AHB_DATA_array[step];
		step = (step + 1) % 8;
	}
		
}

