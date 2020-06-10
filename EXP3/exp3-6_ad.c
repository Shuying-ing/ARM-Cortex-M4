#include "qsy.c"

int main(void){
	Initialize_All();
	//int hour = 1;
	//int minite = 2;
	//int second = 3;
	
	while(1){
		if(systick_1000ms_status)
		{
		  systick_1000ms_status = 0;
			second = second + 1;
		}
		//minite = minite + inc;
			
		Set_Time(hour, minite, second);
		Display_Time();		
		
	}
}


void UART0_Handler(void)
{
	int32_t uart0_int_status;
	uart0_int_status = UARTIntStatus(UART0_BASE, true);		// Get the interrrupt status.

	UARTIntClear(UART0_BASE, uart0_int_status);								//Clear the asserted interrupts

	while (UARTCharsAvail(UART0_BASE))    											// Loop while there are characters in the receive FIFO.
	{
		UARTStringGetNonBlocking(GetChar);
		
		PutChar = getCertainString(GetChar, 2, 3);
		UARTStringPut(PutChar);
		UARTStringPut("\n");
		
		inc = StrToInt(PutChar);
		if (inc > 0) Flash("D0");
		
		Flash("D1");
		Delay(1000);
	}

	UnFlash("D1");
}
