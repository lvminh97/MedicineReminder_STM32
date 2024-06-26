/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
extern uint8_t cur_screen, need_update_menu;
extern uint8_t is_next_day;
extern int upcoming_schedule_pos;
extern SCHEDULE schedule_list[10];
extern uint8_t content[10][21];
extern RTC_Time c_time;
extern int upcoming_time;
int time_update_cnt = 0;
int medicine_notify = 0, medicine_notify_cnt = 0;
int type_a_cnt = 0, type_b_cnt = 0;
unsigned long last_interrupt_cnt_a = 0, last_interrupt_cnt_b = 0;
int sensor_tmout = 0, sensor_tmout_cnt = 0, sensor_tmout_cnt2 = 0;
int upcoming_delay = 0;
uint8_t rx_data[2];
uint8_t command_buffer[COMMAND_BUFFER_SIZE], command_buffer_cnt;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void time_update();
void stepper_control();
void process_command();
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  	HAL_GPIO_WritePin(TYPEA_GPIO_Port, TYPEA_Pin, GPIO_PIN_RESET);
  	HAL_GPIO_WritePin(TYPEB_GPIO_Port, TYPEB_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);		// delay after initializing I2C
	DS3231_Init(&hi2c1);
#if !RTC_EMUL
	// update for correct RTC year
	if (DS3231_GetYear() != 24) {
		DS3231_SetYear(24);
	}
#endif
	lcd_init();
	lcd_clear_display();
	lcd_gotoxy(1, 1);
	lcd_send_string("Initialize...");

	load_schedule();
	upcoming_time = 999999;

	menu_set_content();
	menu_update();

	HAL_UART_Receive_IT(&huart2, (uint8_t*) rx_data, 1);

	HAL_Delay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
		check_button();
		menu_update();
		time_update();
		stepper_control();
		HAL_Delay(TIME_UNIT);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, BUZZ_Pin|TYPEB_Pin|TYPEA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BT_BK_Pin */
  GPIO_InitStruct.Pin = BT_BK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BT_BK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BUZZ_Pin TYPEB_Pin TYPEA_Pin */
  GPIO_InitStruct.Pin = BUZZ_Pin|TYPEB_Pin|TYPEA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : TYPEA_CNT_INTR_Pin TYPEB_CNT_INTR_Pin */
  GPIO_InitStruct.Pin = TYPEA_CNT_INTR_Pin|TYPEB_CNT_INTR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BT_SL_Pin BT_DN_Pin BT_UP_Pin */
  GPIO_InitStruct.Pin = BT_SL_Pin|BT_DN_Pin|BT_UP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void time_update() {
	time_update_cnt++;
	HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, medicine_notify ? GPIO_PIN_SET : GPIO_PIN_RESET);
	if (time_update_cnt == 1000 / TIME_UNIT) {		// 1 second
		time_update_cnt = 0;
#if RTC_EMUL
		c_time.seconds++;
		if(c_time.seconds == 60) {
			c_time.seconds = 0;
			c_time.minutes++;
			if(c_time.minutes == 60) {
				c_time.minutes = 0;
				c_time.hours++;
				if(c_time.hours == 24) {
					c_time.hours = 0;
				}
			}
		}
#else
		DS3231_GetFullDateTime(&c_time);
#endif
		if(sensor_tmout_cnt != 0) {
			sensor_tmout_cnt--;
			if(sensor_tmout_cnt == 0) {
				if(sensor_tmout != 0) {
					sensor_tmout_cnt2 = SENSOR_TIMEOUT_DISP;
					medicine_notify = 0;
					medicine_notify_cnt = 0;
					type_a_cnt = 0;
					type_b_cnt = 0;
					HAL_GPIO_WritePin(TYPEA_GPIO_Port, TYPEA_Pin, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(TYPEB_GPIO_Port, TYPEB_Pin, GPIO_PIN_RESET);
				}
			}
		}

		if(sensor_tmout_cnt2 != 0) {
			sensor_tmout_cnt2--;
		}

		if(medicine_notify_cnt != 0) {
			medicine_notify_cnt--;
			if(medicine_notify_cnt == 0) {
				medicine_notify = 0;
			}
		}

		if(upcoming_delay != 0) {
			upcoming_delay--;
			if(upcoming_delay == 0) {
				update_upcoming_to_esp(current_schedule);
			}
		}

		if (medicine_notify == 0) {
			if (c_time.hours == upcoming_time / 60 && c_time.minutes == upcoming_time % 60) {
				medicine_notify = 1;
				type_a_cnt = schedule_list[upcoming_schedule_pos].type_a;
				type_b_cnt = schedule_list[upcoming_schedule_pos].type_b;
				// sensor time out
				sensor_tmout = 0x03;	// bit0: type A, bit1: type B
				sensor_tmout_cnt = SENSOR_TIMEOUT;
				sensor_tmout_cnt2 = 0;
				// update the upcoming to ESP
				current_schedule = schedule_list[upcoming_schedule_pos];
				upcoming_delay = 3;
//				update_upcoming_to_esp(current_schedule);
				// remove the schedule from the list
				schedule_remove(upcoming_schedule_pos);
				upcoming_time = 999999;
				upcoming_schedule_pos = -1;
				store_schedule();
				update_schedulelist_to_esp();
			}
		}

		if ((convert_to_minute(c_time.hours, c_time.minutes) - upcoming_time >= 1 && is_next_day == 0)
				|| (upcoming_time == 999999))
		{
			find_upcoming_schedule();
		}

		menu_set_content();
	}
}

void stepper_control() {
	// type a
	if(type_a_cnt > 0) {
		HAL_GPIO_TogglePin(TYPEA_GPIO_Port, TYPEA_Pin);
	}
	// type b
	if(type_b_cnt > 0) {
		HAL_GPIO_TogglePin(TYPEB_GPIO_Port, TYPEB_Pin);
	}
}

// External interrupt callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	unsigned long interrupt_time = HAL_GetTick();

	if(GPIO_Pin == GPIO_PIN_14) {	// CNT TYPE A
		if((interrupt_time - last_interrupt_cnt_a > 200) && type_a_cnt > 0) {
			type_a_cnt--;
			sensor_tmout &= ~0x01;	// clear bit0 for type A
			if(type_a_cnt == 0) {
				HAL_GPIO_WritePin(TYPEA_GPIO_Port, TYPEA_Pin, GPIO_PIN_RESET);
				if(type_b_cnt == 0)
					medicine_notify_cnt = 120;	// 2 minutes
			}
		}
		last_interrupt_cnt_a = interrupt_time;
	}
	else if(GPIO_Pin == GPIO_PIN_15) {	// CNT TYPE B
		if((interrupt_time - last_interrupt_cnt_b > 200) && type_b_cnt > 0) {
			type_b_cnt--;
			sensor_tmout &= ~0x02;	// clear bit1 for type B
			if(type_b_cnt == 0) {
				HAL_GPIO_WritePin(TYPEB_GPIO_Port, TYPEB_Pin, GPIO_PIN_RESET);
				if(type_a_cnt == 0)
					medicine_notify_cnt = 120;	// 2 minutes
			}
		}
		last_interrupt_cnt_b = interrupt_time;
	}
}

// UART interrupt callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == USART2) {
		command_buffer[command_buffer_cnt] = rx_data[0];
		command_buffer_cnt++;
		if(command_buffer_cnt >= COMMAND_BUFFER_SIZE)
			command_buffer_cnt = 0;

		process_command();
		HAL_UART_Receive_IT(&huart2, (uint8_t*) rx_data, 1);
	}
}

void process_command() {
	if(command_buffer_cnt >= 4
			&& command_buffer[command_buffer_cnt - 2] == 0x84
			&& command_buffer[command_buffer_cnt - 1] == 0xF1
	) {
		// find start position
		int start_pos;
		for(start_pos = 0; start_pos < command_buffer_cnt - 2; start_pos++) {
			if(command_buffer[start_pos] == 0x84 && command_buffer[start_pos + 1] == 0xF0)
				break;
		}

		// check command type
		uint8_t* buff = command_buffer + start_pos;
		if(buff[2] == 0x81) {	// set RTC time
#if RTC_EMUL
			c_time.hours = buff[3];
			c_time.minutes = buff[4];
			c_time.seconds = buff[5];
			c_time.day = buff[6];
			c_time.month = buff[7];
			c_time.year = buff[8];
#else
			DS3231_SetFullTime(buff[3], buff[4], buff[5]);
			DS3231_SetDate(buff[6]);
			DS3231_SetMonth(buff[7]);
			DS3231_SetYear(buff[8]);
#endif
		}
		else if(buff[2] == 0x82) {	// set schedule list
			schedule_size = buff[3];
			for(int i = 0; i < schedule_size; i++) {
				schedule_list[i].hour = buff[3 * i + 4];
				schedule_list[i].minute = buff[3 * i + 5];
				schedule_list[i].type_a = buff[3 * i + 6] & 0x0F;
				schedule_list[i].type_b = (buff[3 * i + 6] >> 4) & 0x0F;
			}
			store_schedule();
			find_upcoming_schedule();
//			update_to_esp();	// don't need to update to ESP
		}
		else if(buff[2] == 0x83 && buff[3] == 0x80) {	// WiFi connect successful
			wifi_connect = 1;
			menu_set_content();
		}

		command_buffer_cnt = 0;
	}
}

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
