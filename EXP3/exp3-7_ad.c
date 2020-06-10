#include "qsy.c"

int main(void){
	Initialize_All();
	int hour = 1;
	int minite = 2;
	int second = 3;
	
	while(1){
		if(systick_1000ms_status)
		{
		  systick_1000ms_status = 0;
			second = second + 1;
		}
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

		UARTStringPutNonBlocking(GetChar);
		UARTStringPut("\n");
		
		Flash("D1");
		Delay(10);
	}
	while(isPressed("USR_SW2")) Flash("D1");

	UnFlash("D1");
}
