#include "./flash_if/flash_if.h"
#include "./iap/iap.h"

/*
	flash�����ܹ�256k 128ҳ ÿҳ2k 
	bootloader             8ҳ       16k                 0x08000000-----0x08003FFF
    �û�������             2ҳ	     4k				     0x08004000-----0x08004FFF
	APP������              59ҳ      118k                0x08005000-----0x080227FF
	APP������              59ҳ      118k                0x08022800-----0x0803FFFF
	APP����������          0ҳ       0k                
	
	
*/


#define APP_START_ADDR                  0x08005000
#define APP_PAGE                        59
#define APP_BAK_START_ADDR              0x08022800
#define APP_BAK_PAGE                    59


#define APP_BAK_FLAG_ADDR               0x08004000
#define APP_BAK_FLAG_VAL                0x1A1A1A1A
#define APP_BAK_FLAG_CHECK()            (*(volatile uint32_t*)(APP_BAK_FLAG_ADDR) != (APP_BAK_FLAG_VAL))
#define	APP_BAK_FLAG_ERASE()            FLASH_If_Erase(APP_BAK_FLAG_ADDR, 1)  			//����������־λ   

#define APP_STACK_ADDR_MASK         0xFFFF0000
#define APP_INITIAL_SP              (*(volatile uint32_t*)APP_START_ADDR) //������ʼ��ַ��ŵ���ջ����ַ
#define APP_IRAM_START_ADDR         0x20000000 //��MDK:Options->Target->IRAMx->Start


/*
	��תAPP����Ӧ���� ��ʼλ

*/
void JumpToApp(void)
{
		void (*pJumpToApp)(void); //���庯��ָ�����
		if((APP_INITIAL_SP & APP_STACK_ADDR_MASK) ==  APP_IRAM_START_ADDR ) //���Ӧ�ó���(APP)ջ����ַ�Ƿ�Ϸ���0x1FFF8000---0X1FFFFFFF
		{
					printf("        APP start address : 0x%08X\r\n", APP_START_ADDR);
					printf("Jump to APP reset address : 0x%08X\r\n", APP_START_ADDR + 4);
					HAL_Delay(1); //������ʱ ����� printf ���ϸ�����β��
			 pJumpToApp = (void(*)(void))*(volatile uint32_t*)(APP_START_ADDR + 4);
			__set_MSP(APP_INITIAL_SP); //���� Ӧ�ó���(APP)�� ջָ��
			for(uint8_t i=0; i<8; i++) //�رյ����жϣ�Ӧ�ó����ö�������ôд������ôд
            {
                NVIC->ICER[i] = 0xFFFFFFFF;	//�ر��ж�
                NVIC->ICPR[i] = 0xFFFFFFFF;	//����жϱ�־λ
            }
			pJumpToApp(); //��ת��Ӧ�ó���(APP)��λ��ַ��.
		}
}
/*
	����app��
	���̣�����app��Ȼ��ӱ�������������д��app��

*/
char UpgradeApp(void)
{
	//һ���ж��Ƿ���Ҫ����
	if(APP_BAK_FLAG_CHECK() )
	{
		printf("No upgrade file\r\n");
        return UPGRADE_NONE; //����������
	}	
	//��������APP��
    FLASH_If_Init();
    if(FLASH_If_Erase(APP_START_ADDR, APP_PAGE))
    {
        printf("Erase mcu flash error\r\n");
        return UPGRADE_FAIL;        
    }
	//����//д������
    uint32_t i = 0;
    while(i < APP_PAGE*FLASH_PAGE_SIZE)
    {
        uint32_t buf[256]; //�ֽ��� = 1024
        uint32_t bufLen = 256;
        FLASH_If_Read((APP_BAK_START_ADDR + i), buf, bufLen); //�������� APP
        if( FLASH_If_Write((APP_START_ADDR + i), buf, bufLen) ) //д APP ʧ��
        {
            printf("Write mcu flash start address : 0x%07X bufLen : %d Bytes fail\r\n", (APP_START_ADDR + i), bufLen * 4);
            return UPGRADE_FAIL;
        }
//        printf("Write mcu flash start address : 0x%07X bufLen : %d Bytes\r\n", (APP_START_ADDR + i), bufLen * 4);
        i += bufLen * 4;
    }
	//�ġ�������־λ
	if(APP_BAK_FLAG_ERASE())
	{
		printf("Erase app upgrade flag error\r\n");
        return UPGRADE_FAIL;
	}
	printf("Upgrade OK\r\n");
	return UPGRADE_SUCCESS;
}


