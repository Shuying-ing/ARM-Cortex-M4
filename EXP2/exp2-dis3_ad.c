# include "qsy.c"


int main(void)
{
	Initialize_All();
	
	while (1)
	{			
		for(int i=0; i<8; i++){
			Display_1digit_7segment_int(i + 1, i);
			I2C_Flash_2_LED(i + 7);		//����i�Ϳ��ԣ�����һ������ָ���������+7
			Delay(SLOWFLASHTIME);
		}
		
	}
}

