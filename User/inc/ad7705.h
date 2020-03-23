#pragma once
#include "stm32f4xx.h"
#include <stdbool.h>



//вкл/выкл светодиодов на отладочной плате
#define GREEN_ON GPIO_SetBits(GPIOD, GPIO_Pin_12);
#define GREEN_OFF GPIO_ResetBits(GPIOD, GPIO_Pin_12);
#define ORANGE_ON GPIO_SetBits(GPIOD, GPIO_Pin_13);
#define ORANGE_OFF GPIO_ResetBits(GPIOD, GPIO_Pin_13);
#define RED_ON GPIO_SetBits(GPIOD, GPIO_Pin_14);
#define RED_OFF GPIO_ResetBits(GPIOD, GPIO_Pin_14);
#define BLUE_ON GPIO_SetBits(GPIOD, GPIO_Pin_15);
#define BLUE_OFF GPIO_ResetBits(GPIOD, GPIO_Pin_15);

//Chip Select
#define AD7705_CS_EN GPIO_ResetBits(GPIOB, GPIO_Pin_12);
#define AD7705_CS_DIS GPIO_SetBits(GPIOB, GPIO_Pin_12);

//Reset
#define AD7705_EN GPIO_SetBits(GPIOB, GPIO_Pin_10);
#define AD7705_RESET GPIO_ResetBits(GPIOB, GPIO_Pin_10);

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

typedef enum {
    AD7705
} Device_t;

//register selection
#define COMMUNICATION_REG 0x00
#define SETUP_REG   0x10
#define CLOCK_REG   0x20
#define DATA_16b_REG    0x30
#define TEST_REG    0x40
#define NO_OPERATION_REG    0x50
#define OFFSET_24b_REG  0x60
#define GAIN_24b_REG    0x70

//r/w selection
#define AD7705_READ_REG 0x08
#define AD7705_WRITE_REG   0x00

//standby
#define STANDBY_MODE 0x04

//channel select
#define FIRST_PLUS_FIRST_MINUS 0x00
#define SECOND_PLUS_SECOND_MINUS 0x01
#define FIRST_MINUS_FIRST_MINUS 0x02
#define FIRST_MINUS_SECOND_MINUS 0x03

typedef union {
    uint8_t txData[7];
    #pragma pack(push, 1)
    struct 
    {
        Device_t selectedDevice;             //[0]
        uint8_t length;                     //[1]
        uint8_t selectedRegister;           //[2]
        uint8_t content[4];                 //[3-6]
    }Msg;
    #pragma pack(pop)
}TxMessage_t;

typedef union {
    uint8_t rxData[7];
    #pragma pack(push, 1)
    struct 
    {
        Device_t selectedDevice;             //[0]
        uint8_t length;                     //[1]
        uint8_t selectedRegister;           //[2]
        uint8_t content[4];                 //[3-6]
    }Msg;
    #pragma pack(pop)
}RxMessage_t;

bool writeAD7705(uint8_t regName, uint32_t regContain);
void initAD7705( void );
void initSPI( void );
void initInterruptDRDY( void );
void initDMAforSPI( void );
void initDiods( void );
bool getSPITxStatus( void );
bool getAD7705ReadyFlag( void );
TxMessage_t* getTxMessage( void );
void messageSend(TxMessage_t* message);
