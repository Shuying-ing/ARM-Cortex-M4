# include "qsy.c"

int main(void){
	Initialize_All();
	
	while(1){
		if(isPressed("USR_SW1"))Twinkle("LED_M0",FASTFLASHTIME);
		else Twinkle("LED_M0",SLOWFLASHTIME);
	}
}

