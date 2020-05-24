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


/* ------------------ Other Functions ------------------ */ 

void Delay(uint32_t value);
bool isStringEqual(char *chr1, char chr2[]);
bool is_N_StringEqual(char *chr1, char chr2[], int JudgeLength);
void Update_Button_Status(char* Button_Name, int Num_of_States);








