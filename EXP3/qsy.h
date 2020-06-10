#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_i2c.h"
#include "hw_types.h"
#include "i2c.h"
#include "pin_map.h"
#include "sysctl.h"
#include "systick.h"
#include "interrupt.h"
#include "uart.h"
#include "hw_ints.h"

#define TCA6424_I2CADDR 					0x22
#define PCA9557_I2CADDR						0x18

#define PCA9557_INPUT						0x00
#define	PCA9557_OUTPUT						0x01
#define PCA9557_POLINVERT					0x02
#define PCA9557_CONFIG						0x03

#define TCA6424_CONFIG_PORT0			0x0c
#define TCA6424_CONFIG_PORT1			0x0d
#define TCA6424_CONFIG_PORT2			0x0e

#define TCA6424_INPUT_PORT0				0x00
#define TCA6424_INPUT_PORT1				0x01
#define TCA6424_INPUT_PORT2				0x02

#define TCA6424_OUTPUT_PORT0			0x04
#define TCA6424_OUTPUT_PORT1			0x05
#define TCA6424_OUTPUT_PORT2			0x06


/* -------------------- Initialize -------------------- */ 

void Initialize_All(void);



/* ----------------------- GPIO ----------------------- */ 

void S800_GPIO_Init(void);
void Flash(char* LED_Name);
void UnFlash(char* LED_Name);
void Twinkle(char* LED_Name, uint32_t flashtime);
bool isPressed(char* Button_Name);
bool isPressed_ad(char* Button_Name);



/* ----------------------- I2C0 ----------------------- */ 

void S800_I2C0_Init(void);
uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);

// LED 1-8
void I2C_Display_LED(int position);
void I2C_Flash_2_LED(int i);
void I2C_UnFlash_2_LED(int i);
void I2C_Flash_All_LED(void);
void I2C_UnFlash_All_LED(void);

// Digit_7segment
void Display_1digit_7segment_char(char content, int position);
void Display_1digit_7segment_int(int content, int position);
void Display_AnyDigit_7segment_int(int content, int position);
void Display_AnyDigit_7segment_char(char* content, int position);



/* ---------------------- Systick ---------------------- */ 

void SystickInitialize(uint32_t frequency);
void Set_Frequency(uint32_t Frequency);



/* ---------------------- Timer ---------------------- */ 

void Display_Time(void);
void Set_Time(int hour, int min, int sec);
void Inc_hour(int Increasement);
void Inc_minute(int Increasement);
void Inc_second(int Increasement);
void Dec_hour(int Decreasement);
void Dec_minute(int Decreasement);
void Dec_second(int Decreasement);
void Inc_Time(int IncHour, int IncMin, int IncSec);
void Dec_Time(int DecHour, int DecMin, int DecSec);



/* ----------------------- UART ----------------------- */ 

void S800_UART_Init(void);
void UARTStringPut(const char *cMessage);
void UARTStringPutNonBlocking(const char *cMessage);
void UARTStringGet(char *msg);
void UARTStringGetNonBlocking(char *msg);


/* ------------------ Other Functions ------------------ */ 

void Delay(uint32_t value);
bool isStringEqual(char *chr1, char chr2[]);
bool is_N_StringEqual(char *chr1, char chr2[], int JudgeLength);
void Update_Button_Status(char* Button_Name, int Num_of_States);
int digit(int n);
char* getCertainString(char *origin, int Length, int StartPosition);
int String2Int(char *ch, int start, int length);
char* Int2String(int num);
int JudgeMonth(char *ch);


