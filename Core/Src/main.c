/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * Function to perform jump to system memory boot from user application
 *
 * Call function when you want to jump to system memory
 */
void JumpToBootloader(void)
{
	typedef void (*jumpFunction)(void);
	/**
	 * Step: Set system memory address. For STM32L476 System memory address: 0x1FFF 0000 (28 kB)
	 */
	volatile uint32_t sysmemaddr = 0x1FFF0000;

	/**
	 * Step: Disable RCC and other peripherals
	 */
	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
	HAL_GPIO_DeInit(LD2_GPIO_Port, LD2_Pin) ;
	HAL_GPIO_DeInit(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin);
	uint8_t isDeInitOk = 0;
	if (	HAL_RCC_DeInit() 										||
			HAL_UART_DeInit(&huart2) 								||
			HAL_TIM_Base_DeInit(&htim6)								||
			HAL_DeInit()
			== HAL_OK)
	{
		isDeInitOk = 1;
	}

	/**
	* Step: Disable systick timer and reset it to default values
	*/
		SysTick->CTRL = 0;
		SysTick->LOAD = 0;
		SysTick->VAL = 0;

	/**
	 * Step: Disable all interrupts
	 */
	__disable_irq();

	/**
	 * Step: Remap system memory to address 0x0000 0000 in address space
	 */
	__HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

	/**
	 * Step: Set jump memory location to system memory
	 */
	uint32_t jumpAddress = *(__IO uint32_t*)(sysmemaddr + 4);
	jumpFunction sysMemBootJump = (jumpFunction)jumpAddress;

	/**
		 * Step: Set main stack pointer.
		 *       This step must be done last otherwise local variables in this function
		 *       don't have proper value since stack pointer is located on different position
		 *
		 *       Set direct address location which specifies stack pointer in SRAM location
		 */
		__set_MSP(*(__IO uint32_t *)sysmemaddr);

		/**
		 * Step: Actually call our function to jump to set location. This will start system memory execution.
		 * If isDeinitOk flag is not set, then call system reset.
		 */
		if(isDeInitOk)
		{
			sysMemBootJump();
		}
		else
		{
			HAL_NVIC_SystemReset();
		}
}

int __io_putchar(int ch)
{
  if (ch == '\n') {
    __io_putchar('\r');
  }

  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);

  return 1;
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}
static int flag = 0;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == USER_BUTTON_Pin)
	{
		/**
		 * DEBUG logs
		 */
		printf("Current remaped memory on address 0x0 is flash memory\n");
		for (int i = 1; i <= 12; i++)
		{
			printf("%08lx\t", *((uint32_t*)(0x0 + 4 * (i-1))));
			if (i % 4 == 0)
				printf("\n");
		}
		printf("\n\nThe same memory on address 0x0800 0000\n");
			for (int i = 1; i <= 12; i++)
			{
				printf("%08lx\t", *((uint32_t*)(0x08000000 + 4 * (i-1))));
				if (i % 4 == 0)
					printf("\n");
			}
		printf("\n\nSystem memory address 0x1FFF 0000\n");
		for (uint32_t i = 1; i <= 12; i++)
		{
			printf("%08lx\t", *((uint32_t*)(0x1FFF0000 + 4 * (i-1))));
			if (i % 4 == 0)
				printf("\n");
		}

		printf("\n\nEntering bootloader...\n");
		for (int i = 0; i < 100000; i++)
		{
			if( i%10000 == 0)
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		}

		/**
		 * Jump to bootloader
		 */
		flag =1;

	}
}

void paintStack()
{
	extern uint32_t _estack;
	static uintptr_t *sp;
	asm volatile ("mrs %0, msp" : "=r"(sp));
	uintptr_t *stackEnd =  ((uintptr_t *)(&_estack));
	uintptr_t* dst = stackEnd - (8192 / sizeof(uintptr_t));
	while (dst < sp)
	{
		*dst = 0xDEADC0DE;
		dst++;
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim6);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (flag == 1)
	  {
		  JumpToBootloader();
	  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
