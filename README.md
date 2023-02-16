# STM32-JumpToBootloader
Program allow to jump to built-in bootloader without shorting pins on Nucleo-L476RG board  
(but simple customizable through replace HAL library)

## Description
***
When program run, then green led blink one time per second.  
After click user button, the program try to go to bootloader, then the led blink fast few times.  
You could check if the program jumped correctly by using stm32 programmer and read mapped memory on 0x0000 0000.  
If data match with address 0x1FFF 0000 (system memory) then jump executed correctly.  
If not - the program will call NVIC_SystemReset().  
Use this code as fundamental ingredient of your own bootloader. Have fun :)
