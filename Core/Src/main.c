#include <stdio.h>
#include "main.h"

unsigned char position[5] = {0x08, 0x10, 0x20, 0x40, 0x80};  // Tương ứng với led số 1, số 2, số 3, số 4, số 5
unsigned char number[11] = {0xAF, 0xA0, 0x6E, 0xEA, 0xE1, 0xCB, 0xCF, 0xA2, 0xEF, 0xEB, 0x00};  // 0 1 2 3 4 5 6 7 8 9 tắt
unsigned char data165 = 0xFF;

uint8_t led_values[5] = {0, 0, 0, 0, 0};
uint8_t btn1_last_state = 1; // Trạng thái trước đó của nút 1 (1: Nhả, 0: Nhấn)
uint8_t btn2_last_state = 1; // Trạng thái trước đó của nút 2 (1: Nhả, 0: Nhấn)
uint8_t btn3_last_state = 1;
volatile uint8_t count_mode = 0;     // 0: Dừng đếm, 1: Tự đếm LÊN, 2: Tự đếm XUỐNG
volatile uint8_t counter_2digits = 0; // Biến tổng quản lý giá trị đếm từ 00 đến 99
volatile uint8_t display_enabled = 1; // 1: Đang hiển thị, 0: Tắt màn hình

uint8_t seconds = 0;
uint8_t minutes = 0;
uint32_t previous_tick = 0;

void Clock_Init(void)
{
  RCC->AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 4);
  GPIOB->MODER &= ~(3<<6);
  GPIOB->MODER |= (1<<22) | (1<<20) | (1<<10) | (1<<6);  // Enable PB3, PB5, PB10 và PB11 là output
  GPIOB->MODER &= ~(3<<8); // Enable B4 là input
  GPIOB->OSPEEDR = 0;
  GPIOB->ODR = 0xFFFF;
  GPIOE->MODER |= (1<<4) | (1<<2) |(1<<0); // Enable PE0, PE1 và PE2
  GPIOE->OSPEEDR = 0;
  GPIOE->ODR &= ~(1<<2);

  // PA8
  GPIOA->MODER &= ~(3 << (8 * 2));
  GPIOA->MODER |=  (1 << (8 * 2));
  // PC9
  GPIOC->MODER &= ~(3 << (9 * 2));
  GPIOC->MODER |=  (1 << (9 * 2));

  GPIOE->MODER &= ~(3 << (12 * 2)); // PE12 -> Input
  GPIOE->MODER &= ~(3 << (13 * 2)); // PE13 -> Input
  GPIOE->PUPDR |= (1 << (12 * 2)); // Pull-up cho PE12
  GPIOE->PUPDR |= (1 << (13 * 2)); // Pull-up cho PE13
}

void Timer2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 16000 - 1;
    TIM2->ARR = 1000 - 1;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;
    NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler(void)
{

  if (TIM2->SR & TIM_SR_UIF)
  {
    TIM2->SR &= ~TIM_SR_UIF;
    if (count_mode == 1)
    {
      counter_2digits++;
      if (counter_2digits > 99)
      {
        counter_2digits = 0;
      }
    }
    else if (count_mode == 2)
    {
      if (counter_2digits == 0)
      {
        counter_2digits = 99;
      }
      else
      {
        counter_2digits--;
      }
    }
  }
}

void PB3sendPulse(void)
{
  GPIOB->ODR |= (1<<3);
  GPIOB->ODR &= ~(1<<3);
}

void data_74_decode(unsigned char data)
{
  for(int i = 0; i<8; i++)
  {
    if ((data & (1<<(7-i))) == 0)
    {
      GPIOB->ODR &= ~(1<<5);
    }
    else
    {
      GPIOB->ODR |= (1<<5);
    }
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

void displayLED(uint32_t numbers)
{
    uint8_t disp[5];
    disp[4] = numbers / 10000;
    disp[3] = (numbers % 10000) / 1000;
    disp[2] = (numbers % 1000) / 100;
    disp[1] = (numbers % 100) / 10;
    disp[0] = numbers % 10;

    for(int i=0; i<5; i++)
    {
        data_74_decode(position[4-i]);
        data_74_decode(number[disp[i]]);
        GPIOE->ODR |= (1<<0);
        GPIOE->ODR &= ~(1<<0);
        HAL_Delay(2);
    }
}

void display_MM_SS(uint8_t min, uint8_t sec)
{
    uint8_t disp[5];
    disp[4] = min / 10;
    disp[3] = min % 10;
    disp[2] = 10;
    disp[1] = sec / 10;
    disp[0] = sec % 10;

    for(int i=0; i<5; i++)
    {
        data_74_decode(position[4-i]);
        if (disp[i] == 10) {
            data_74_decode(0x40);
        } else {
            data_74_decode(number[disp[i]]);
        }
        GPIOE->ODR |= (1<<0);
        GPIOE->ODR &= ~(1<<0);
        HAL_Delay(2);
    }
}

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

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  Clock_Init();

  // Đèn tác động mức thấp (Active Low)
  GPIOC->ODR &= ~(1 << 9);  // PC9 ON
  GPIOA->ODR &= ~(1 << 8);  // PA8 ON

  HAL_Delay(100);
  previous_tick = HAL_GetTick();
  Timer2_Init();

  while (1)
  {
	  // lấy giá trị 165 liên tục
      read_74HC165_Data();

      // Yêu cầu ấn nút 1 đếm lên từ 00 tới 99
      // Yêu cầu ấn nút 2 đếm xuống từ 99-00
      // nút 3 để bật tắt led
      // nút 1
      uint8_t current_btn1 = (data165 >> 0) & 1;
      if (current_btn1 == 0 && btn1_last_state == 1)
      {
          HAL_Delay(20);
          read_74HC165_Data();
          if ((data165 & (1 << 0)) == 0)
          {
              if (count_mode == 1) count_mode = 0;
              else count_mode = 1;
          }
      }
      btn1_last_state = current_btn1; // Cập nhật trạng thái nút

      // nút 2
      uint8_t current_btn2 = (data165 >> 5) & 1;
      if (current_btn2 == 0 && btn2_last_state == 1)
      {
          HAL_Delay(20);
          read_74HC165_Data();
          if ((data165 & (1 << 5)) == 0)
          {
              if (count_mode == 2) count_mode = 0;
              else count_mode = 2;
          }
      }
      btn2_last_state = current_btn2; // Cập nhật trạng thái nút

      // cập nhật giá trị
      led_values[1] = counter_2digits / 10; // Hàng chục lên LED 1
      led_values[0] = counter_2digits % 10; // Hàng đơn vị lên LED 2

      if (HAL_GetTick() - previous_tick >= 1000)
      {
          previous_tick = HAL_GetTick();
          seconds++;
          if (seconds >= 60)
          {
              seconds = 0;
              minutes++;
              if (minutes >= 60) {
                  minutes = 0;
              }
          }
      }

      // nút 3
      uint8_t current_btn3 = (data165 >> 6) & 1;
      if (current_btn3 == 0 && btn3_last_state == 1)
      {
        HAL_Delay(20);
        read_74HC165_Data();
        if (((data165 >> 6) & 1) == 0) {
          display_enabled = !display_enabled; // Đảo trạng thái (Bật <-> Tắt)
        }
      }
      btn3_last_state = current_btn3; // Cập nhật trạng thái nút 3

      displayLED_Position();
      //displayLED(12345);
      //display_MM_SS(minutes, seconds);

  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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

void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
