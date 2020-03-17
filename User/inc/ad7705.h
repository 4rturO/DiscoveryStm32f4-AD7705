#pragma once
#include "stm32f4xx.h"

void initDiods( void );
void initSPI( void );

#define GREEN_ON GPIO_SetBits(GPIOD, GPIO_Pin_12);
#define GREEN_OFF GPIO_ResetBits(GPIOD, GPIO_Pin_12);
#define ORANGE_ON GPIO_SetBits(GPIOD, GPIO_Pin_13);
#define ORANGE_OFF GPIO_ResetBits(GPIOD, GPIO_Pin_13);
#define RED_ON GPIO_SetBits(GPIOD, GPIO_Pin_14);
#define RED_OFF GPIO_ResetBits(GPIOD, GPIO_Pin_14);
#define BLUE_ON GPIO_SetBits(GPIOD, GPIO_Pin_15);
#define BLUE_OFF GPIO_ResetBits(GPIOD, GPIO_Pin_15);

#define CS_EN GPIO_ResetBits(GPIOB, GPIO_Pin_12);
#define CS_DIS GPIO_SetBits(GPIOB, GPIO_Pin_12);
//Operation modes
#define ADC_NORMAL 0x00
#define ADC_SELF 0x40
#define ADC_ZERO_SCALE 0x80
#define ADC_FULL_SCALE 0xc0

//Gain settings
#define ADC_GAIN_1 0x00
#define ADC_GAIN_2 0x08
#define ADC_GAIN_4 0x10
#define ADC_GAIN_8 0x18
#define ADC_GAIN_16 0x20
#define ADC_GAIN_32 0x28
#define ADC_GAIN_64 0x30
#define ADC_GAIN_128 0x38

//Polar operations
#define ADC_BIPOLAR 0x04
#define ADC_UNIPOLAR 0x00

//update rates
#define ADC_50 0x04
#define ADC_60 0x05
#define ADC_250 0x06
#define ADC_500 0x07
