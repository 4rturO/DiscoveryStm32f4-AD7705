#include "main.h"
#include "ad7705.h"

int main( void )
{
    __disable_irq();
    initDiods();
    initSPI();
    
    GREEN_ON

//    /* Disable DMA SPI TX Stream */
//    DMA_Cmd(DMA1_Stream4, DISABLE);

//    /* Disable DMA SPI RX Stream */
//    DMA_Cmd(DMA1_Stream3, DISABLE);  

//    /* Disable SPI DMA TX Requsts */
//    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, DISABLE);

//    /* Disable SPI DMA RX Requsts */
//    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, DISABLE);

//    /* Disable the SPI peripheral */
//    SPI_Cmd(SPI2, DISABLE); 

    __enable_irq();
    while( 1 )
    {
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
