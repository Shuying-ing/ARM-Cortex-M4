#include "qsy.c"

int main(void){
	Initialize_All();
	
	UARTStringPut("\r\nHelloooo, world!\r\n");
	
	while(1){
		while (UARTCharsAvail(UART0_BASE)) {
			UARTStringPut("\r\nHello, world!\r\n");
			
			UARTStringGet(GetChar);
			if (GetChar == "1") UARTStringPut("aaa");
			if (isStringEqual(GetChar, "AT+CLASS")) UARTStringPut("CLASSF1703202");
			
		}
	}
	
}

