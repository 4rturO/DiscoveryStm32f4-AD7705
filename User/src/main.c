#include "main.h"
#include "ad7705.h"
#include "queue.h"
#include "SPI.h"

TxMessage_t txMessageQueue[8];
RxMessage_t rxMessageQueue[8];

//Создание очередей под отправку и прием
void createQueue( void )
{
    Queue_t * txQueue;
    txQueue = queueGetId(TX_QUEUE_ID);
    queueInit(txQueue, &txMessageQueue, ARR_LENGTH(txMessageQueue), sizeof(TxMessage_t));
    
    Queue_t * rxQueue;
    rxQueue = queueGetId(RX_QUEUE_ID);
    queueInit(rxQueue, &rxMessageQueue, ARR_LENGTH(rxMessageQueue), sizeof(RxMessage_t));
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

//Диспетчер очереди для отправки
//Сравнивает заполненность очередей и доступность ресурсов
void dispatcherTxQueue( void )
{
    //взятие из очереди - queue.c
    Queue_t *queueTx = queueGetId(TX_QUEUE_ID);
    
    //структура для вынимания из очереди
    TxMessage_t txMessage;
    
    //отправка из очереди
    if( getSPITxStatus()==false )
    {
        if( queueDequeue(queueTx, &txMessage) )
        {
            messageSend(&txMessage);
        }
    }
}

//Инициализация AD7705
//Входные параметры: нет
//Выходные параметры: нет
void initPeripheralsAD7705( void ){

    initSPI();
    //включение модуля АЦП
    AD7705_EN 
    initInterruptDRDY();
    initDMAforSPI();
}

int main( void )
{
    __disable_irq();
    createQueue();
    initDiods(); //диоды на отладочной плате. Используются для отладки
    initPeripheralsAD7705();
    initAD7705();
    __enable_irq();
    
    while( 1 )
    {
        dispatcherTxQueue();
        dispatcherRxQueue();
        dispatcherAD7705();
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif
