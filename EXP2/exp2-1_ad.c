# include "qsy.c"


int main(void)
{
	Initialize_All();
	
	while (1)
	{		
		Display_1digit_7segment_int(0, 0);
		I2C_Flash_All_LED();
		
		Twinkle("LED_M0", 800000);
	}
}

