# include "qsy.c"

int main(void){
	Initialize_All();
		
	while(1){
		if(isPressed("USR_SW1"))Flash("LED_M0");
		else UnFlash("LED_M0");
		
		if(isPressed("USR_SW2"))Flash("LED_M1");
		else UnFlash("LED_M1");
	}
}

