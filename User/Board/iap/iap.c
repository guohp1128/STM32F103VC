#include "./flash_if/flash_if.h"
#include "./iap/iap.h"

/*
	flash分配总共256k 128页 每页2k 
	bootloader             8页       16k                 0x08000000-----0x08003FFF
    用户数据区             2页	     4k				     0x08004000-----0x08004FFF
	APP程序区              59页      118k                0x08005000-----0x080227FF
	APP备份区              59页      118k                0x08022800-----0x0803FFFF
	APP出厂程序区          0页       0k                
	
	
*/


#define APP_START_ADDR                  0x08005000
#define APP_PAGE                        59
#define APP_BAK_START_ADDR              0x08022800
#define APP_BAK_PAGE                    59


#define APP_BAK_FLAG_ADDR               0x08004000
#define APP_BAK_FLAG_VAL                0x1A1A1A1A
#define APP_BAK_FLAG_CHECK()            (*(volatile uint32_t*)(APP_BAK_FLAG_ADDR) != (APP_BAK_FLAG_VAL))
#define	APP_BAK_FLAG_ERASE()            FLASH_If_Erase(APP_BAK_FLAG_ADDR, 1)  			//擦除升级标志位   

#define APP_STACK_ADDR_MASK         0xFFFF0000
#define APP_INITIAL_SP              (*(volatile uint32_t*)APP_START_ADDR) //程序起始地址存放的是栈顶地址
#define APP_IRAM_START_ADDR         0x20000000 //见MDK:Options->Target->IRAMx->Start


/*
	跳转APP程序应用区 起始位

*/
void JumpToApp(void)
{
		void (*pJumpToApp)(void); //定义函数指针变量
		if((APP_INITIAL_SP & APP_STACK_ADDR_MASK) ==  APP_IRAM_START_ADDR ) //检测应用程序(APP)栈顶地址是否合法。0x1FFF8000---0X1FFFFFFF
		{
					printf("        APP start address : 0x%08X\r\n", APP_START_ADDR);
					printf("Jump to APP reset address : 0x%08X\r\n", APP_START_ADDR + 4);
					HAL_Delay(1); //不加延时 上面的 printf 会拖个乱码尾巴
			 pJumpToApp = (void(*)(void))*(volatile uint32_t*)(APP_START_ADDR + 4);
			__set_MSP(APP_INITIAL_SP); //设置 应用程序(APP)的 栈指针
			for(uint8_t i=0; i<8; i++) //关闭单个中断，应用程序不用动，该怎么写还是怎么写
            {
                NVIC->ICER[i] = 0xFFFFFFFF;	//关闭中断
                NVIC->ICPR[i] = 0xFFFFFFFF;	//清除中断标志位
            }
			pJumpToApp(); //跳转到应用程序(APP)复位地址处.
		}
}
/*
	升级app区
	流程：擦除app区然后从备份区复制数据写到app区

*/
char UpgradeApp(void)
{
	//一、判断是否需要升级
	if(APP_BAK_FLAG_CHECK() )
	{
		printf("No upgrade file\r\n");
        return UPGRADE_NONE; //无升级操作
	}	
	//二、擦除APP段
    FLASH_If_Init();
    if(FLASH_If_Erase(APP_START_ADDR, APP_PAGE))
    {
        printf("Erase mcu flash error\r\n");
        return UPGRADE_FAIL;        
    }
	//三、//写入数据
    uint32_t i = 0;
    while(i < APP_PAGE*FLASH_PAGE_SIZE)
    {
        uint32_t buf[256]; //字节数 = 1024
        uint32_t bufLen = 256;
        FLASH_If_Read((APP_BAK_START_ADDR + i), buf, bufLen); //读备份区 APP
        if( FLASH_If_Write((APP_START_ADDR + i), buf, bufLen) ) //写 APP 失败
        {
            printf("Write mcu flash start address : 0x%07X bufLen : %d Bytes fail\r\n", (APP_START_ADDR + i), bufLen * 4);
            return UPGRADE_FAIL;
        }
//        printf("Write mcu flash start address : 0x%07X bufLen : %d Bytes\r\n", (APP_START_ADDR + i), bufLen * 4);
        i += bufLen * 4;
    }
	//四、擦除标志位
	if(APP_BAK_FLAG_ERASE())
	{
		printf("Erase app upgrade flag error\r\n");
        return UPGRADE_FAIL;
	}
	printf("Upgrade OK\r\n");
	return UPGRADE_SUCCESS;
}


