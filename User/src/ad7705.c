#include "ad7705.h"
#include "queue.h"

//флаг истинный если модуль АЦП подготовил данные для передачи
static volatile bool readyFlag = false;

uint16_t ADCvalue;
uint8_t testComm = 1;   //счетчик команд для тестового сценария

//Диспетчер очереди для приема
//Проверяет, пришло ли сообщение
void dispatcherRxQueue( void )
{
    //взятие из очереди - queue.c
    Queue_t *queueRx = queueGetId(RX_QUEUE_ID);
    
    //структура для вынимания из очереди
    RxMessage_t rxMsg;
    RxMessage_t *rxMessage = &rxMsg;
    
    //прием из очереди
    if( !queueIsEmpty(queueRx) )
    {
        if( queueDequeue(queueRx, rxMessage) )
        {
            ADCvalue = rxMessage->Msg.content[0] + (rxMessage->Msg.content[1]<<8);
            if( ADCvalue == (DEF_SETUP_REG) )
            { BLUE_ON }
            else
            { BLUE_OFF }
        }
    }
}

//Диспетчер AD7705
//Реализует управление FSM для AD7705
void dispatcherAD7705( void )
{
    if( getAD7705ReadyFlag() && ( getSPITxStatus()==false ) )   
    {
        if(testComm)
        {
            if( writeAD7705(SETUP_REG|AD7705_READ_REG, 0x00) == false )
            {  RED_ON }
            //testComm = 0;
        }
    }  
}

//Инициализация AD7705 начальными значениями
void initAD7705( void )
{
    writeAD7705(CLOCK_REG, DEF_CLOCK_REG);
    writeAD7705(SETUP_REG, ADC_SELF|DEF_SETUP_REG);
}

//Функция чтения регистра AD7705 - заготовка.
//Входные параметры: нет
//Выходные параметры: нет
bool readAD7705(){
     return true;
}

//Функция записи в регистр AD7705
//Входные параметры: имя регистра, указатель на очередь
//Выходные параметры: успешность записи в очередь (что не корректно! тогда надо назвать ф-ию writeAndAddInQueue или как-то так!)
bool writeAD7705(uint8_t regName, uint32_t regContain){
    
    Queue_t *queueTx = queueGetId(TX_QUEUE_ID);
    TxMessage_t txMsg;
    //Для корректной работы нужно также поменять размер Rx сообщения
    txMsg.Msg.selectedDevice = AD7705;
    txMsg.Msg.selectedRegister = regName;
    //Выбор размера сообщения в зависимости от типа регистра
    if( (regName&0x30) == OFFSET_24b_REG || regName == GAIN_24b_REG )
    {
        txMsg.Msg.length = 4;
        txMsg.Msg.content[0] = regContain;
        txMsg.Msg.content[1] = regContain>>8;
        txMsg.Msg.content[2] = regContain>>16;
    }
    else if( (regName&0x30) == DATA_16b_REG )  //можно ли записывать в DataRegister?
    {
        txMsg.Msg.length = 3;
        txMsg.Msg.content[0] = regContain;
        txMsg.Msg.content[1] = regContain>>8;
    }
    else
    {
        txMsg.Msg.length = 2;
        txMsg.Msg.content[0] = regContain;
    }
    //занесение в очередь
    return queueEnqueue(queueTx, &txMsg);
}

//Получение статуса готовности данных в AD7705
//Входные параметры: нет
//Выходные параметры: состояние флага. ИСТИНА если данные готовы для считывания
bool getAD7705ReadyFlag(){
    
    return readyFlag;
}

//Получение статуса готовности данных в AD7705
//Входные параметры: нет
//Выходные параметры: состояние флага. ИСТИНА если данные готовы для считывания
void setAD7705ReadyFlag( bool newState ){
    
    readyFlag = newState;
}

//Обработчик прерываний от ножки RDY AD7705
//Входные параметры: нет
//Выходные параметры: нет
void EXTI15_10_IRQHandler(void){    
    
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
    {
        readyFlag = true;
        EXTI_ClearITPendingBit(EXTI_Line11);    
    }

}

//Настройка прерывания по ножке DRDY
//Низкий уровень показывает готовность данных для считывания
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
