# include "qsy.c"


int main(void)
{
	Initialize_All();
	int hour = 1;
	int minite = 59;
	int second = 50;
	
	while (1)
	{		
		
		if(systick_1000ms_status)
		{
		  systick_1000ms_status = 0;
			second = second + 1;
		}
			
		
		//if(isPressed_ad("USR_SW1")) {second = second + 1; key1_released = false;}
		//if(isPressed_ad("USR_SW2")) {minite = minite + 1; key2_released = false;}
		
		
		while(isPressed("USR_SW1")){
			result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);		//clear content
			if(systick_200ms_status){systick_200ms_status = 0; second = second + 1;}
			Flash("D1");
		}
		if(!isPressed("USR_SW1")) UnFlash("D1");
		
		
		while(isPressed("USR_SW2")){
			result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);		//clear content
			if(systick_200ms_status){systick_200ms_status = 0; minite = minite + 1;}
			Flash("D2");
		}
		if(!isPressed("USR_SW2")) UnFlash("D2");
		
		
		Set_Time(hour, minite, second);
		Display_Time();
		
	}
}

