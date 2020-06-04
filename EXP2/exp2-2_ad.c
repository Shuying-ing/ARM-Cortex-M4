# include "qsy.c"


int main(void)
{
	Initialize_All();
	
	while (1)
	{			
		for(int i=0; i<8; i++){
			Display_1digit_7segment_int(i + 1, i);
			I2C_Display_LED(i);
			Delay(SLOWFLASHTIME);
		}
		
		// Twinkle("LED_M0", 800000);
	}
}

