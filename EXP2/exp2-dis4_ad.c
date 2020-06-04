# include "qsy.c"


int main(void)
{
	Initialize_All();
	int i, num = 0;
	
	while (1)
	{				
		if (i == 7) num = 81;
		else num = 10 * (i+1) + i + 2;
		
		Display_AnyDigit_7segment_int(num, i); 
		I2C_Flash_2_LED(i + 8);
		
		if(systick_1000ms_status){systick_1000ms_status = 0; i = (i + 1) % 8;;}
		
	}
		
}


