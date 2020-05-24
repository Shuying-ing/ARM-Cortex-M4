# include "qsy.c"

int main(void){
	Initialize_All();
	
	while(1){
		Update_Button_Status("USR_SW1",4);
		switch(Button_Status){
			case 1:Twinkle("LED_M0",MIDFLASHTIME);break;
			case 2:UnFlash("LED_M0");break;
			case 3:Twinkle("LED_M1",MIDFLASHTIME);break;
			case 0:UnFlash("LED_M1");break;
		}
				
	}
}