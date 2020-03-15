#include "ad7705.h"

#define RX_SIZE 6
#define TX_SIZE 4

uint8_t rxBuffer[RX_SIZE] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
uint8_t txBuffer[TX_SIZE] = {0x20, 0xC0|ADC_500, 0x10, ADC_SELF|ADC_GAIN_1|ADC_BIPOLAR};

void SPI2_IRQHandler(void){

    static uint8_t counter = 0;
    CS_DIS
    if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_IT_TXE) != SET) {
        CS_EN
        SPI2->DR = txBuffer[counter++];
        if(counter == TX_SIZE)
        {
            SPI_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
        }
    }
    
    if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET) {
      rxBuffer[0] = SPI1->DR;
    }
}
//Rx
void DMA1_Stream3_IRQHandler(void){
    
    RED_ON
    if(DMA_GetITStatus(DMA1_Stream3, DMA_IT_HTIF3)==SET)
    {
        DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_HTIF3);
    }
    if(DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3)==SET)
    {
        DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);
    }
}

//Tx
void DMA1_Stream4_IRQHandler(void){
    
    if(DMA_GetITStatus(DMA1_Stream4, DMA_IT_HTIF4)==SET)
    {
        DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_HTIF4);
    }
    if(DMA_GetITStatus(DMA1_Stream4, DMA_IT_TCIF4)==SET)
    {
        // Ждем последний байт
        while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
        while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
        
        BLUE_ON
        CS_DIS
        DMA_Cmd(DMA1_Stream4, DISABLE);
        DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
    }
}

void EXTI15_10_IRQHandler(void){    
    
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
    {
        ORANGE_ON
        EXTI_ClearITPendingBit(EXTI_Line11);    
    }

}

void initSPI( void ){
    
    GPIO_InitTypeDef    GPIO_InitStructure;
    DMA_InitTypeDef     DMA_InitStructure;
    EXTI_InitTypeDef    EXTI_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;
    
    GPIO_StructInit(&GPIO_InitStructure);
    DMA_StructInit(&DMA_InitStructure);
    EXTI_StructInit(&EXTI_InitStructure);
    
    
    /* Peripheral Clock Enable -------------------------------------------------*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    /* GPIO for SPI configuration ----------------------------------------------*/
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10|GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    CS_EN
    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
    //GPIO_SetBits(GPIOB, GPIO_Pin_10|GPIO_Pin_12);
    
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource11);
    /* Configure EXTI Line11 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line11;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    /* Enable and set EXTI Line0 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0E;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
      /* Connect SPI pins to AF5 */  
//    GPIO_PinAFConfig(GPIOD, GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15, GPIO_AF_SPI2);
//    GPIO_PinAFConfig(GPIOD, GPIO_Pin_14, GPIO_AF_SPI2);    
//    GPIO_PinAFConfig(GPIOD, GPIO_Pin_15, GPIO_AF_SPI2);
    
    SPI_InitTypeDef SPI_InitStructure;
    SPI_StructInit(&SPI_InitStructure);
    
    /* SPI configuration -------------------------------------------------------*/
    SPI_I2S_DeInit(SPI2);
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &SPI_InitStructure);
    
    /* Configure DMA Initialization Structure */
    DMA_InitStructure.DMA_BufferSize = RX_SIZE;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(SPI2->DR)) ;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_Channel = DMA_Channel_0;
    /* Configure TX DMA */
    DMA_InitStructure.DMA_BufferSize = TX_SIZE;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
    DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)&txBuffer ;
    DMA_Init(DMA1_Stream4, &DMA_InitStructure);
    /* Configure RX DMA */
    DMA_InitStructure.DMA_BufferSize = RX_SIZE;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
    DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)&rxBuffer ; 
    DMA_Init(DMA1_Stream3, &DMA_InitStructure);
    
    /* Enable DMA SPI TX Stream */
    DMA_Cmd(DMA1_Stream4,ENABLE);

    /* Enable DMA SPI RX Stream */
    DMA_Cmd(DMA1_Stream3,ENABLE);  

//    /* Enable SPI DMA TX Requsts */
//    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

//    /* Enable SPI DMA RX Requsts */
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);

//    /* Enable the SPI peripheral */
    SPI_Cmd(SPI2, ENABLE);
    DMA_ITConfig(DMA1_Stream3, DMA_IT_HT, ENABLE);
    DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
//    DMA_ITConfig(DMA1_Stream4, DMA_IT_HT, ENABLE);
//    DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);
//    NVIC_EnableIRQ(DMA1_Stream3_IRQn);
//    NVIC_EnableIRQ(DMA1_Stream4_IRQn);
    
    //SPI_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
    SPI_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
    NVIC_EnableIRQ(SPI2_IRQn);
}

void initDiods( void ){
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    //диоды
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

}
