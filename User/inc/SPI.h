#pragma once
#include "stm32f4xx.h"
#include <stdbool.h>
#include "ad7705.h"

#define SPI2_RX_DMA_HANDLER DMA1_Stream3_IRQHandler
#define SPI2_TX_DMA_HANDLER DMA1_Stream4_IRQHandler

#define RED_ON GPIO_SetBits(GPIOD, GPIO_Pin_14);
#define RED_OFF GPIO_ResetBits(GPIOD, GPIO_Pin_14);

//Chip Select
#define AD7705_CS_EN GPIO_ResetBits(GPIOB, GPIO_Pin_12);
#define AD7705_CS_DIS GPIO_SetBits(GPIOB, GPIO_Pin_12);

void initDMAforSPI( void );
void initSPI( void );
void messageSend(TxMessage_t* message);
