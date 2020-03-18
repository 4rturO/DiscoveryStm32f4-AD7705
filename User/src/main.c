#include "main.h"
#include "ad7705.h"
#include "queue.h"


TxMessage_t txMessageQueue[32];
RxMessage_t rxMessageQueue[24];
uint16_t ADCvalue;

int main( void )
{
    __disable_irq();
    
    //Создание очередей
    Queue_t * txQueue;
    txQueue = queueGetId(TX_QUEUE_ID);
    queueInit(txQueue, &txMessageQueue, ARR_LENGTH(txMessageQueue), sizeof(TxMessage_t));
    
    Queue_t * rxQueue;
    rxQueue = queueGetId(RX_QUEUE_ID);
    queueInit(rxQueue, &rxMessageQueue, ARR_LENGTH(rxMessageQueue), sizeof(RxMessage_t));
    
    //диоды на отладочной плате
    initDiods();
    
    initAD7705();
    
    GREEN_ON

    //взятие из очереди - внешняя функция из queue.c
    Queue_t *queueRx = queueGetId(RX_QUEUE_ID);
    Queue_t *queueTx = queueGetId(TX_QUEUE_ID);
    
    TxMessage_t txMsg;
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
    
    //структура для вынимания из очереди
    TxMessage_t* txMessage  = getTxMessage();
    //структура для вынимания из очереди
    RxMessage_t rxMsg;
    RxMessage_t *rxMessage = &rxMsg;
    
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
        if( getAD7705ReadyFlag() == true )
        {
//            txMsg.Msg.selectedDevice = AD7705;
//            txMsg.Msg.length = 1;
//            txMsg.Msg.selectedRegister = DATA_16b_REG|AD7705_READ_REG;
//            //занесение в очередь
//            if( queueEnqueue(queueTx, &txMsg) == false )
//            {  RED_ON }
//            
//            txMsg.Msg.selectedDevice = AD7705;
//            txMsg.Msg.length = 2;
//            txMsg.Msg.selectedRegister = 0x00;
//            txMsg.Msg.content[0] = 0x0;
//            //занесение в очередь
//            if( queueEnqueue(queueTx, &txMsg) == false )
//            {  RED_ON }
            txMsg.Msg.selectedDevice = AD7705;
            txMsg.Msg.length = 3;
            txMsg.Msg.selectedRegister = DATA_16b_REG|AD7705_READ_REG;
            txMsg.Msg.content[0] = 0x00;
            txMsg.Msg.content[1] = 0x00;
            //занесение в очередь
            if( queueEnqueue(queueTx, &txMsg) == false )
            {  RED_ON }
            
            ORANGE_ON
        }
        if( !queueIsEmpty(queueRx) )
        {
            if( queueDequeue(queueRx, rxMessage) )
            {
                ADCvalue = rxMessage->Msg.content[1] + (rxMessage->Msg.content[0]<<8);
            }
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
