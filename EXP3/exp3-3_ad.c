#include "qsy.c"

int main(void){
	Initialize_All();
	while(1){}
}

void UART0_Handler(void)
{
	int32_t uart0_int_status;
	uart0_int_status = UARTIntStatus(UART0_BASE, true);		// Get the interrrupt status.

	UARTIntClear(UART0_BASE, uart0_int_status);								//Clear the asserted interrupts

	while (UARTCharsAvail(UART0_BASE))    											// Loop while there are characters in the receive FIFO.
	{
		UARTStringGetNonBlocking(GetChar);

		UARTStringPut(GetChar);
		UARTStringPut("\n");
		
		Flash("D1");
		Delay(10000);
	}
	
	UnFlash("D1");

}


