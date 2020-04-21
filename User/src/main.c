#include "main.h"
#include "ad7705.h"
#include "queue.h"

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
