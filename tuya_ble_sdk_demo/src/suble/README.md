//该文件记录一些重要信息和临时信息



/*********************************************************
FN: 重要信息
*/
tuya_ble_sdk改动的地方：授权、串口模拟硬件通道


功耗：
低压模式：2uA左右
sleep模式：6uA左右


IRQ: Interrupt Request
FIQ: Fast Interrupt Request


官方源码中，在400e3地址写入Mac地址，后面写了16字节其他数据，该页无其他数据


HZ32000 —— 32K 时钟源切换开关
MCU_CLK_64M


rf_init //花费约1.5s时间


收到从机发来的数据 - gattc_event_ind_handler


编译冲突改动的位置:
stack_rom_symbol.txt
#0x00017e91 T __aeabi_memclr
#0x00017e91 T __aeabi_memclr4
#0x00017e91 T __aeabi_memclr8







/*********************************************************
FN: 临时信息
*/











