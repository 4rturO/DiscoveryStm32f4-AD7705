#include "ad7705.h"
#include "queue.h"

#define RX_SIZE 2
#define TX_SIZE 4

uint8_t rxBuffer[RX_SIZE];
//uint8_t txBuffer[TX_SIZE] = {0x20, ADC_500, 0x10, ADC_SELF|ADC_GAIN_1|ADC_BIPOLAR};
uint8_t txBuffer[TX_SIZE];

TxMessage_t transferMsg;
uint8_t transferSize;
RxMessage_t rxMsg;

//флаг истинный пока происходит передача сообщения
static volatile bool txBusy = false;

//флаг истинный если модуль АЦП подготовил данные для передачи
static volatile bool readyFlag = false;

//флаг чтения данных
static volatile bool readFlag = false;

void delay( void ){
    
    for( uint32_t i = 0; i<500; i++)
    {}
}

bool readAD7705(){
     return true;
}

bool writeAD7705(uint8_t regName, uint32_t regContain){
    
    Queue_t *queueTx = queueGetId(TX_QUEUE_ID);
    TxMessage_t txMsg;
    
    txMsg.Msg.selectedDevice = AD7705;
    if( (regName&0x30) == OFFSET_24b_REG || regName == GAIN_24b_REG )
    {
        txMsg.Msg.length = 4;
        txMsg.Msg.selectedRegister = regName;
        txMsg.Msg.content[0] = regContain;
        txMsg.Msg.content[1] = regContain>>8;
        txMsg.Msg.content[2] = regContain>>16;
    }
    else if( (regName&0x30) == DATA_16b_REG )  //можно ли записывать в DataRegister?
    {
        txMsg.Msg.length = 3;
        txMsg.Msg.selectedRegister = regName;
        txMsg.Msg.content[0] = regContain;
        txMsg.Msg.content[1] = regContain>>8;
    }
    else
    {
        txMsg.Msg.length = 2;
        txMsg.Msg.selectedRegister = regName;
        txMsg.Msg.content[0] = regContain;
    }
    
    //занесение в очередь
    return queueEnqueue(queueTx, &txMsg);
}

bool getSPITxStatus(){
    
    return txBusy;
}

bool getAD7705ReadyFlag(){
    
    return readyFlag;
}

TxMessage_t* getTxMessage()
{
    return &transferMsg;
}

void messageSend(TxMessage_t* message){
    
    //DMA_Cmd(DMA1_Stream3, DISABLE);
    //!не понятно как без плясок с буфером сделать универсальную запись!
    txBuffer[0] = message->Msg.selectedRegister;
    txBuffer[1] = message->Msg.content[0];
    transferSize = message->Msg.length;
    if(message->Msg.selectedRegister & AD7705_READ_REG)
    {
        readFlag = true;
        SPI_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
        //DMA1_Stream3->NDTR = transferSize;
        //DMA_Cmd(DMA1_Stream3, ENABLE);
    }
    //выбор устройства
    if(message->Msg.selectedDevice == AD7705)
    {
        AD7705_CS_EN
    }
    SPI_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
    txBusy = true;
}

void SPI2_IRQHandler(void){

    static uint8_t counterTx = 0;
    static uint8_t counterRx = 0;
    
    if( (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == SET) && (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != SET) ) {
        
        if( counterTx < transferSize )
        {  
            //побайтовая передача
            SPI2->DR = txBuffer[counterTx++];
        }
        else if(counterTx >= transferSize)
        {
            //отключение прерывания
            SPI_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
            //отключение выбора устройства
//            if( txBuffer[0]& AD7705_READ_REG )
//            { delay();}
            AD7705_CS_DIS
            //дописать сюда другие варианты
            
            //передатчик больше не занят
            txBusy = false;
            //начинается ожидание ответа от модуля
            readyFlag = false;
            counterTx = 0;
        }
        
    }
    
    if( (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET) && readFlag ) {
        if( counterRx<RX_SIZE )
        {
            rxBuffer[counterRx++] = SPI2->DR;
        }
        else
        {
            readFlag = false;
            counterRx = 0;
            SPI_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
            //взятие из очереди - внешняя функция из queue.c
            Queue_t *queueRx = queueGetId(RX_QUEUE_ID);
            rxMsg.Msg.selectedDevice = AD7705;
            rxMsg.Msg.length = 2;
            rxMsg.Msg.content[0] = rxBuffer[0];
            rxMsg.Msg.content[1] = rxBuffer[1];
            if( queueEnqueue(queueRx, &rxMsg) == false )
            {  RED_ON }
        }
        SPI_I2S_ClearFlag(SPI2, SPI_I2S_FLAG_RXNE);
    }
}
//Rx
void DMA1_Stream3_IRQHandler(void){
    
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
        DMA_Cmd(DMA1_Stream4, DISABLE);
        DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
    }
}

void EXTI15_10_IRQHandler(void){    
    
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
    {
        readyFlag = true;
        EXTI_ClearITPendingBit(EXTI_Line11);    
    }

}

void initAD7705( void ){

    initSPI();
    initInterruptDRDY();
    initDMAforSPI();
}

void initSPI( void ){
    //Включение тактирования
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    GPIO_InitTypeDef    GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    // Настройка AF для ножек SPI
    GPIOB->AFR[1] |= 0x55500000;
    // Следующие функции не приводят к изменению регистра AFR
    // Более того при попытке сконфигурировать биты для 14 пина контроллер падает в HardFault
//    GPIO_PinAFConfig(GPIOB, GPIO_Pin_13, GPIO_AF_SPI2);
//    GPIO_PinAFConfig(GPIOB, GPIO_Pin_15, GPIO_AF_SPI2);
//    GPIO_PinAFConfig(GPIOB, GPIO_Pin_14, GPIO_AF_SPI2);

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
    //включение модуля АЦП
    AD7705_EN   
   
    //Настройка SPI
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    SPI_InitTypeDef SPI_InitStructure;
    SPI_StructInit(&SPI_InitStructure);

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
    //Включение SPI
    SPI_Cmd(SPI2, ENABLE);
    //Включение прерываний для SPI
    //SPI_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
    //SPI_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
    NVIC_EnableIRQ(SPI2_IRQn);
}

void initInterruptDRDY( void ){
    
    //DRDY - выход с модуля показывающий готовность к работе
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    EXTI_InitTypeDef    EXTI_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;
    EXTI_StructInit(&EXTI_InitStructure);
    GPIO_InitTypeDef    GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource11);
    EXTI_InitStructure.EXTI_Line = EXTI_Line11;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0E;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
}

void initDMAforSPI( void ){
    
    //Настройка DMA для SPI
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    DMA_InitTypeDef     DMA_InitStructure;
    DMA_StructInit(&DMA_InitStructure);
    
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
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_BufferSize = RX_SIZE;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
    DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)&rxBuffer ; 
    DMA_Init(DMA1_Stream3, &DMA_InitStructure);
    
    /* Enable DMA SPI TX Stream */
//    DMA_Cmd(DMA1_Stream4,ENABLE);

    /* Enable DMA SPI RX Stream */
//    DMA_Cmd(DMA1_Stream3,ENABLE);  

//    /* Enable SPI DMA TX Requsts */
//    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

//    /* Enable SPI DMA RX Requsts */
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);

//    /* Enable the SPI peripheral */
    
//    DMA_ITConfig(DMA1_Stream3, DMA_IT_HT, ENABLE);
    DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
//    DMA_ITConfig(DMA1_Stream4, DMA_IT_HT, ENABLE);
//    DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);
    NVIC_EnableIRQ(DMA1_Stream3_IRQn);
//    NVIC_EnableIRQ(DMA1_Stream4_IRQn);
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
