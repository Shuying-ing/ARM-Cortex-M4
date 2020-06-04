# include "qsy.c"


int main(void){
	int cnt = 0;
	//Pulse_Frequency = 300;
	Initialize_All();
	
	while(1){
		
		while(isPressed("USR_SW1"))
		{
			Flash("D2");I2C_UnFlash_All_LED();result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);
		}
		if(!isPressed("USR_SW1")) UnFlash("D2");
		
		Twinkle("LED_M0", MIDFLASHTIME);
		Display_1digit_7segment_int(cnt,cnt);
		I2C_Display_LED(cnt);
				
		if(systick_ms_status && !isPressed("USR_SW1"))
			{
			systick_ms_status = 0;
			cnt = (cnt + 1) % 8;
		}
			
	}
}

