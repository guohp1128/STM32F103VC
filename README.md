# 项目介绍

stm32f103vc bootloader 项目。

# flash地址分配

flash总共256k 128页 每页2k 
bootloader              8页         16k                   0x08000000-----0x08003FFF
用户数据区               2页	     4k				     0x08004000-----0x08004FFF
APP程序区                59页      118k                 0x08005000-----0x080227FF
APP备份区                59页      118k                 0x08022800-----0x0803FFFF
APP出厂程序区        0页         0k         
