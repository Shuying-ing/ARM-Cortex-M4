#include "qsy.c"

int main(void){
	Initialize_All();
	while(1){
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
		
		int currentmonth = JudgeMonth(GetChar);
		int val = String2Int(GetChar, 4, 2);
		
		int month;
		if(GetChar[3]==43) month = currentmonth + val;
		else month = currentmonth - val;
		
		if(month <= 0) month += 12;
		if(month%12 == 0) month = 12;
		else month = month % 12;
		
		PutChar = Month_ch[month - 1];
		UARTStringPutNonBlocking(PutChar);
		UARTStringPut("\n\n");
		
		Flash("D1");
		Delay(10);
	}

	UnFlash("D1");
}
