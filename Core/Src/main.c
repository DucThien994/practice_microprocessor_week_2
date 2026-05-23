/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
unsigned char position[5] = {0x08, 0x10, 0x20, 0x40, 0x80};  //Tuong ung voi led so 1, so 2, so 3, so 4, so 5
unsigned char number[10] = {0xAF, 0xA0, 0x6E, 0xEA, 0xE1, 0xCB, 0xCF, 0xA2, 0xEF, 0xEB};  //0 1 2 3 4 5 6 7 8 9
unsigned char data165 = 0xFF;
unsigned char dot = 0x40;

uint8_t led_values[5] = {0, 0, 0, 0, 0};
int bt0_state = 0; //Button 0 state
int bt5_state = 0; //Button 5 state
int bt6_state = 0; //Button 6 state
int bt7_state = 0; //Button 7 state
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
void Clock_Init(void);
void data_74_decode(unsigned char data);
void displayLED_Position(void);
void displayLED(uint32_t numbers);
void read_74HC165_Data(void);
void delay(uint16_t time);
void PB3sendPulse(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  /* USER CODE BEGIN 2 */
  Clock_Init();
	GPIOC->ODR |= (1 << 9);   // PC9 ON
  GPIOA->ODR &= ~(1 << 8);  // PA8 OFF

	HAL_Delay(100);
  read_74HC165_Data();
  if ((data165 & (1<<0)) == 0) bt0_state = 1;
  if ((data165 & (1<<5)) == 0) bt5_state = 1;
  if ((data165 & (1<<6)) == 0) bt6_state = 1;
  if ((data165 & (1<<7)) == 0) bt7_state = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if ((GPIOE->IDR & (1 << 13)) == 0)
      {
          GPIOA->ODR |= (1 << 8);
          GPIOC->ODR &= ~(1 << 9);
      }
      if ((GPIOE->IDR & (1 << 12)) == 0)
      {
				  led_values[3] = 0;
				  led_values[2] = 0;
				  led_values[1] = 0;
				  led_values[0] = 0;
          GPIOA->ODR &= ~(1 << 8);
          GPIOC->ODR |= (1 << 9);

      }

			read_74HC165_Data();
			//Button 0 - LED 1
			if(((data165 & (1<<0))==0) && (bt0_state == 0))
			{
				led_values[3]++;
				if (led_values[3] > 9)
					{
						led_values[3] = 0;
					}
				bt0_state = 1;
			}
			else if (((data165 & (1<<0))!=0) && (bt0_state == 1))
			{
				bt0_state = 0;
			}
			//Button 5 - LED 2
			if(((data165 & (1<<5))==0) && (bt5_state == 0))
			{
				led_values[2]++;
				if (led_values[2] > 9)
				{
					led_values[2] = 0;
				}
				bt5_state = 1;
			}
			else if (((data165 & (1<<5))!=0) && (bt5_state == 1))
			{
				bt5_state = 0;
			}
			//Button 6
			if(((data165 & (1<<6))==0) && (bt6_state == 0))
			{
				led_values[1]++;
				if (led_values[1] > 9)
				{
					led_values[1] = 0;
				}
				bt6_state = 1;
			}
			else if (((data165 & (1<<6))!=0) && (bt6_state == 1))
			{
				bt6_state = 0;
			}
      //Button 7
			if(((data165 & (1<<7))==0) && (bt7_state == 0))
			{
				led_values[0]++;
				if (led_values[0] > 9)
				{
					led_values[0] = 0;
				}
				bt7_state = 1;
			}
			else if (((data165 & (1<<7))!=0) && (bt7_state == 1))
			{
				bt7_state = 0;
			}

			displayLED_Position();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
void Clock_Init(void)
{
	// Port A (bit 0), B (bit 1), C (bit 2), E (bit 4)
  RCC->AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 4);
	GPIOB->MODER &= ~(3<<6);
	GPIOB->MODER |= (1<<22) | (1<<20) | (1<<10) | (1<<6);  //Enable PB3, PB5, PB10 and PB11 is output
	GPIOB->MODER &= ~(3<<8); //Enable B4 as input
	GPIOB->OSPEEDR = 0; //Low speed
	GPIOB->ODR = 0xFFFF; //Reset all LEDs
	GPIOE->MODER |= (1<<4) | (1<<2) |(1<<0); //Enable PE0, PE1 and PE2
	GPIOE->OSPEEDR = 0; //Low speed
	GPIOE->ODR &= ~(1<<2);

  // PA8
	GPIOA->MODER &= ~(3 << (8 * 2)); // Reset
  GPIOA->MODER |=  (1 << (8 * 2)); // Output Mode
  // PC9
  GPIOC->MODER &= ~(3 << (9 * 2)); // Reset
  GPIOC->MODER |=  (1 << (9 * 2)); // Output Mode

  GPIOE->MODER &= ~(3 << (12 * 2)); // Reset mode PE12 -> Input
  GPIOE->MODER &= ~(3 << (13 * 2)); // Reset mode PE13 -> Input
  GPIOE->PUPDR |= (1 << (12 * 2)); // 01: Pull-up cho PE12
  GPIOE->PUPDR |= (1 << (13 * 2)); // 01: Pull-up cho PE13

}
//Function to make PB3 send a pulse
void PB3sendPulse(void)
{
	GPIOB->ODR |= (1<<3);
	GPIOB->ODR &= ~(1<<3);
}
//Send data to 74HC594 decoder
void data_74_decode(unsigned char data)
{
	//SET Data (DS) mode
	for(int i = 0; i<8; i++) //From 0 to 7
	{
		if ((data & (1<<(7-i))) == 0)
		{
			GPIOB->ODR &= ~(1<<5); //Low
		}
		else
		{
			GPIOB->ODR |= (1<<5);//High
		}
		//Send a Pulse
		PB3sendPulse();
	}
}

void displayLED_Position(void)
{
    for(int i=0; i<5; i++)
    {
        data_74_decode(position[4-i]);
        data_74_decode(number[led_values[i]]);
        GPIOE->ODR |= (1<<0);
        GPIOE->ODR &= ~(1<<0);
        HAL_Delay(2);
    }
}

//void displayLED(uint32_t numbers)
//{
//    uint8_t disp[5];
//    disp[4] = numbers / 10000;
//    disp[3] = numbers % 10000 / 1000;
//    disp[2] = numbers % 1000 / 100;
//    disp[1] = numbers % 100 / 10;
//    disp[0] = numbers % 10;

//    for(int i=0; i<5; i++)
//    {
//        data_74_decode(position[4-i]);
//        data_74_decode(number[disp[i]]);
//        GPIOE->ODR |= (1<<0);
//        GPIOE->ODR &= ~(1<<0);

//        HAL_Delay(2);
//    }
//}

void read_74HC165_Data(void)
{
	GPIOE->ODR &= ~(1<<1);
	GPIOE->ODR |= (1<<1);
	for(int i = 0; i<8; i++)
	{
		if ((GPIOB->IDR & (1<<4))==0)
			data165 &= ~(1<<(7-i));
		else
			data165 |= (1<<(7-i));
		PB3sendPulse();
	}
}
void delay(uint16_t time)
{
   while(time>0) time--;
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
