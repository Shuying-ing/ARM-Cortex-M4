# include "qsy.c"


int main(void)
{
	Initialize_All();
	int i = 0;
	
	while (1)
	{		
		if (!isPressed("USR_SW1")) i = (i + 1) % 8;
				
		Display_1digit_7segment_int(i + 1, i);
		I2C_Display_LED(i);
		Delay(SLOWFLASHTIME);
		
	}
}

