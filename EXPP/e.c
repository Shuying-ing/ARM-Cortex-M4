# include "qsy.c"

int main(void){
	Initialize_All();
	int year = 2020, month = 6, day = 16;
	Set_Date(year, month, day);
	int hour = 19, min = 15, sec = 2, msec = 2;
	Set_Time(hour, min, sec, msec, apm);
	
	while(1)
  {
		Pulse_Frequency = 10;
		if(systick_ms_status)	{systick_ms_status = 0; _msec = _msec + 1;}
		Set_Time(_hour, _minute, _second, _msec, apm);
		// Display_Time();
		
		if(isPressed_ad("USR_SW2") && motor_flag1 == false) {motor_flag1 = true; key2_released = 0; PWM_Disable();}
		if(isPressed_ad("USR_SW2") && motor_flag1 == true) {motor_flag1 = false; key2_released = 0; PWM_Disable();}
		
		if(isPressed_ad("USR_SW1") && motor_clockwise == false) {motor_clockwise = true; key1_released = 0; PWM_Enable();}
		if(isPressed_ad("USR_SW1") && motor_clockwise == true) {motor_clockwise = false; key1_released = 0; PWM_Enable();}
		
		t = 3;
		
  }
}


void UART0_Handler(void)
{
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
			// Set ARM1
			if (GetChar[3] == 'P'){		// ARMPxxx
				ArmRx_1 = String2Int_ad(GetChar, 4);
				_hour1 = ArmRx_1 / 100; _minute1 = ArmRx_1 % 100;
				arm1flag = true;
			}	
			
			i = 0;		// A must
		}
		
}




void TIMER0A_Handler(void) { 
	uint32_t  intStatus; 
	intStatus = TimerIntStatus(TIMER0_BASE, true); 
	TimerIntClear(TIMER0_BASE, intStatus ); 

	if(current_step < 450 * t){
		if(motor_flag1 || motor_flag2){
			if(motor_clockwise) GPIO_PORTF_AHB_DATA_R = AHB_DATA_array[step];
			else GPIO_PORTF_AHB_DATA_R = AHB_DATA_array[7 - step];
			step = (step + 1) % 8;
		}
		else GPIO_PORTF_AHB_DATA_R = 0;
		++current_step;
	}	
	
}
		

