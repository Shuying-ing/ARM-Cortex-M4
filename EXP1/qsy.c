#include "qsy.h"

#define   FASTFLASHTIME			(uint32_t) 300000
#define   SLOWFLASHTIME			(uint32_t) FASTFLASHTIME*20
#define   MIDFLASHTIME			(uint32_t) FASTFLASHTIME*1.5


int Button_Status = 0;
bool PJ0ReadFlag = 0, PJ1ReadFlag = 0;

uint32_t ui32SysClock, ui32IntPriorityGroup, ui32IntPriorityMask;
uint32_t ui32IntPrioritySystick, ui32IntPriorityUart0;


// ================================================================================
//
//									Initialize
//
// ================================================================================

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

}



// ================================================================================
//
//										GPIO
//
// 		"USR_SW1"--PJ0	"USR_SW2"--PJ1
//		"LED_M0"--PF0	"LED_M1"--PF1	"D1"--PN1	"D2"--PN0
// ================================================================================
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



// ================================================================================
//
//									Other Functions
//
// ================================================================================

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












