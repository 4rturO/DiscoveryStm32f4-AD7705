#include "main.h"
#include "ad7705.h"
#include "queue.h"


TxMessage_t txMessage[24];
TxMessage_t txMsg;

int main( void )
{
    __disable_irq();
    
    Queue_t * txQueue;
    txQueue = queueGetId(TX_QUEUE_ID);
    queueInit(txQueue, &txMessage, ARR_LENGTH(txMessage), sizeof(TxMessage_t));
    
    //диоды на отладочной плате
    initDiods();
    initAD7705();
    
    GREEN_ON

    //взятие из очереди - внешняя функция из queue.c
    //Queue_t *queueRx = queueGetId(RX_QUEUE_ID);
    Queue_t *queueTx = queueGetId(TX_QUEUE_ID);
    TxMessage_t* txMessage  = getTxMessage();
    
    txMsg.Msg.selectedDevice = AD7705;
    txMsg.Msg.length = 2;
    txMsg.Msg.selectedRegister = CLOCK_REG;
    txMsg.Msg.content[0] = ADC_500;
    //занесение в очередь
    queueEnqueue(queueTx, &txMsg);
    
    txMsg.Msg.selectedDevice = AD7705;
    txMsg.Msg.length = 2;
    txMsg.Msg.selectedRegister = SETUP_REG;
    txMsg.Msg.content[0] = ADC_SELF|ADC_GAIN_1|ADC_BIPOLAR;
    //занесение в очередь
    queueEnqueue(queueTx, &txMsg);
    
    __enable_irq();
    
    while( 1 )
    {
        
        //вынимание из очереди
        if( getSPITxStatus()==false )
        {
            if( queueDequeue(queueTx, txMessage) )
            {
                messageSend(txMessage);
            }
        }
        if( getAD7705ReadeFlag()==true )
        {
            ORANGE_ON
        }
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
