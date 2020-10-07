#include "qsy.h"

#define   FASTFLASHTIME			(uint32_t) 300000
#define   SLOWFLASHTIME			(uint32_t) FASTFLASHTIME*20
#define   MIDFLASHTIME			(uint32_t) FASTFLASHTIME*1.5


uint8_t const Display_LED_Position[] = { 0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f };
uint8_t const Display_7segment_Position[] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80 };			//increse bit by bit
uint8_t const Display_7segment_Content[] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x79,0x71,0x5c };
uint8_t const SW_Position[] = { 0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f };
uint8_t const AHB_DATA_array[8] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x09};
int date_array[8] = { 2, 0, 2, 0, 0, 6, 2, 1 };
int time_array[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };


int Button_Status = 0;
bool PJ0ReadFlag = 0, PJ1ReadFlag = 0;
bool key1_pressed = 0, key1_released = 0;				// USR_SW1
bool key2_pressed = 0, key2_released = 0;				// USR_SW2
bool keySW_pressed[9] = { 1, 0, 0, 0, 0, 0, 0, 0, 0 };		// 第一项不用
int cnt[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };		// keySW_pressed


volatile uint8_t result;
uint16_t Pulse_Frequency;
volatile uint16_t systick_1ms_counter, systick_1ms01_counter, systick_10ms_counter, systick_500ms_counter, systick_200ms_counter, systick_250ms_counter, systick_260ms_counter, 
	systick_1000ms_counter, systick_1000ms01_counter, systick_1000ms02_counter, systick_ms_counter;
volatile uint8_t systick_1ms_status, systick_1ms01_status, systick_10ms_status, systick_500ms_status, systick_200ms_status, systick_250ms_status, systick_260ms_status, 
	systick_1000ms_status, systick_1000ms01_status, systick_1000ms02_status, systick_ms_status;

int _year = 0, _month = 0, _day = 0;
int _hour = 0, _minute = 0, _second = 0, _msec = 0;
int _hour1 = 0, _minute1 = 0;
int _hour2 = 0, _minute2 = 0;
bool apm = true, pm = false;		//true = 24h
bool nightflag = false;					// true = night
bool monthflag = false, dayflag = false, hourflag = false, minuteflag = false;
bool arm1flag = false, arm2flag = false, arm1_over_flag = false, arm2_over_flag = false;
bool SutdentNum_flag = true;

uint32_t current_step = 0;
uint8_t step = 0;
bool motor_flag1 = false, motor_flag2 = false, motor_clockwise = true;
int BeepPeriod = 1600;

char GetChar[100], a[100];
int i = 0;		// UART
char *PutChar, *DatePutChar, *TimePutChar;
int DateRx, TimeRx, ArmRx_1, ArmRx_2;
bool inputflag = false;


uint32_t ui32SysClock, ui32IntPriorityGroup, ui32IntPriorityMask;
uint32_t ui32IntPrioritySystick, ui32IntPriorityUart0;



// ********************************************************************************
//
//									Initialize
//
// ********************************************************************************

void Initialize_All() {
	/* Set SystemClock Frequency */
	
	//use internal 16M oscillator, HSI
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ | SYSCTL_OSC_INT | SYSCTL_USE_OSC), 16000000);

	//use internal 16M oscillator, PIOSC
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 16000000);		
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 8000000);		

	//use external 25M oscillator, MOSC
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 25000000);		

	//use external 25M oscillator and PLL to 120M
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 120000000);		
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ | SYSCTL_OSC_INT | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 20000000);
	
	
	S800_GPIO_Init();
	S800_I2C0_Init();
	S800_Timer0_Init();
	S800_PWM_Init();
	S800_UART_Init();
	
	IntMasterEnable();
	SystickInitialize(1000);		// Enable systick
	IntEnable(INT_TIMER0A); 		// Enable Timer0
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT); 
	IntEnable(INT_UART0);				// Enable UART0_int
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);	//Enable UART0 RX,TX interrupt
	
	UnDisplay_AnyDigit();
}


void Update_LED(){
	for(int i = 0; i < 6; ++i){
		if (Button_Status == i) I2C_Display_LED(i);
	}
	
	if(!apm) {
		if(pm) {Flash("D1"); UnFlash("D3");}
		else {Flash("D3"); UnFlash("D1");}
	}
	else{UnFlash("D1"); UnFlash("D3");}
		
	
	if(arm1flag)	Flash("LED_M2");
	else UnFlash("LED_M2");
	if(arm1_over_flag) {ARM1_over(); nightflag = false;}
	
	if(arm2flag)	Flash("LED_M3");
	else UnFlash("LED_M3");
	if(arm2_over_flag) {ARM2_over(); nightflag = false;}
	
	if(nightflag){
		GPIOPinWrite(GPIO_PORTF_BASE,  GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, 0);
		GPIOPinWrite(GPIO_PORTN_BASE,  GPIO_PIN_0 | GPIO_PIN_1, 0);
		I2C_UnFlash_All_LED();
		result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);	
	}
}


void Update_APM(){
	if(isPressed_ad("USR_SW2")){
		if(apm){		// 24小时制
			apm = false;
			if(_hour > 11) pm = true;
			else pm = false;
		}
		else{			// 12小时制
			apm = true;
			if(pm) _hour += 12;
		}
	}
	key2_released = false;
}


void Update_DayNight(){
	if(keySW_pressed[7]) {
		if(!nightflag) nightflag = true;
		else nightflag = false;
		keySW_pressed[7] = false;
	}
}





// ********************************************************************************
//
//										GPIO
//
// 		"USR_SW1"--PJ0	"USR_SW2"--PJ1
//		"LED_M0/1/2/3"--PF0/1/2/3	"D1"--PN1	"D2"--PN0 "D3"--PF4
// ********************************************************************************
//
//		Button_Name = "USR_SW1"/"USR_SW2"
//		LED_Name ="LED_M0"/"LED_M1"/"LED_M2"/"LED_M3"/"D2"/"D1"/"D3"
// ---------------------------------------------------------------------

/* Initiate GPIO = Enable ports + Inport/Output + ConfigSet */
void S800_GPIO_Init(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);							//Enable PortF
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);							//Enable PortJ
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);							//Enable PortN
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));			//Wait for the GPIO moduleN ready
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK); 
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){}
	
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);       //Set PJ0,PJ1 as input pin
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4);			//Set PF0,PF1 as Output pin
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);			//Set PN0,PN1 as Output pin
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_5);
		
	GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_8MA,GPIO_PIN_TYPE_STD_WPU);
	GPIOPinConfigure(GPIO_PK5_M0PWM7);
	GPIOPinTypePWM(GPIO_PORTK_BASE, GPIO_PIN_5);

}

/* Put on the LED */
void Flash(char* LED_Name) {
	if (isStringEqual(LED_Name, "LED_M0"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
	if (isStringEqual(LED_Name, "LED_M1"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
	if (isStringEqual(LED_Name, "LED_M2"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
	if (isStringEqual(LED_Name, "LED_M3"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
	if (isStringEqual(LED_Name, "D2"))GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
	if (isStringEqual(LED_Name, "D1"))GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
	if (isStringEqual(LED_Name, "D3"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
	if (isStringEqual(LED_Name, "D4"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
}

/* Put off the LED */
void UnFlash(char* LED_Name) {
	if (isStringEqual(LED_Name, "LED_M0"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
	if (isStringEqual(LED_Name, "LED_M1"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
	if (isStringEqual(LED_Name, "LED_M2"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
	if (isStringEqual(LED_Name, "LED_M3"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
	if (isStringEqual(LED_Name, "D2"))GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
	if (isStringEqual(LED_Name, "D1"))GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
}

/* Twinkle Twinkle little star */
void Twinkle(char* LED_Name,uint32_t FlashTime) {		// FlashTime = xxxFLASHTIME
	Flash(LED_Name); 
	Delay(FlashTime); 
	UnFlash(LED_Name);
	Delay(FlashTime);
}


/* Judge if the button is pressed */
bool isPressed(char* Button_Name) {
	if (isStringEqual(Button_Name, "USR_SW1")) return !GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0);
		
	if (isStringEqual(Button_Name, "USR_SW2")) return !GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1);
}


/* Judge if the button is pressed, more precise */
bool isPressed_ad(char* Button_Name) {
	if (isStringEqual(Button_Name, "USR_SW1")) 
	{
		if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == 0)
		{
			if (key1_released==false) key1_pressed = true;
		}
		else
		{
			if(key1_pressed==true)
			{
				key1_released = true;
				key1_pressed = false;
			}
		}
		return key1_released;
	}
		
	if (isStringEqual(Button_Name, "USR_SW2"));
	{
		if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1) == 0)
		{
			if (key2_released==false) key2_pressed = true;
		}
		else
		{
			if(key2_pressed==true)
			{
				key2_released = true;
				key2_pressed = false;
			}
		}
		return key2_released;
	}
	
}




// ********************************************************************************
//
//										I2C
//
// ********************************************************************************


/* Initiate I2C0 */
void S800_I2C0_Init(void)
{
	uint8_t result;
	
	//Enable I2C0 and GPIO portB containing the I2C pins(PB2&PB3)
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);//Initiallize i2c module
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);//Use I2C module 0,pin set as I2C0SCL--PB2,I2C0SDA--PB3
	
	//Configure the PB2 and PB3 pins for I2C operation
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);//Set PB2 as I2C0SCL
	GPIOPinConfigure(GPIO_PB3_I2C0SDA);//Set PB3 as I2C0SDA
	GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);//I2C uses GPIO_PIN_2 as SCL
	GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);//I2C uses GPIO_PIN_3 as SDA

	//Configure I2C0 Master mode, 400kbps
	I2CMasterInitExpClk(I2C0_BASE, ui32SysClock, true);								//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);

	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_CONFIG_PORT0, 0x0ff);			//config port 0 as input
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_CONFIG_PORT1, 0x0);			//config port 1 as output
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_CONFIG_PORT2, 0x0);			//config port 2 as output 

	result = I2C0_WriteByte(PCA9557_I2CADDR, PCA9557_CONFIG, 0x00);					//config port as output
	result = I2C0_WriteByte(PCA9557_I2CADDR, PCA9557_OUTPUT, 0x0ff);				//turn off the LED1-8
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);			//clear content	
}


// ------------------------------------
// 	Write && Read
// ------------------------------------
uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData)
{
	uint8_t rop;
	while (I2CMasterBusy(I2C0_BASE)) {};//Wait if I2C is busy;
	
	//Set Devaddr &&  RegAddr
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);//false means master writes slave, true means slave reads master
	I2CMasterDataPut(I2C0_BASE, RegAddr);//Master writes the address of device;

	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);//Repeat writing;
	while (I2CMasterBusy(I2C0_BASE)) {};
	
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);//For test
	
	//Set data
	I2CMasterDataPut(I2C0_BASE, WriteData);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);//Repeat writing and ends.
	while (I2CMasterBusy(I2C0_BASE)) {};

	rop = (uint8_t)I2CMasterErr(I2C0_BASE);//For  test

	return rop;//return the type of error ,return 0 if no error.
}

uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr)
{
	uint8_t value, rop;
	while (I2CMasterBusy(I2C0_BASE)) {};
	
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
	
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);//Read once;
	while (I2CMasterBusBusy(I2C0_BASE));
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	Delay(100);
	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true);//Set the address of slave
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);//Read once;
	while (I2CMasterBusBusy(I2C0_BASE));
	value = I2CMasterDataGet(I2C0_BASE);//Get data;
	Delay(100);
	return value;
}


// ------------------------------------
// 	LED_1-8
// ------------------------------------
void I2C_Display_LED(int position){ result = I2C0_WriteByte(PCA9557_I2CADDR, PCA9557_OUTPUT, Display_LED_Position[position % 8]);}
void I2C_Flash_2_LED(int i) { result = I2C0_WriteByte(PCA9557_I2CADDR, PCA9557_OUTPUT, Display_LED_Position[i % 8] & Display_LED_Position[(i + 1)%8]); }
void I2C_UnFlash_2_LED(int i) { result = I2C0_WriteByte(PCA9557_I2CADDR, PCA9557_OUTPUT, ~Display_LED_Position[i % 8] | ~Display_LED_Position[(i + 1) % 8]); }
void I2C_Flash_All_LED() { result = I2C0_WriteByte(PCA9557_I2CADDR, PCA9557_OUTPUT, 0x00); }
void I2C_UnFlash_All_LED() { result = I2C0_WriteByte(PCA9557_I2CADDR, PCA9557_OUTPUT, 0xff); }
void I2C_Twinkle_LED(int position, uint32_t FlashTime){
	I2C_Display_LED(position);
	Delay(FlashTime);
	I2C_UnFlash_All_LED();
	Delay(FlashTime); 
}



// ------------------------------------
// 	Digit_7segment
// 
// 	position = 0 - 7
// ------------------------------------



void Display_1digit_7segment_int(int content, int position) {
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);	//clear content	
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT2, Display_7segment_Position[position % 8]);	//write position
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, Display_7segment_Content[content]);	//write port 1 
}

void Display_AnyDigit_7segment_int(int content, int position) {
	int Digit = digit(content);
	int i = 0;
	int number[10];
	for (i = 0; i < Digit; i++) {
		number[i] = content % 10;
		content = content / 10;
	}
	for (i = 0; i < Digit; i++) {
		Display_1digit_7segment_int(number[Digit - i - 1], position + i);
	}
}


void Twinkle_1digit_7segment_int(int content, int position, uint32_t FlashTime){
	Display_1digit_7segment_int(content, position);
	Delay(FlashTime);
	UnDisplay_AnyDigit();
	Delay(FlashTime);
}



// ------------------------------------
// 	Twinkle Time && Date
// ------------------------------------

void Twinkle_Minute(){
	do{
		Update_Button_Status("USR_SW1",6);
		Update_Time();
		Set_Time(_hour, _minute, _second, _msec, apm);
		Update_SW();
		Update_LED();
		Update_APM();
		Update_DayNight();
		Motor_Enable();
		Set_Arm1();
		Set_Arm2();
		
		if(keySW_pressed[3]) {_minute += 1; keySW_pressed[3] = false;}
		if(keySW_pressed[4]) {_minute -= 1; 
			if(_minute == -1) {_minute = 59; _hour -= 1;}
			keySW_pressed[4] = false;}
		
		if(systick_260ms_status)	{systick_260ms_status = 0;
  	  if(minuteflag) minuteflag = false; else{minuteflag = true;}
		}
		
		if(minuteflag) {
			for(int i = 0; i < 8; i++) Display_1digit_7segment_int(time_array[i], i);
		}
		else{
			for(int i = 0; i < 2; i++) Display_1digit_7segment_int(time_array[i], i);
			for(int i = 4; i < 8; i++) Display_1digit_7segment_int(time_array[i], i);
		}

	}while(Button_Status == 4 && keySW_pressed[5] == 0 && keySW_pressed[6] == 0);

}

void Twinkle_Hour(){
	do{
		Update_Button_Status("USR_SW1",6);
		Update_Time();
		Set_Time(_hour, _minute, _second, _msec, apm);
		Update_SW();
		Update_LED();
		Update_APM();
		Update_DayNight();
		Motor_Enable();
		Set_Arm1();
		Set_Arm2();
		
		if(keySW_pressed[6]) {_hour += 1; keySW_pressed[6] = false;}
		if(keySW_pressed[5]) {_hour -= 1; 
			if(_hour == 0) _hour = 23;
			keySW_pressed[5] = false;}
		
		if(systick_260ms_status)	{systick_260ms_status = 0;
  	  if(hourflag) hourflag = false; else{hourflag = true;}
		}
		
		if(hourflag) {
			for(int i = 0; i < 8; i++) Display_1digit_7segment_int(time_array[i], i);
		}
		else{
			for(int i = 2; i < 8; i++) Display_1digit_7segment_int(time_array[i], i);
		}
		
	}while(Button_Status == 4 && keySW_pressed[3] == 0 && keySW_pressed[4] == 0);

}


void Twinkle_Day(){
	do{
		Update_Button_Status("USR_SW1",6);
		Set_Date(_year, _month, _day);
		Update_Date();
		Update_SW();
		Update_LED();
		Update_APM();
		Update_DayNight();
		Motor_Enable();
		Set_Arm1();
		Set_Arm2();
		
		if(keySW_pressed[6]) {_day += 1; keySW_pressed[6] = false;}
		if(keySW_pressed[5]) {_day -= 1; 
			if(_day == -1) {_day = 30; _month -= 1;}
			keySW_pressed[5] = false;}
		
		if(systick_260ms_status)	{systick_260ms_status = 0;
  	  if(dayflag) dayflag = false; else{dayflag = true;}
		}
		
		if(dayflag) {
			for(int i = 0; i < 8; i++) Display_1digit_7segment_int(date_array[i], i);
		}
		else{
			for(int i = 0; i < 6; i++) Display_1digit_7segment_int(date_array[i], i);
			result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);	//clear content
		}
		
	}while(Button_Status == 5 && keySW_pressed[3] == 0 && keySW_pressed[4] == 0);

}


void Twinkle_Month(){
	do{
		Update_Button_Status("USR_SW1",6);
		Set_Date(_year, _month, _day);
		Update_Date();
		Update_SW();
		Update_LED();
		Update_APM();
		Update_DayNight();
		Motor_Enable();
		Set_Arm1();
		Set_Arm2();
		
		if(keySW_pressed[3]) {_month += 1; keySW_pressed[3] = false;}
		if(keySW_pressed[4]) {_month -= 1; keySW_pressed[4] = false;}
		
		if(systick_260ms_status)	{systick_260ms_status = 0;
  	  if(monthflag) monthflag = false; else{monthflag = true;}
		}

		if(monthflag) {
			for(int i = 0; i < 8; i++) Display_1digit_7segment_int(date_array[i], i);
		}
		else{
			for(int i = 0; i < 4; i++) Display_1digit_7segment_int(date_array[i], i);
			for(int i = 6; i < 8; i++) Display_1digit_7segment_int(date_array[i], i);
		}
		
	}while(Button_Status == 5  && keySW_pressed[5] == 0 && keySW_pressed[6] == 0);
	
}


void Display_StudentNum(){
	int x = 0;
	while(SutdentNum_flag){
		if(systick_1000ms_status)	{systick_1000ms_status = 0;
			++x;
			if(x == 2) SutdentNum_flag = false;
		}
		Display_AnyDigit_7segment_int(21910790, 0);
	}
}


void Display_StudentNum_ad(){
	int y = 0;
	while(y < 500){
		++y;
		Display_AnyDigit_7segment_int(21910790, 0);
	}
}


void UnDisplay_1digit_7segment_int(int position) {
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);	//clear content	
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT2, Display_7segment_Position[position % 8]);	//write position
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);	//write port 1 
}


void UnDisplay_AnyDigit(void){result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);	}





// ------------------------------------
// 	SW 1-8
// ------------------------------------

/* Judge if the button is pressed */
bool is_SW_Pressed(char* Button_Name) {
	result = I2C0_ReadByte(TCA6424_I2CADDR, TCA6424_INPUT_PORT0);
	if (isStringEqual(Button_Name, "SW1")) return result == SW_Position[0];
	if (isStringEqual(Button_Name, "SW2")) return result == SW_Position[1];
	if (isStringEqual(Button_Name, "SW3")) return result == SW_Position[2];
	if (isStringEqual(Button_Name, "SW4")) return result == SW_Position[3];
	if (isStringEqual(Button_Name, "SW5")) return result == SW_Position[4];
	if (isStringEqual(Button_Name, "SW6")) return result == SW_Position[5];
	if (isStringEqual(Button_Name, "SW7")) return result == SW_Position[6];
	if (isStringEqual(Button_Name, "SW8")) return result == SW_Position[7];
}

void Update_SW(){
		if(systick_10ms_status)	{systick_10ms_status = 0;
			if(is_SW_Pressed("SW1")) ++cnt[0];
			if(is_SW_Pressed("SW2")) ++cnt[1];
			if(is_SW_Pressed("SW3")) ++cnt[2];
			if(is_SW_Pressed("SW4")) ++cnt[3];
			if(is_SW_Pressed("SW5")) ++cnt[4];
			if(is_SW_Pressed("SW6")) ++cnt[5];
			if(is_SW_Pressed("SW7")) ++cnt[6];
			if(is_SW_Pressed("SW8")) ++cnt[7];
		}
		
		for(int i = 0; i < 7; i++){
			if(cnt[i] == 15) {keySW_pressed[i + 1] = true; cnt[i] = 0;}
		}
}




// ********************************************************************************
//
//									 Systick
//
// ********************************************************************************

/* Initiate Systick */
void SystickInitialize(uint32_t frequency) {
	SysTickPeriodSet(ui32SysClock / frequency);
	SysTickEnable();
	SysTickIntEnable();
}


void SysTick_Handler(void) {
	if (systick_200ms_counter != 0)
		systick_200ms_counter--;
	else
	{
		systick_200ms_counter = 200;
		systick_200ms_status = 1;
	}
	
	if (systick_250ms_counter != 0)
		systick_250ms_counter--;
	else
	{
		systick_250ms_counter = 250;
		systick_250ms_status = 1;
	}
	
	if (systick_260ms_counter != 0)
		systick_260ms_counter--;
	else
	{
		systick_260ms_counter = 260;
		systick_260ms_status = 1;
	}
	
	if (systick_500ms_counter != 0)
		systick_500ms_counter--;
	else
	{
		systick_500ms_counter = 500;
		systick_500ms_status = 1;
	}

	if (systick_1000ms_counter != 0)
		systick_1000ms_counter--;
	else
	{
		systick_1000ms_counter = 1000;
		systick_1000ms_status = 1;
	}

	if (systick_1000ms01_counter != 0)
		systick_1000ms01_counter--;
	else
	{
		systick_1000ms01_counter = 1000;
		systick_1000ms01_status = 1;
	}
	
	if (systick_1000ms02_counter != 0)
		systick_1000ms02_counter--;
	else
	{
		systick_1000ms02_counter = 1000;
		systick_1000ms02_status = 1;
	}
	
	if (systick_1ms_counter != 0)
		systick_1ms_counter--;
	else
	{
		systick_1ms_counter = 1;
		systick_1ms_status = 1;
	}
	
	if (systick_1ms01_counter != 0)
		systick_1ms01_counter--;
	else
	{
		systick_1ms01_counter = 1;
		systick_1ms01_status = 1;
	}
	
	if (systick_10ms_counter != 0)
		systick_10ms_counter--;
	else
	{
		systick_10ms_counter = 10;
		systick_10ms_status = 1;
	}

	if (systick_ms_counter != 0)
		systick_ms_counter--;
	else
	{
		systick_ms_counter = Pulse_Frequency;
		systick_ms_status = 1;
	}
}




// ********************************************************************************
//
//									 Timer
//
// ********************************************************************************

/* Initiate S800_Timer0 */
void S800_Timer0_Init(void) { 
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); 
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); 
	TimerLoadSet(TIMER0_BASE, TIMER_A, (ui32SysClock / 800));			// Set the count time 
	TimerEnable(TIMER0_BASE, TIMER_A); 
}




// ********************************************************************************
//
//									 PWM
//
// ********************************************************************************

/* Initiate S800_PWM */
void S800_PWM_Init(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) { } 
	PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_1);
	PWMGenConfigure(PWM0_BASE,PWM_GEN_3, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
	PWMGenPeriodSet(PWM0_BASE,PWM_GEN_3,BeepPeriod);		// 设置PWM周期
	PWMPulseWidthSet(PWM0_BASE,PWM_OUT_7,BeepPeriod / 4);
	PWMOutputState(PWM0_BASE,PWM_OUT_7_BIT,true); 
	// PWMGenEnable(PWM0_BASE,PWM_GEN_3);
}

void PWM_Enable(void) {PWMGenEnable(PWM0_BASE,PWM_GEN_3);}
void PWM_Disable(void) {PWMGenDisable(PWM0_BASE,PWM_GEN_3);}


void Motor_Enable(){
	if(Button_Status == 2 && arm1flag && arm1_over_flag == false) {motor_flag1 = true; motor_flag2 = false;}
	else if(Button_Status == 3 && arm2flag && arm2_over_flag == false) {motor_flag2 = true; motor_flag1 = false;}
	else motor_flag1 = motor_flag2 = false;
}




// ********************************************************************************
//
//									 Time
//
// ********************************************************************************

void Display_Date() {
	uint16_t month_1 = _month % 10;
	uint16_t month_10 = _month / 10;
	uint16_t day_1 = _day % 10;
	uint16_t day_10 = _day / 10;

	Display_AnyDigit_7segment_int(_year, 0);
	Display_1digit_7segment_int(month_10, 4);
	Display_1digit_7segment_int(month_1, 5);
	Display_1digit_7segment_int(day_10, 6);
	Display_1digit_7segment_int(day_1, 7);
}

void Set_Date(int year, int month, int day) {
	_year = year;
	
	if(month % 12 == 0) _month = 12;
	else _month = month % 12;
	Inc_year(month / 13);
	
	_day = day % 31;
	Inc_month(day / 31);
}

void Update_Date(){
	int y = _year;
	date_array[0] = y / 1000; y = y % 1000;
	date_array[1] = y / 100; y = y % 100;
	date_array[2] = y / 10; y = y % 10;
	date_array[3] = y;
	date_array[4] = _month / 10;
	date_array[5] = _month % 10;
	date_array[6] = _day / 10;
	date_array[7] = _day % 10;
}


void Inc_year(int Increasement) {
	_year = _year + Increasement;
}
void Inc_month(int Increasement) {
	_month = _month + Increasement;
	while (_month > 12) { _month -= 12; Inc_year(1); }
}


void Display_Time() {
	uint16_t hour_1 = _hour % 10;
	uint16_t hour_10 = _hour / 10;
	uint16_t minute_1 = _minute % 10;
	uint16_t minute_10 = _minute / 10;
	uint16_t second_1 = _second % 10;
	uint16_t second_10 = _second / 10;
	uint16_t msec_1 = _msec % 10;
	uint16_t msec_10 = _msec / 10;

	Display_1digit_7segment_int(hour_10, 0);
	Display_1digit_7segment_int(hour_1, 1);
	Display_1digit_7segment_int(minute_10, 2);
	Display_1digit_7segment_int(minute_1, 3);
	Display_1digit_7segment_int(second_10, 4);
	Display_1digit_7segment_int(second_1, 5);
	Display_1digit_7segment_int(msec_10, 6);
	Display_1digit_7segment_int(msec_1, 7);
}

void Set_Time(int hour, int min, int sec, int msec, bool apm) {
	if(apm) _hour = hour % 24;
	else _hour = hour % 12;

	_minute = min % 60;
	Inc_hour(min / 60, apm);

	_second = sec % 60;
	Inc_minute(sec / 60, apm);
	
	_msec = msec % 100;
	Inc_second(msec / 100, apm);
}


void Update_Time(){
	time_array[0] = _hour / 10;
	time_array[1] = _hour % 10;
	time_array[2] = _minute / 10;
	time_array[3] = _minute % 10;
}


void Inc_hour(int Increasement, bool apm) {
	if(apm) _hour = (_hour + Increasement) % 24;
	else _hour = (_hour + Increasement) % 12;
}
void Inc_minute(int Increasement, bool apm) {
	_minute = _minute + Increasement;
	while (_minute > 59) { _minute -= 60; Inc_hour(1, apm); }
}
void Inc_second(int Increasement, bool apm) {
	_second = _second + Increasement;
	while (_second > 59) { _second -= 60; Inc_minute(1, apm); }
}




void Dec_hour1(int Decreasement, bool apm) {
	_hour1 = (_hour1 - Decreasement);
	while (_hour1 < 0){
		if(apm) _hour1 += 24;
		else _hour1 += 12;
	}
}
void Dec_minute1(int Decreasement, bool apm) {
	_minute1 = (_minute1 - Decreasement);
	while (_minute1 < 0) { _minute1 += 60; Dec_hour1(1, apm); }
}
void Dec_second(int Decreasement) {
	_second = (_second - Decreasement);
	while (_second < 0) { _second += 60; Dec_minute1(1, apm); }
}


void Inc_Time(int IncHour,int IncMin,int IncSec, bool apm) {
	Inc_hour(IncHour, apm);
	Inc_minute(IncMin, apm);
	Inc_second(IncSec, apm);
}
void Dec_Time1(int DecHour, int DecMin, bool apm) {
	Dec_hour1(DecHour, apm);
	Dec_minute1(DecMin, apm);
}




// ********************************************************************************
//
//									 ARM
//
// ********************************************************************************
//
//					ARM1
// --------------------------------

void Set_Arm1(){
	if(apm) _hour1 = _hour1 % 24;
	else _hour1 = _hour1 % 12;
	
	if(arm1flag){
		if(systick_1000ms01_status)	{systick_1000ms01_status = 0;	--_minute1;}
		if(_minute1 == 0)
			if(_hour1 > 0) {--_hour1; _minute1 = 59;}
			else {arm1flag = false; arm1_over_flag = true;}
		}
	
	if(keySW_pressed[1]){		// 手动关闭闹钟1
		arm1flag = false;
		keySW_pressed[1] = false;
	}
}

void Display_Arm1(){
	if(arm1flag){
		uint16_t hour_1 = _hour1 % 10;
		uint16_t hour_10 = _hour1 / 10;
		uint16_t minute_1 = _minute1 % 10;
		uint16_t minute_10 = _minute1 / 10;

		Display_1digit_7segment_int(hour_10, 0);
		Display_1digit_7segment_int(hour_1, 1);
		Display_1digit_7segment_int(minute_10, 2);
		Display_1digit_7segment_int(minute_1, 3);
	}
	else
		result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);	//clear content
}

void ARM1_over(){
	int m = 0;
	bool flag1 = false;
	do{
		// 4Hz
		if(systick_250ms_status)	{systick_250ms_status = 0;
			++m;
			if(flag1) flag1 = false; else{flag1 = true;}
		}
		
		if(flag1) {
			Flash("LED_M2"); PWM_Enable();
		}
		else { 
			UnFlash("LED_M2"); PWM_Disable();
		}
		result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);
	
	}while(m < 15);
	
	arm1_over_flag = false;
	UnFlash("LED_M2"); 
	PWM_Disable();
	nightflag = false;
}


// --------------------------------
//					ARM2
// --------------------------------

void Set_Arm2(){
	if(apm) _hour2 = _hour2 % 24;
	else _hour2 = _hour2 % 12;
	
	if(arm2flag){
		if(systick_1000ms02_status)	{systick_1000ms02_status = 0;	--_minute2;}
		if(_minute2 == 0)
			if(_hour2 > 0) {--_hour2; _minute2 = 59;}
			else {arm2flag = false; arm2_over_flag = true;}
		}
	
	if(keySW_pressed[2]){		// 手动关闭闹钟2
		arm2flag = false;
		keySW_pressed[2] = false;
	}
}

void Display_Arm2(){
	if(arm2flag){
		uint16_t hour_1 = _hour2 % 10;
		uint16_t hour_10 = _hour2 / 10;
		uint16_t minute_1 = _minute2 % 10;
		uint16_t minute_10 = _minute2 / 10;

		Display_1digit_7segment_int(hour_10, 4);
		Display_1digit_7segment_int(hour_1, 5);
		Display_1digit_7segment_int(minute_10, 6);
		Display_1digit_7segment_int(minute_1, 7);
	}
	else
		result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);	//clear content
}

void ARM2_over(){
	int n = 0;
	bool flag2 = false;
	do{
		// 2Hz
		if(systick_500ms_status)	{systick_500ms_status = 0;
			++n;
			if(flag2) flag2 = false; else{flag2 = true;}
		}
		
		if(flag2) {
			Flash("LED_M3"); PWM_Enable();
		}
		else { 
			UnFlash("LED_M3"); PWM_Disable();
		}
		result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);
		
	}while(n < 10);
	
	arm2_over_flag = false;
	UnFlash("LED_M3");
	PWM_Disable();
	nightflag = false;
}




// ********************************************************************************
//
//									 UART
//
// ********************************************************************************

/* Initiate UART */
void S800_UART_Init(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);					//Enable PortA
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));			//Wait for the GPIO moduleA ready

	GPIOPinConfigure(GPIO_PA0_U0RX);								// Set GPIO A0 and A1 as UART pins.
	GPIOPinConfigure(GPIO_PA1_U0TX);

	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Configure the UART for 115,200, 8-N-1 operation.
	UARTConfigSetExpClk(UART0_BASE, ui32SysClock, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

/* Put Message */
void UARTStringPut(const char *cMessage) {
	while (*cMessage != '\0')
		UARTCharPut(UART0_BASE, *(cMessage++));
}

void UARTStringPutNonBlocking(const char *cMessage) {
	while (*cMessage != '\0')
		UARTCharPutNonBlocking(UART0_BASE, *(cMessage++));
}

/* Get Message */
void UARTStringGet(char *msg) {
	while (1) {
		*msg = UARTCharGet(UART0_BASE);
		if (*msg == '\r') {
			*msg = UARTCharGet(UART0_BASE);
			break;
		}
		msg++;
	}
	*msg = '\0';
}

void UARTStringGetNonBlocking(char *msg) {
	while (UARTCharsAvail(UART0_BASE)) {
		*msg++ = UARTCharGetNonBlocking(UART0_BASE); 
	} 
	*msg = '\0'; 
}





// ********************************************************************************
//
//									Other Functions
//
// ********************************************************************************

/* Delay for a certain time */
void Delay(uint32_t value) {
	uint32_t ui32Loop;
	for (ui32Loop = 0; ui32Loop < value; ui32Loop++) {};
}

/* Judge if chr1 == chr2 */
bool isStringEqual(char *chr1, char chr2[]) {
	int i = 0;
	while (chr1[i] != '\0' && chr2[i] != '\0') {
		if (chr1[i] != chr2[i] && chr1[i] != chr2[i] - ('A' - 'a')) 
			return false;
		i++;
	}
	return true;
}

/* Judge if chr1 == chr2 within JudgeLength */
bool is_N_StringEqual(char *chr1, char chr2[], int JudgeLength) {
	int i;
	for (i = 0; i < JudgeLength; i++) { 
		if (chr1[i] != chr2[i])	return false;
	}
	return true;
}

/* Update Button Status */
void Update_Button_Status(char* Button_Name,int Num_of_States) {		// Button_Name = "USR_SW1"/"USR_SW2"
	if (isStringEqual(Button_Name, "USR_SW1"))  {		//PJ0
		// Button On
		if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == GPIO_PIN_0)
			PJ0ReadFlag = 1;
		// Button Off
		if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == 0 && PJ0ReadFlag)
		{
			Button_Status = (Button_Status + 1) % Num_of_States;
			PJ0ReadFlag = 0;
		}
	}
	
	if (isStringEqual(Button_Name, "USR_SW2")) {		//PJ1
		if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1) == GPIO_PIN_1)
			PJ1ReadFlag = 1;
		if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1) == 0 && PJ1ReadFlag)
		{
			Button_Status = (Button_Status + 1) % Num_of_States;
			PJ1ReadFlag = 0;
		}
	}
}

/* Judge the digit of an int */
int digit(int n) {
	int digit = 0;
	
	if (n == 0)return 1;
	for (; n > 0; n /= 10){digit++;}
	return digit;
}

int String2Int(char *ch, int start, int length){
	int num = 0;
	for (int i=start; i < start + length; i++) {num = 10 * num + ch[i] - '0';}
	return num;
}

int String2Int_ad(char *ch, int length){
	int num = 0, i = 0;
	while(ch[i]<'0' || ch[i]>'9') i++;
	if(*ch=='\0') return 0;
	for(int j = i; j < i + length; ++j) {num = 10 * num + ch[j] - '0';}
	return num;
}

char* Int2String(int num){
	int i = 0;
	char s[100];
	do{
		s[i++] = num % 10 + '0';
	}while((num /= 10) > 0);
	s[i]='\0';
	
	char temp;
	for(int j = 0; j<(i+1)/2; j++) {temp = s[j]; s[j] = s[i-1-j];	s[i-1-j] = temp;}
		
	return s;
}

void Int2String_ad(int num){
	int i = 0;
	do{
		a[i++] = num % 10 + '0';
	}while((num /= 10) > 0);
	a[i]='\0';
	
	char temp;
	for(int j = 0; j<(i+1)/2; j++) {temp = a[j]; a[j] = a[i-1-j];	a[i-1-j] = temp;}
}


