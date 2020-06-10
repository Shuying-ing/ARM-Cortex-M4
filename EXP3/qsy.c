#include "qsy.h"

#define   FASTFLASHTIME			(uint32_t) 300000
#define   SLOWFLASHTIME			(uint32_t) FASTFLASHTIME*20
#define   MIDFLASHTIME			(uint32_t) FASTFLASHTIME*1.5


uint8_t const Display_LED_Position[] = { 0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f };
uint8_t const Display_7segment_Position[] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80 };			//increse bit by bit
uint8_t const Display_7segment_Content[] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x79,0x71,0x5c };
char* Month_ch[12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
	


int Button_Status = 0;
bool PJ0ReadFlag = 0, PJ1ReadFlag = 0;
bool key1_pressed = 0, key1_released = 0;
bool key2_pressed = 0, key2_released = 0;

volatile uint8_t result;
uint16_t Pulse_Frequency;
volatile uint16_t systick_1ms_counter,systick_500ms_counter, systick_200ms_counter, systick_100ms_counter, systick_1000ms_counter, systick_ms_counter;
volatile uint8_t systick_1ms_status,systick_500ms_status, systick_200ms_status, systick_100ms_status, systick_1000ms_status, systick_ms_status;

int _hour = 0, _minute = 0, _second = 0;
int _time;

char GetChar[100];
char *PutChar;
int inc = 0;

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
	S800_UART_Init();
	
	IntMasterEnable();
	SystickInitialize(1000);		//Enable systick
	IntEnable(INT_UART0);			//Enable UART0_int
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);	//Enable UART0 RX,TX interrupt
	
	//Priority Set
	ui32IntPriorityMask = IntPriorityMaskGet();
	IntPriorityGroupingSet(3);														//Set all priority to pre-emtption priority

	IntPrioritySet(INT_UART0, 0);									//Set INT_UART0 to lowest priority
	IntPrioritySet(FAULT_SYSTICK,0xe0);									//Set INT_SYSTICK to highest priority

	ui32IntPriorityGroup = IntPriorityGroupingGet();

	ui32IntPriorityUart0 = IntPriorityGet(INT_UART0);
	ui32IntPrioritySystick = IntPriorityGet(FAULT_SYSTICK);	
}



// ********************************************************************************
//
//										GPIO
//
// 		"USR_SW1"--PJ0	"USR_SW2"--PJ1
//		"LED_M0"--PF0	"LED_M1"--PF1	"D1"--PN1	"D2"--PN0
// ********************************************************************************
//
//		Button_Name = "USR_SW1"/"USR_SW2"
//		LED_Name ="LED_M0"/"LED_M1"/"D2"/"D1"
// -------------------------------------------------

/* Initiate GPIO = Enable ports + Inport/Output + ConfigSet */
void S800_GPIO_Init(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);							//Enable PortF
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);							//Enable PortJ
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);							//Enable PortN
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));			//Wait for the GPIO moduleN ready
	
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);       //Set PJ0,PJ1 as input pin
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1);			//Set PF0,PF1 as Output pin
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);			//Set PN0,PN1 as Output pin
	
	GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

/* Put on the LED */
void Flash(char* LED_Name) {
	if (isStringEqual(LED_Name, "LED_M0"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
	if (isStringEqual(LED_Name, "LED_M1"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
	if (isStringEqual(LED_Name, "D2"))GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
	if (isStringEqual(LED_Name, "D1"))GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
}

/* Put off the LED */
void UnFlash(char* LED_Name) {
	if (isStringEqual(LED_Name, "LED_M0"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
	if (isStringEqual(LED_Name, "LED_M1"))GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
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
	I2CMasterInitExpClk(I2C0_BASE, ui32SysClock, true);										//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);

	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_CONFIG_PORT0, 0x0ff);			//config port 0 as input
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_CONFIG_PORT1, 0x0);			//config port 1 as output
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_CONFIG_PORT2, 0x0);			//config port 2 as output 

	result = I2C0_WriteByte(PCA9557_I2CADDR, PCA9557_CONFIG, 0x00);					//config port as output
	result = I2C0_WriteByte(PCA9557_I2CADDR, PCA9557_OUTPUT, 0x0ff);				//turn off the LED1-8
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);	//clear content	
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
	//Delay(1);
	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true);//Set the address of slave
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);//Read once;
	while (I2CMasterBusBusy(I2C0_BASE));
	value = I2CMasterDataGet(I2C0_BASE);//Get data;
	//Delay(1);
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


// ------------------------------------
// 	Digit_7segment
// 
// 	position = 0 - 7
// ------------------------------------

void Display_1digit_7segment_char(char content, int position) {
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);	//clear content	
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT2, Display_7segment_Position[position % 8]);	//write position
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, Display_7segment_Content[(content - '0') % 16]);	//write port 1 
}

void Display_1digit_7segment_int(int content, int position) {
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);	//clear content	
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT2, Display_7segment_Position[position % 8]);	//write position
	result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, Display_7segment_Content[content]);	//write port 1 
	//result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, Display_7segment_Content[content % 15]);	//write port 1 
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

void Display_AnyDigit_7segment_char(char* content, int position) {
	int Digit = strlen(content);
	int i = 0;
	for (i = 0; i < Digit; i++) {
		Display_1digit_7segment_char(content[i], position + i);
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

void Set_Frequency(uint32_t Frequency) { SysTickPeriodSet(ui32SysClock / Frequency); }


void SysTick_Handler(void) {
	//if(isPressed("pj0"))Flash("pn1");else UnFlash("pn1");
	//while(isPressed("sw1")){Flash("PN0");I2C_UnFlash_All_LED();result = I2C0_WriteByte(TCA6424_I2CADDR, TCA6424_OUTPUT_PORT1, 0x00);		}
	//if(!isPressed("sw1")) UnFlash("PN0");
	
	if (systick_100ms_counter != 0)
		systick_100ms_counter--;
	else
	{
		systick_100ms_counter = 100;
		systick_100ms_status = 1;
	}
	
	if (systick_200ms_counter != 0)
		systick_200ms_counter--;
	else
	{
		systick_200ms_counter = 100;
		systick_200ms_status = 1;
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

	if (systick_1ms_counter != 0)
		systick_1ms_counter--;
	else
	{
		systick_1ms_counter = 1;
		systick_1ms_status = 1;
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
void Display_Time() {
	uint16_t hour_1 = _hour % 10;
	uint16_t hour_10 = _hour / 10;
	uint16_t minute_1 = _minute % 10;
	uint16_t minute_10 = _minute / 10;
	uint16_t second_1 = _second % 10;
	uint16_t second_10 = _second / 10;

	Display_1digit_7segment_int(hour_10, 0);
	Display_1digit_7segment_int(hour_1, 1);
	Display_1digit_7segment_int(minute_10, 2);
	Display_1digit_7segment_int(minute_1, 3);
	Display_1digit_7segment_int(second_10, 4);
	Display_1digit_7segment_int(second_1, 5);
}
void Set_Time(int hour, int min, int sec) {
	_hour = hour % 24;

	_minute = min % 60;
	Inc_hour(min / 60);

	_second = sec % 60;
	Inc_minute(sec / 60);
}
void Inc_hour(int Increasement) {
	_hour = (_hour + Increasement) % 24;
}
void Inc_minute(int Increasement) {
	_minute = _minute + Increasement;
	while (_minute > 59) { _minute -= 60; Inc_hour(1); }
}
void Inc_second(int Increasement) {
	_second = _second + Increasement;
	while (_second > 59) { _second -= 60; Inc_minute(1); }
}
void Dec_hour(int Decreasement) {
	_hour = (_hour - Decreasement);
	while (_hour < 0)_hour += 24;
}
void Dec_minute(int Decreasement) {
	_minute = (_minute - Decreasement);
	while (_minute < 0) { _minute += 60; Dec_hour(1); }
}
void Dec_second(int Decreasement) {
	_second = (_second - Decreasement);
	while (_second < 0) { _second += 60; Dec_minute(1); }
}
void Inc_Time(int IncHour,int IncMin,int IncSec) {
	Inc_hour(IncHour);
	Inc_minute(IncMin);
	Inc_second(IncSec);
}
void Dec_Time(int DecHour, int DecMin, int DecSec) {
	Dec_hour(DecHour);
	Dec_minute(DecMin);
	Dec_second(DecSec);
}





// ********************************************************************************
//
//										UART
//
// ********************************************************************************

/* Initiate UART */
void S800_UART_Init(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);						//Enable PortA
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));			//Wait for the GPIO moduleA ready

	GPIOPinConfigure(GPIO_PA0_U0RX);												// Set GPIO A0 and A1 as UART pins.
	GPIOPinConfigure(GPIO_PA1_U0TX);

	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Configure the UART for 115,200, 8-N-1 operation.
	UARTConfigSetExpClk(UART0_BASE, ui32SysClock, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	//UARTStringPut((uint8_t *)"\r\nHellooooo, world!\r\n");
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


/* Get part of the string */
char* getCertainString(char *origin, int Length, int StartPosition){
	char final[100];
	for(int i = 0; i < Length; i++){
		final[i] = origin[StartPosition + i];
	}
	return final;
}

/* Exchange String to Int */
int String2Int(char *ch, int start, int length){
	int num = 0;
	for (int i=start; i < start + length; i++){
		num = 10 * num + ch[i] - '0'; 
	}
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
	for(int j = 0; j<(i+1)/2; j++){
		temp = s[j];
		s[j] = s[i-1-j];
		s[i-1-j] = temp;
	}
		
	return s;
}

int JudgeMonth(char *ch){
	if(ch[0]=='J' && ch[1]=='A' && ch[2]=='N') return 1;
	if(ch[0]=='F') return 2;
	if(ch[0]=='M' && ch[1]=='A' && ch[2]=='R') return 3;
	if(ch[0]=='A' && ch[1]=='P') return 4;
	if(ch[0]=='M' && ch[1]=='A' && ch[2]=='Y') return 5;
	if(ch[0]=='J' && ch[1]=='U' && ch[2]=='N') return 6;
	if(ch[0]=='J' && ch[1]=='U' && ch[2]=='L') return 7;
	if(ch[0]=='A' && ch[1]=='U') return 8;
	if(ch[0]=='S') return 9;
	if(ch[0]=='O') return 10;
	if(ch[0]=='N') return 11;
	if(ch[0]=='D') return 12;
}

