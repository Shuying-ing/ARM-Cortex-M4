# include "qsy.c"


int main(void){
	int cnt = 0;
	Initialize_All();
	
	while(1){

		Display_1digit_7segment_int(cnt+1,cnt);
		I2C_Display_LED(cnt);
		
		Update_Button_Status("USR_SW1",4);
		switch(Button_Status)
		{
			case 1:Pulse_Frequency = 1000; break;
			case 2:Pulse_Frequency = 2000; break;
			case 3:Pulse_Frequency = 200; break;
			case 0:Pulse_Frequency = 500; break;
		}
							
		if(systick_ms_status && !isPressed("USR_SW1"))
		{
			systick_ms_status = 0;
			cnt = (cnt + 1) % 8;
		}
			
	}
}

