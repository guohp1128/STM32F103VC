
#include "main.h"
#include "usart.h"


uint8_t dbgBuf[100];
int dbgRxIdx = 0;


void user_dbg_rec_data(uint8_t data);

/**
 *******************************************************************************
 * @brief   �ض���c�� fputc ����
 * @note    printf������
 *******************************************************************************
 */
int fputc(int ch, FILE *f)
{
		HAL_UART_Transmit(&huart1 , (uint8_t *)&ch, 1, 0xFFFF);
		return ch;
}

void BspDebugRecOpen(void)
{
    HAL_UART_Receive_IT(&huart1,dbgBuf,1);
}


//���ڽ��ջص�
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        user_dbg_rec_data(dbgBuf[0]);

        BspDebugRecOpen();
    }
   
    
}


//debug ���ڽ������ݽ��� ����
void user_dbg_rec_data(uint8_t data)
{
    static char rxbuf[100];
    static uint8_t rxidx = 0;
    
    rxbuf[rxidx ++] = data;
//    printf("%c",rxbuf[rxidx]);

}
    





