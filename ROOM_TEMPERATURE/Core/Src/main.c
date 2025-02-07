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
#include "eth.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "BMPXX80.h"
#include "string.h"
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
float current_temperature_f = 0.0f;
float set_temp_f = 32.0f;
float pwm_duty_f = 0.0f;
uint16_t pwm_duty_u = 0;
float pressure = 0.0f;
char buffer[100];
int i = 0;
char full_response[100];

#define MAX_OUTPUT  999
#define MIN_OUTPUT  0
#define MAX_I_TERM 0.2f   // Half of the PWM period (htim3.Init.Period)
#define MIN_I_TERM -0.2f  // Negative limit to allow for correction in the other direction


struct PID_Controller{
	float Kp;
	float Ki;
	float Kd;
	float Tp;
	float prev_error;
	float prev_u_I;
};
struct PID_Controller PID1;
float u_P, u_I, u_D;

float calculate_PID(struct PID_Controller *PID, float set_temp, float measured_temp) {
    float u = 0;
    float error;

    error = set_temp - measured_temp;

    // Proportional gain
    u_P = PID->Kp * error;

    // Integral gain
    u_I = PID->Ki * PID->Tp / 2.0 * (error + PID->prev_error) + PID->prev_u_I;

    // Derivative gain
    u_D = PID->Kd * (error - PID->prev_error) / PID->Tp;

    u = u_P + u_I + u_D;

    if (u > MAX_OUTPUT) {
        u = MAX_OUTPUT;
        u_I = PID->prev_u_I;
    } else if (u < MIN_OUTPUT) {
        u = MIN_OUTPUT;
        u_I = PID->prev_u_I;
    } else {
        PID->prev_u_I = u_I;
    }

    PID->prev_error = error;

    return u;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
    	current_temperature_f = BMP280_ReadTemperature();
    	pwm_duty_f = (htim3.Init.Period * calculate_PID(&PID1, set_temp_f, current_temperature_f));
    	if (pwm_duty_f < 0.0 ){
    		pwm_duty_u = 0;
    	}
    	else if (pwm_duty_f > htim3.Init.Period){
    		pwm_duty_u = htim3.Init.Period;
    	}
    	else pwm_duty_u = (uint16_t) pwm_duty_f;
    	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm_duty_u);
        uint32_t timestamp = HAL_GetTick();
        sprintf(full_response, "Timestamp: %lu ms, Set Temperature: %.2fC, Current Temperature: %.2fC, PWM Value: %u\r\n",
                timestamp, set_temp_f, current_temperature_f, pwm_duty_u);
        HAL_UART_Transmit(&huart3, (uint8_t*)full_response, strlen(full_response), 900);
    }
}

void slice(const char* str, char* result, size_t start, size_t end) {
    strncpy(result, str + start, end - start);
}

#define BUFFER_SIZE 10
char received[BUFFER_SIZE];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) {
        // Check and process received command
        if (strncmp(received, "SETTEMP", 7) == 0) {
            char *tempStr = &received[7];
            int tempValue = atoi(tempStr);

            if (tempValue > 0 && tempValue <= 100) {
                set_temp_f = (float)tempValue;
                PID1.prev_u_I = 0.0f;
                char response[50];
                sprintf(response, "New temperature set: %.2fC\r\n", set_temp_f);
                HAL_UART_Transmit(&huart3, (uint8_t *)response, strlen(response), 100);
            } else {
                char response[50];
                sprintf(response, "Invalid temperature value: %s\r\n", tempStr);
                HAL_UART_Transmit(&huart3, (uint8_t *)response, strlen(response), 100);
            }
        } else {
            // Unrecognized command
            char response[50];
            sprintf(response, "Failed to recognize command: %s\r\n", received);
            HAL_UART_Transmit(&huart3, (uint8_t *)response, strlen(response), 100);
        }

        // Clear the buffer for the next command
        memset(received, '\0', BUFFER_SIZE);

        // Re-enable UART interrupt for the next reception
        HAL_UART_Receive_IT(&huart3, (uint8_t *)received, BUFFER_SIZE - 1);
    }
}



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
	PID1.Kp = 0.045102305639845;
	PID1.Ki = 0.000655333866370601;
	PID1.Kd = 0.075898381935333;
	PID1.Tp = 1;
	PID1.prev_error = 0;
	PID1.prev_u_I = 0;
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
  MX_ETH_Init();
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  BMP280_Init(&hi2c1, BMP280_TEMPERATURE_16BIT, BMP280_STANDARD, BMP280_FORCEDMODE);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_UART_Receive_IT(&huart3, (uint8_t *)received, BUFFER_SIZE - 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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
