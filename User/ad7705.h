#pragma once
#include "stm32f4xx.h"

void initDiods( void );
void initSPI( void );

#define GREEN_ON GPIO_SetBits(GPIOD, GPIO_Pin_12);
#define GREEN_OFF GPIO_SetBits(GPIOD, GPIO_Pin_12);
#define ORANGE_ON GPIO_SetBits(GPIOD, GPIO_Pin_13);
#define ORANGE_OFF GPIO_SetBits(GPIOD, GPIO_Pin_13);
#define RED_ON GPIO_SetBits(GPIOD, GPIO_Pin_14);
#define RED_OFF GPIO_SetBits(GPIOD, GPIO_Pin_14);
#define BLUE_ON GPIO_SetBits(GPIOD, GPIO_Pin_15);
#define BLUE_OFF GPIO_SetBits(GPIOD, GPIO_Pin_15);
