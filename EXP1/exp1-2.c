#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_types.h"
#include "pin_map.h"
#include "sysctl.h"


#define   FASTFLASHTIME			(uint32_t) 300000
#define   SLOWFLASHTIME			(uint32_t) FASTFLASHTIME*20


void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);
void		PF0_Flash(uint32_t key_value);

int main(void)
{
	S800_GPIO_Init();
	while(1)
  {
		if(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)	//USR_SW1 - PJ0
			GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_PIN_0);		//Turn on the LED_M0 - PF0
		else
			GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,0x0);
		
		if(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1) == 0)	//USR_SW2 - PJ1
			GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,GPIO_PIN_1);		//Turn on the LED_M1 - PF1
		else
			GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,0x0);

	}
}


void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	//����ԭ�ͣ�void SysCtlPeripheralEnable(uint32_t ui32Peripheral)
	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	//����ԭ�ͣ�bool SysCtlPeripheralReady(uint32_t ui32Peripheral)
	//���ָ�������豻ʹ�ܳɹ�������true�����򷵻�false
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1);	//Set the PF0,PF1 as Output pin
	//LED_M0 && D4 - PF0
	//LED_M1 - PF1
	//����ԭ�ͣ�void GPIOPinTypeGPIOOutput(uint32_t ui32Port, uint8_t ui8Pins)
	//����GPIO�˿�����Ϊ������ţ�����ַ��ͣ�uint8_t������ui8PinsĳλΪ1����GPIO�˿ڶ�Ӧλ����Ϊ�������

	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);	//Set the PJ0,PJ1 as input pin
	//����GPIO�˿�����Ϊ�������ţ���GPIOPinTypeGPIOOutput()���ơ�GPIO_PIN_0 | GPIO_PIN_1 = 00000011b

	
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
	//����ԭ�ͣ�void GPIOPadConfigSet(uint32_t ui32Port, uint8_t ui8Pins, uint32_t ui32Strength, uint32_t ui32PinType)
	//GPIO�˿����á�uint32_t ui32Port��GPIO�˿ڻ���ַ
	//ui8Pins���˿�����λ��ϱ�ʾ����10000001b��ʾ���ö˿ڵ�D7��D0λ
	//ui32Strength���˿ڵ��������������������������Ч����ѡ�����GPIO_STRENGTH_2MA/4MA/8MA/8MA_SC/6MA/10MA/12MA
	//ui32PinType���������ͣ���ѡ�����GPIO_PIN_TYPE_STD�����죩��GPIO_PIN_TYPE_STD_WPU��������������GPIO_PIN_TYPE_STD_WPD��������������
	//GPIO_PIN_TYPE_OD����©����GPIO_PIN_TYPE_ANALOG��ģ�⣩��GPIO_PIN_TYPE_WAKE_HIGH���ߵ�ƽ�Ӷ��߻��ѣ���GPIO_PIN_TYPE_WAKE_LOW���ͣ�
}

