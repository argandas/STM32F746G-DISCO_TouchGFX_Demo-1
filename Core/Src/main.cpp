/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "lwip.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stm32746g_discovery_qspi.h>
#include <stm32746g_discovery.h>
#include <stm32f7xx_hal_rng.h>
    
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "sprintf.h"

#include "lwip/api.h"

#include "jsmn.h"
#include "lwhttp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Enable/Disable usage for FatFs mkfs function */
// #define USE_FATFS_MKFS 0

#define MAX_DHCP_TRIES  4

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

QSPI_HandleTypeDef hqspi;

RNG_HandleTypeDef hrng;

RTC_HandleTypeDef hrtc;

SD_HandleTypeDef hsd1;
DMA_HandleTypeDef hdma_sdmmc1_rx;
DMA_HandleTypeDef hdma_sdmmc1_tx;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart6;

osThreadId defaultTaskHandle;
osThreadId buttonTaskHandle;
osThreadId ledTaskHandle;
osThreadId sdTaskHandle;
osThreadId lwipTaskHandle;
osThreadId dhcpTaskHandle;
osMessageQId buttonQueueHandle;
osMessageQId ledQueueHandle;
osMessageQId tcpQueueHandle;
osSemaphoreId dhcpBinarySemHandle;
/* USER CODE BEGIN PV */

osThreadId httpClientTaskHandle;
osThreadId httpServerTaskHandle;

FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL MyFile;     /* File object */


FIL HTTP_File;     /* File object */


#if 0
uint8_t workBuffer[2*_MAX_SS];
#endif

char cli_buff[512];

uint8_t DHCP_state = DHCP_OFF;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
extern "C" void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CRC_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C3_Init(void);
static void MX_QUADSPI_Init(void);
static void MX_RTC_Init(void);
static void MX_USART1_UART_Init(void);
extern void GRAPHICS_HW_Init(void);
extern void GRAPHICS_Init(void);
extern void GRAPHICS_MainTask(void);
static void MX_SDMMC1_SD_Init(void);
static void MX_RNG_Init(void);
static void MX_USART6_UART_Init(void);
void StartDefaultTask(void const * argument);
void StartButtonTask(void const * argument);
void StartLEDTask(void const * argument);
void StartSDTask(void const * argument);
void StartLWIPTask(void const * argument);
void StartDHCPTask(void const * argument);

/* USER CODE BEGIN PFP */
void Start_HTTP_Client_Task(void const * argument);
void Start_HTTP_Server_Task(void const * argument);

uint16_t cli_printf(const char* fmt, ...);
uint16_t cli_write(const char* src, uint16_t len);
FRESULT scan_files(char* path);
FRESULT get_free_clusters(FATFS* fs);
static int jsoneq(const char *json, jsmntok_t *tok, const char *s);

#if 0
const uint8_t index_html[] = {
  0x3c, 0x21, 0x44, 0x4f, 0x43, 0x54, 0x59, 0x50, 0x45, 0x20, 0x68, 0x74, 0x6d, 0x6c, 0x3e, 0x3c, 0x68, 0x74, 0x6d, 0x6c, 0x3e, 0x3c, 0x68, 0x65, 0x61, 0x64, 0x3e, 0x3c, 0x6d, 0x65, 0x74, 0x61, 0x20, 0x68, 0x74, 0x74, 0x70, 0x2d, 0x65, 0x71, 0x75, 0x69, 0x76, 0x3d, 0x22, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79, 0x70, 0x65, 0x22, 0x20, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x3d, 0x22, 0x74, 0x65, 0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x3b, 0x20, 0x63, 0x68, 0x61, 0x72, 0x73, 0x65, 0x74, 0x3d, 0x77, 0x69, 0x6e, 0x64, 0x6f, 0x77, 0x73, 0x2d, 0x31, 0x32, 0x35, 0x32, 0x22, 0x3e, 0x3c, 0x62, 0x6f, 0x64, 0x79, 0x3e, 0x3c, 0x73, 0x74, 0x79, 0x6c, 0x65, 0x3e, 0x2a, 0x20, 0x7b, 0x62, 0x6f, 0x78, 0x2d, 0x73, 0x69, 0x7a, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x62, 0x6f, 0x72, 0x64, 0x65, 0x72, 0x2d, 0x62, 0x6f, 0x78, 0x3b, 0x7d, 0x62, 0x6f, 0x64, 0x79, 0x20, 0x7b, 0x20, 0x20, 0x6d, 0x61, 0x72, 0x67, 0x69, 0x6e, 0x3a, 0x20, 0x30, 0x3b, 0x20, 0x20, 0x66, 0x6f, 0x6e, 0x74, 0x2d, 0x66, 0x61, 0x6d, 0x69, 0x6c, 0x79, 0x3a, 0x20, 0x41, 0x72, 0x69, 0x61, 0x6c, 0x2c, 0x20, 0x48, 0x65, 0x6c, 0x76, 0x65, 0x74, 0x69, 0x63, 0x61, 0x2c, 0x20, 0x73, 0x61, 0x6e, 0x73, 0x2d, 0x73, 0x65, 0x72, 0x69, 0x66, 0x3b, 0x7d, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x7b, 0x20, 0x20, 0x6f, 0x76, 0x65, 0x72, 0x66, 0x6c, 0x6f, 0x77, 0x3a, 0x20, 0x68, 0x69, 0x64, 0x64, 0x65, 0x6e, 0x3b, 0x20, 0x20, 0x62, 0x61, 0x63, 0x6b, 0x67, 0x72, 0x6f, 0x75, 0x6e, 0x64, 0x2d, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x20, 0x23, 0x65, 0x39, 0x65, 0x39, 0x65, 0x39, 0x3b, 0x7d, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x61, 0x20, 0x7b, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x3a, 0x20, 0x6c, 0x65, 0x66, 0x74, 0x3b, 0x20, 0x20, 0x64, 0x69, 0x73, 0x70, 0x6c, 0x61, 0x79, 0x3a, 0x20, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x3b, 0x20, 0x20, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x20, 0x62, 0x6c, 0x61, 0x63, 0x6b, 0x3b, 0x20, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2d, 0x61, 0x6c, 0x69, 0x67, 0x6e, 0x3a, 0x20, 0x63, 0x65, 0x6e, 0x74, 0x65, 0x72, 0x3b, 0x20, 0x20, 0x70, 0x61, 0x64, 0x64, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x31, 0x34, 0x70, 0x78, 0x20, 0x31, 0x36, 0x70, 0x78, 0x3b, 0x20, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2d, 0x64, 0x65, 0x63, 0x6f, 0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x3a, 0x20, 0x6e, 0x6f, 0x6e, 0x65, 0x3b, 0x20, 0x20, 0x66, 0x6f, 0x6e, 0x74, 0x2d, 0x73, 0x69, 0x7a, 0x65, 0x3a, 0x20, 0x31, 0x37, 0x70, 0x78, 0x3b, 0x7d, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x61, 0x3a, 0x68, 0x6f, 0x76, 0x65, 0x72, 0x20, 0x7b, 0x20, 0x20, 0x62, 0x61, 0x63, 0x6b, 0x67, 0x72, 0x6f, 0x75, 0x6e, 0x64, 0x2d, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x20, 0x23, 0x64, 0x64, 0x64, 0x3b, 0x20, 0x20, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x20, 0x62, 0x6c, 0x61, 0x63, 0x6b, 0x3b, 0x7d, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x61, 0x2e, 0x61, 0x63, 0x74, 0x69, 0x76, 0x65, 0x20, 0x7b, 0x20, 0x20, 0x62, 0x61, 0x63, 0x6b, 0x67, 0x72, 0x6f, 0x75, 0x6e, 0x64, 0x2d, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x20, 0x23, 0x32, 0x31, 0x39, 0x36, 0x46, 0x33, 0x3b, 0x20, 0x20, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x20, 0x77, 0x68, 0x69, 0x74, 0x65, 0x3b, 0x7d, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x2e, 0x6c, 0x6f, 0x67, 0x69, 0x6e, 0x2d, 0x63, 0x6f, 0x6e, 0x74, 0x61, 0x69, 0x6e, 0x65, 0x72, 0x20, 0x7b, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x3a, 0x20, 0x72, 0x69, 0x67, 0x68, 0x74, 0x3b, 0x7d, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x69, 0x6e, 0x70, 0x75, 0x74, 0x5b, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x74, 0x65, 0x78, 0x74, 0x5d, 0x20, 0x7b, 0x20, 0x20, 0x70, 0x61, 0x64, 0x64, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x36, 0x70, 0x78, 0x3b, 0x20, 0x20, 0x6d, 0x61, 0x72, 0x67, 0x69, 0x6e, 0x2d, 0x74, 0x6f, 0x70, 0x3a, 0x20, 0x38, 0x70, 0x78, 0x3b, 0x20, 0x20, 0x66, 0x6f, 0x6e, 0x74, 0x2d, 0x73, 0x69, 0x7a, 0x65, 0x3a, 0x20, 0x31, 0x37, 0x70, 0x78, 0x3b, 0x20, 0x20, 0x62, 0x6f, 0x72, 0x64, 0x65, 0x72, 0x3a, 0x20, 0x6e, 0x6f, 0x6e, 0x65, 0x3b, 0x20, 0x20, 0x77, 0x69, 0x64, 0x74, 0x68, 0x3a, 0x31, 0x32, 0x30, 0x70, 0x78, 0x3b, 0x7d, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x2e, 0x6c, 0x6f, 0x67, 0x69, 0x6e, 0x2d, 0x63, 0x6f, 0x6e, 0x74, 0x61, 0x69, 0x6e, 0x65, 0x72, 0x20, 0x62, 0x75, 0x74, 0x74, 0x6f, 0x6e, 0x20, 0x7b, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x3a, 0x20, 0x72, 0x69, 0x67, 0x68, 0x74, 0x3b, 0x20, 0x20, 0x70, 0x61, 0x64, 0x64, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x36, 0x70, 0x78, 0x20, 0x31, 0x30, 0x70, 0x78, 0x3b, 0x20, 0x20, 0x6d, 0x61, 0x72, 0x67, 0x69, 0x6e, 0x2d, 0x74, 0x6f, 0x70, 0x3a, 0x20, 0x38, 0x70, 0x78, 0x3b, 0x20, 0x20, 0x6d, 0x61, 0x72, 0x67, 0x69, 0x6e, 0x2d, 0x72, 0x69, 0x67, 0x68, 0x74, 0x3a, 0x20, 0x31, 0x36, 0x70, 0x78, 0x3b, 0x20, 0x20, 0x62, 0x61, 0x63, 0x6b, 0x67, 0x72, 0x6f, 0x75, 0x6e, 0x64, 0x2d, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x20, 0x23, 0x35, 0x35, 0x35, 0x3b, 0x20, 0x20, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x20, 0x77, 0x68, 0x69, 0x74, 0x65, 0x3b, 0x20, 0x20, 0x66, 0x6f, 0x6e, 0x74, 0x2d, 0x73, 0x69, 0x7a, 0x65, 0x3a, 0x20, 0x31, 0x37, 0x70, 0x78, 0x3b, 0x20, 0x20, 0x62, 0x6f, 0x72, 0x64, 0x65, 0x72, 0x3a, 0x20, 0x6e, 0x6f, 0x6e, 0x65, 0x3b, 0x20, 0x20, 0x63, 0x75, 0x72, 0x73, 0x6f, 0x72, 0x3a, 0x20, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x3b, 0x7d, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x2e, 0x6c, 0x6f, 0x67, 0x69, 0x6e, 0x2d, 0x63, 0x6f, 0x6e, 0x74, 0x61, 0x69, 0x6e, 0x65, 0x72, 0x20, 0x62, 0x75, 0x74, 0x74, 0x6f, 0x6e, 0x3a, 0x68, 0x6f, 0x76, 0x65, 0x72, 0x20, 0x7b, 0x20, 0x20, 0x62, 0x61, 0x63, 0x6b, 0x67, 0x72, 0x6f, 0x75, 0x6e, 0x64, 0x2d, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x20, 0x67, 0x72, 0x65, 0x65, 0x6e, 0x3b, 0x7d, 0x40, 0x6d, 0x65, 0x64, 0x69, 0x61, 0x20, 0x73, 0x63, 0x72, 0x65, 0x65, 0x6e, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x28, 0x6d, 0x61, 0x78, 0x2d, 0x77, 0x69, 0x64, 0x74, 0x68, 0x3a, 0x20, 0x36, 0x30, 0x30, 0x70, 0x78, 0x29, 0x20, 0x7b, 0x20, 0x20, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x2e, 0x6c, 0x6f, 0x67, 0x69, 0x6e, 0x2d, 0x63, 0x6f, 0x6e, 0x74, 0x61, 0x69, 0x6e, 0x65, 0x72, 0x20, 0x7b, 0x20, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x3a, 0x20, 0x6e, 0x6f, 0x6e, 0x65, 0x3b, 0x20, 0x20, 0x7d, 0x20, 0x20, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x61, 0x2c, 0x20, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x69, 0x6e, 0x70, 0x75, 0x74, 0x5b, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x74, 0x65, 0x78, 0x74, 0x5d, 0x2c, 0x20, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x2e, 0x6c, 0x6f, 0x67, 0x69, 0x6e, 0x2d, 0x63, 0x6f, 0x6e, 0x74, 0x61, 0x69, 0x6e, 0x65, 0x72, 0x20, 0x62, 0x75, 0x74, 0x74, 0x6f, 0x6e, 0x20, 0x7b, 0x20, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x3a, 0x20, 0x6e, 0x6f, 0x6e, 0x65, 0x3b, 0x20, 0x20, 0x20, 0x20, 0x64, 0x69, 0x73, 0x70, 0x6c, 0x61, 0x79, 0x3a, 0x20, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x3b, 0x20, 0x20, 0x20, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2d, 0x61, 0x6c, 0x69, 0x67, 0x6e, 0x3a, 0x20, 0x6c, 0x65, 0x66, 0x74, 0x3b, 0x20, 0x20, 0x20, 0x20, 0x77, 0x69, 0x64, 0x74, 0x68, 0x3a, 0x20, 0x31, 0x30, 0x30, 0x25, 0x3b, 0x20, 0x20, 0x20, 0x20, 0x6d, 0x61, 0x72, 0x67, 0x69, 0x6e, 0x3a, 0x20, 0x30, 0x3b, 0x20, 0x20, 0x20, 0x20, 0x70, 0x61, 0x64, 0x64, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x31, 0x34, 0x70, 0x78, 0x3b, 0x20, 0x20, 0x7d, 0x20, 0x20, 0x2e, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x20, 0x69, 0x6e, 0x70, 0x75, 0x74, 0x5b, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x74, 0x65, 0x78, 0x74, 0x5d, 0x20, 0x7b, 0x20, 0x20, 0x20, 0x20, 0x62, 0x6f, 0x72, 0x64, 0x65, 0x72, 0x3a, 0x20, 0x31, 0x70, 0x78, 0x20, 0x73, 0x6f, 0x6c, 0x69, 0x64, 0x20, 0x23, 0x63, 0x63, 0x63, 0x3b, 0x20, 0x20, 0x20, 0x20, 0x7d, 0x7d, 0x3c, 0x2f, 0x73, 0x74, 0x79, 0x6c, 0x65, 0x3e, 0x3c, 0x2f, 0x68, 0x65, 0x61, 0x64, 0x3e, 0x3c, 0x64, 0x69, 0x76, 0x20, 0x63, 0x6c, 0x61, 0x73, 0x73, 0x3d, 0x22, 0x74, 0x6f, 0x70, 0x6e, 0x61, 0x76, 0x22, 0x3e, 0x20, 0x20, 0x3c, 0x61, 0x20, 0x63, 0x6c, 0x61, 0x73, 0x73, 0x3d, 0x22, 0x61, 0x63, 0x74, 0x69, 0x76, 0x65, 0x22, 0x20, 0x68, 0x72, 0x65, 0x66, 0x3d, 0x22, 0x23, 0x68, 0x6f, 0x6d, 0x65, 0x22, 0x3e, 0x48, 0x6f, 0x6d, 0x65, 0x3c, 0x2f, 0x61, 0x3e, 0x20, 0x20, 0x3c, 0x61, 0x20, 0x68, 0x72, 0x65, 0x66, 0x3d, 0x22, 0x23, 0x61, 0x62, 0x6f, 0x75, 0x74, 0x22, 0x3e, 0x41, 0x62, 0x6f, 0x75, 0x74, 0x3c, 0x2f, 0x61, 0x3e, 0x20, 0x20, 0x3c, 0x61, 0x20, 0x68, 0x72, 0x65, 0x66, 0x3d, 0x22, 0x23, 0x63, 0x6f, 0x6e, 0x74, 0x61, 0x63, 0x74, 0x22, 0x3e, 0x43, 0x6f, 0x6e, 0x74, 0x61, 0x63, 0x74, 0x3c, 0x2f, 0x61, 0x3e, 0x20, 0x20, 0x3c, 0x64, 0x69, 0x76, 0x20, 0x63, 0x6c, 0x61, 0x73, 0x73, 0x3d, 0x22, 0x6c, 0x6f, 0x67, 0x69, 0x6e, 0x2d, 0x63, 0x6f, 0x6e, 0x74, 0x61, 0x69, 0x6e, 0x65, 0x72, 0x22, 0x3e, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x66, 0x6f, 0x72, 0x6d, 0x20, 0x61, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x3d, 0x22, 0x61, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x5f, 0x70, 0x61, 0x67, 0x65, 0x2e, 0x70, 0x68, 0x70, 0x22, 0x3e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x69, 0x6e, 0x70, 0x75, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x22, 0x74, 0x65, 0x78, 0x74, 0x22, 0x20, 0x70, 0x6c, 0x61, 0x63, 0x65, 0x68, 0x6f, 0x6c, 0x64, 0x65, 0x72, 0x3d, 0x22, 0x55, 0x73, 0x65, 0x72, 0x6e, 0x61, 0x6d, 0x65, 0x22, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x22, 0x75, 0x73, 0x65, 0x72, 0x6e, 0x61, 0x6d, 0x65, 0x22, 0x3e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x69, 0x6e, 0x70, 0x75, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x22, 0x74, 0x65, 0x78, 0x74, 0x22, 0x20, 0x70, 0x6c, 0x61, 0x63, 0x65, 0x68, 0x6f, 0x6c, 0x64, 0x65, 0x72, 0x3d, 0x22, 0x50, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0x22, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x22, 0x70, 0x73, 0x77, 0x22, 0x3e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x62, 0x75, 0x74, 0x74, 0x6f, 0x6e, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x22, 0x73, 0x75, 0x62, 0x6d, 0x69, 0x74, 0x22, 0x3e, 0x4c, 0x6f, 0x67, 0x69, 0x6e, 0x3c, 0x2f, 0x62, 0x75, 0x74, 0x74, 0x6f, 0x6e, 0x3e, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x2f, 0x66, 0x6f, 0x72, 0x6d, 0x3e, 0x20, 0x20, 0x3c, 0x2f, 0x64, 0x69, 0x76, 0x3e, 0x3c, 0x2f, 0x64, 0x69, 0x76, 0x3e, 0x3c, 0x64, 0x69, 0x76, 0x20, 0x73, 0x74, 0x79, 0x6c, 0x65, 0x3d, 0x22, 0x70, 0x61, 0x64, 0x64, 0x69, 0x6e, 0x67, 0x2d, 0x6c, 0x65, 0x66, 0x74, 0x3a, 0x31, 0x36, 0x70, 0x78, 0x22, 0x3e, 0x20, 0x20, 0x3c, 0x68, 0x32, 0x3e, 0x52, 0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x69, 0x76, 0x65, 0x20, 0x4c, 0x6f, 0x67, 0x69, 0x6e, 0x20, 0x46, 0x6f, 0x72, 0x6d, 0x20, 0x69, 0x6e, 0x20, 0x4e, 0x61, 0x76, 0x62, 0x61, 0x72, 0x3c, 0x2f, 0x68, 0x32, 0x3e, 0x20, 0x20, 0x3c, 0x70, 0x3e, 0x4e, 0x61, 0x76, 0x69, 0x67, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x6d, 0x65, 0x6e, 0x75, 0x20, 0x77, 0x69, 0x74, 0x68, 0x20, 0x61, 0x20, 0x6c, 0x6f, 0x67, 0x69, 0x6e, 0x20, 0x66, 0x6f, 0x72, 0x6d, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x61, 0x20, 0x73, 0x75, 0x62, 0x6d, 0x69, 0x74, 0x20, 0x62, 0x75, 0x74, 0x74, 0x6f, 0x6e, 0x20, 0x69, 0x6e, 0x73, 0x69, 0x64, 0x65, 0x20, 0x6f, 0x66, 0x20, 0x69, 0x74, 0x2e, 0x3c, 0x2f, 0x70, 0x3e, 0x20, 0x20, 0x3c, 0x70, 0x3e, 0x52, 0x65, 0x73, 0x69, 0x7a, 0x65, 0x20, 0x74, 0x68, 0x65, 0x20, 0x62, 0x72, 0x6f, 0x77, 0x73, 0x65, 0x72, 0x20, 0x77, 0x69, 0x6e, 0x64, 0x6f, 0x77, 0x20, 0x74, 0x6f, 0x20, 0x73, 0x65, 0x65, 0x20, 0x74, 0x68, 0x65, 0x20, 0x72, 0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x69, 0x76, 0x65, 0x20, 0x65, 0x66, 0x66, 0x65, 0x63, 0x74, 0x2e, 0x3c, 0x2f, 0x70, 0x3e, 0x3c, 0x2f, 0x64, 0x69, 0x76, 0x3e, 0x3c, 0x2f, 0x62, 0x6f, 0x64, 0x79, 0x3e, 0x3c, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x3e
};
#endif 

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern struct netif gnetif;
extern ip4_addr_t ipaddr;
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
  MX_DMA_Init();
  MX_CRC_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_QUADSPI_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_SDMMC1_SD_Init();
  MX_RNG_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

/* Initialise the graphical hardware */
  GRAPHICS_HW_Init();

  /* Initialise the graphical stack engine */
  GRAPHICS_Init();
      
  

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of dhcpBinarySem */
  osSemaphoreDef(dhcpBinarySem);
  dhcpBinarySemHandle = osSemaphoreCreate(osSemaphore(dhcpBinarySem), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityHigh, 0, configMINIMAL_STACK_SIZE * 4);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of buttonTask */
  osThreadDef(buttonTask, StartButtonTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 4);
  buttonTaskHandle = osThreadCreate(osThread(buttonTask), NULL);

  /* definition and creation of ledTask */
  osThreadDef(ledTask, StartLEDTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 4);
  ledTaskHandle = osThreadCreate(osThread(ledTask), NULL);

  /* definition and creation of sdTask */
  osThreadDef(sdTask, StartSDTask, osPriorityAboveNormal, 0, configMINIMAL_STACK_SIZE * 8);
  sdTaskHandle = osThreadCreate(osThread(sdTask), NULL);

  /* definition and creation of lwipTask */
  osThreadDef(lwipTask, StartLWIPTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 4);
  lwipTaskHandle = osThreadCreate(osThread(lwipTask), (void*)&gnetif);

  /* definition and creation of dhcpTask */
  osThreadDef(dhcpTask, StartDHCPTask, osPriorityHigh, 0, configMINIMAL_STACK_SIZE * 2);
  dhcpTaskHandle = osThreadCreate(osThread(dhcpTask), (void*)&gnetif);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  /* definition and creation of httpClientTask */
  osThreadDef(httpClientTask, Start_HTTP_Client_Task, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 8);
  httpClientTaskHandle = osThreadCreate(osThread(httpClientTask), (void*)&gnetif);
  
  /* definition and creation of httpServerTask */
  osThreadDef(httpServerTask, Start_HTTP_Server_Task, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 8);
  httpServerTaskHandle = osThreadCreate(osThread(httpServerTask), (void*)&gnetif);
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of buttonQueue */
  osMessageQDef(buttonQueue, 16, uint8_t);
  buttonQueueHandle = osMessageCreate(osMessageQ(buttonQueue), NULL);

  /* definition and creation of ledQueue */
  osMessageQDef(ledQueue, 16, uint8_t);
  ledQueueHandle = osMessageCreate(osMessageQ(ledQueue), NULL);

  /* definition and creation of tcpQueue */
  osMessageQDef(tcpQueue, 8, uint16_t);
  tcpQueueHandle = osMessageCreate(osMessageQ(tcpQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
 

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /**Configure LSE Drive Capability 
  */
  HAL_PWR_EnableBkUpAccess();
  /**Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /**Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_RTC
                              |RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART6
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_I2C3
                              |RCC_PERIPHCLK_SDMMC1|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV8;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
  PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

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
  hi2c1.Init.Timing = 0x00C0EAFF;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x00C0EAFF;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 1;
  hqspi.Init.FifoThreshold = 4;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = 24;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_6_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

    BSP_QSPI_Init();

    BSP_QSPI_MemoryMappedMode();
    HAL_NVIC_DisableIRQ(QUADSPI_IRQn);

    MPU_Region_InitTypeDef MPU_InitStruct;
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0x90000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_256MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER2;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Configure the MPU attributes as WT for QSPI (used 16Mbytes) */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0x90000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_16MB; /* NOTE! Change this if you change QSPI flash size! */
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER3;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
  /* USER CODE END QUADSPI_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /**Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /**Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /**Enable the Alarm A 
  */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /**Enable the Alarm B 
  */
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_B;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /**Enable the TimeStamp 
  */
  if (HAL_RTCEx_SetTimeStamp(&hrtc, RTC_TIMESTAMPEDGE_RISING, RTC_TIMESTAMPPIN_POS1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 921600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 9600;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, ARDUINO_D7_Pin|ARDUINO_D8_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_Port, LCD_BL_CTRL_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_DISP_GPIO_Port, LCD_DISP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DCMI_PWR_EN_GPIO_Port, DCMI_PWR_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, ARDUINO_D4_Pin|ARDUINO_D2_Pin|EXT_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : OTG_HS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_HS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_HS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ULPI_D7_Pin ULPI_D6_Pin ULPI_D5_Pin ULPI_D3_Pin 
                           ULPI_D2_Pin ULPI_D1_Pin ULPI_D4_Pin */
  GPIO_InitStruct.Pin = ULPI_D7_Pin|ULPI_D6_Pin|ULPI_D5_Pin|ULPI_D3_Pin 
                          |ULPI_D2_Pin|ULPI_D1_Pin|ULPI_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_D3_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(ARDUINO_PWM_D3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPDIF_RX0_Pin */
  GPIO_InitStruct.Pin = SPDIF_RX0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF8_SPDIFRX;
  HAL_GPIO_Init(SPDIF_RX0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_D9_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_D9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(ARDUINO_PWM_D9_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DCMI_D6_Pin DCMI_D7_Pin */
  GPIO_InitStruct.Pin = DCMI_D6_Pin|DCMI_D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_VBUS_Pin */
  GPIO_InitStruct.Pin = OTG_FS_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_VBUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Audio_INT_Pin */
  GPIO_InitStruct.Pin = Audio_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Audio_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OTG_FS_P_Pin OTG_FS_N_Pin OTG_FS_ID_Pin */
  GPIO_InitStruct.Pin = OTG_FS_P_Pin|OTG_FS_N_Pin|OTG_FS_ID_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SAI2_MCLKA_Pin SAI2_SCKA_Pin SAI2_FSA_Pin SAI2_SDA_Pin */
  GPIO_InitStruct.Pin = SAI2_MCLKA_Pin|SAI2_SCKA_Pin|SAI2_FSA_Pin|SAI2_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : SAI2_SDB_Pin */
  GPIO_InitStruct.Pin = SAI2_SDB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
  HAL_GPIO_Init(SAI2_SDB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DCMI_D5_Pin */
  GPIO_InitStruct.Pin = DCMI_D5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(DCMI_D5_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_D7_Pin ARDUINO_D8_Pin LCD_DISP_Pin */
  GPIO_InitStruct.Pin = ARDUINO_D7_Pin|ARDUINO_D8_Pin|LCD_DISP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : uSD_Detect_Pin */
  GPIO_InitStruct.Pin = uSD_Detect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(uSD_Detect_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_BL_CTRL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_CTRL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BL_CTRL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DCMI_VSYNC_Pin */
  GPIO_InitStruct.Pin = DCMI_VSYNC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(DCMI_VSYNC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TP3_Pin NC2_Pin */
  GPIO_InitStruct.Pin = TP3_Pin|NC2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_SCK_D13_Pin */
  GPIO_InitStruct.Pin = ARDUINO_SCK_D13_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(ARDUINO_SCK_D13_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DCMI_PWR_EN_Pin */
  GPIO_InitStruct.Pin = DCMI_PWR_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DCMI_PWR_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DCMI_D4_Pin DCMI_D3_Pin DCMI_D0_Pin DCMI_D2_Pin 
                           DCMI_D1_Pin */
  GPIO_InitStruct.Pin = DCMI_D4_Pin|DCMI_D3_Pin|DCMI_D0_Pin|DCMI_D2_Pin 
                          |DCMI_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_CS_D5_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_CS_D5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
  HAL_GPIO_Init(ARDUINO_PWM_CS_D5_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_D10_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_D10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(ARDUINO_PWM_D10_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_INT_Pin */
  GPIO_InitStruct.Pin = LCD_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LCD_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ULPI_NXT_Pin */
  GPIO_InitStruct.Pin = ULPI_NXT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(ULPI_NXT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_D4_Pin ARDUINO_D2_Pin EXT_RST_Pin */
  GPIO_InitStruct.Pin = ARDUINO_D4_Pin|ARDUINO_D2_Pin|EXT_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_A4_Pin ARDUINO_A5_Pin ARDUINO_A1_Pin ARDUINO_A2_Pin 
                           ARDUINO_A3_Pin */
  GPIO_InitStruct.Pin = ARDUINO_A4_Pin|ARDUINO_A5_Pin|ARDUINO_A1_Pin|ARDUINO_A2_Pin 
                          |ARDUINO_A3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : ULPI_STP_Pin ULPI_DIR_Pin */
  GPIO_InitStruct.Pin = ULPI_STP_Pin|ULPI_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_RXER_Pin */
  GPIO_InitStruct.Pin = RMII_RXER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RMII_RXER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_A0_Pin */
  GPIO_InitStruct.Pin = ARDUINO_A0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ARDUINO_A0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DCMI_HSYNC_Pin PA6 */
  GPIO_InitStruct.Pin = DCMI_HSYNC_Pin|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ULPI_CLK_Pin ULPI_D0_Pin */
  GPIO_InitStruct.Pin = ULPI_CLK_Pin|ULPI_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_D6_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_D6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
  HAL_GPIO_Init(ARDUINO_PWM_D6_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_MISO_D12_Pin ARDUINO_MOSI_PWM_D11_Pin */
  GPIO_InitStruct.Pin = ARDUINO_MISO_D12_Pin|ARDUINO_MOSI_PWM_D11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

uint16_t cli_printf(const char* fmt, ...)
{
  uint16_t len = 0;
  va_list args;
  va_start(args, fmt);
  vsnprintf(cli_buff, sizeof(cli_buff), fmt, args);
  // tfp_vsnprintf(cli_buff, sizeof(cli_buff), fmt, args);
  len = cli_write(&cli_buff[0], strlen(cli_buff));
  va_end(args);    

  return len;
}

void jsmn_parser_example(char* json_string, uint16_t len)
{
  int i;
  int r;
  jsmn_parser p;
  jsmntok_t t[128]; /* We expect no more than 128 tokens */

  jsmn_init(&p);
  r = jsmn_parse(&p, json_string, len, t, sizeof(t)/sizeof(t[0]));
  if (r < 0) {
          cli_printf("Failed to parse JSON: %d\r\n", r);
          //return 1;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    cli_printf("[JSMN] ERROR: Object expected\r\n");
          // return 1;
  }

  /* Loop over all keys of the root object */
  for (i = 1; i < r; i++) {
          if (jsoneq(json_string, &t[i], "channel_id") == 0) {
                  /* We may use strndup() to fetch string value */
                  cli_printf("[JSMN] Channel ID: %.*s\r\n", t[i+1].end-t[i+1].start,
                                  json_string + t[i+1].start);
                  i++;
          } else if (jsoneq(json_string, &t[i], "entry_id") == 0) {
                  /* We may additionally check if the value is either "true" or "false" */
                  cli_printf("[JSMN] Entry ID: %.*s\r\n", t[i+1].end-t[i+1].start,
                                  json_string + t[i+1].start);
                  i++;
          } else if (jsoneq(json_string, &t[i], "field1") == 0) {
                  /* We may want to do strtol() here to get numeric value */
                  cli_printf("[JSMN] Field1: %.*s\r\n", t[i+1].end-t[i+1].start,
                                  json_string + t[i+1].start);
                  i++;
#if 0
          } else if (jsoneq(json_string, &t[i], "groups") == 0) {
                  int j;
                  cli_printf("- Groups:\r\n");
                  if (t[i+1].type != JSMN_ARRAY) {
                          continue; /* We expect groups to be an array of strings */
                  }
                  for (j = 0; j < t[i+1].size; j++) {
                          jsmntok_t *g = &t[i+j+2];
                          cli_printf("  * %.*s\r\n", g->end - g->start, json_string + g->start);
                  }
                  i += t[i+1].size + 1;
#endif
          } else {
                  // cli_printf("Unexpected key: %.*s\r\n", t[i].end-t[i].start, json_string + t[i].start);
          }
  }
}


uint16_t cli_write(const char* src, uint16_t len)
{
  if ((src != NULL) && (len > 0))
  {
    HAL_UART_Transmit(&huart1, (uint8_t*)src, len, 100);
    // HAL_UART_Transmit(&huart6, (uint8_t*)src, len, 100);
  }
  return len;
}

FRESULT get_free_clusters(FATFS* fs)
{
    FRESULT fr;
    DWORD fre_clust, fre_sect, tot_sect;

    /* Get volume information and free clusters of used drive */
    fr = f_getfree((TCHAR const*)SDPath, &fre_clust, &fs);
    if (fr != FR_OK)
    {
      cli_printf("[FatFs] f_getfree: %d\r\n", fr);
    }
    else
    {
      /* Get total sectors and free sectors */
      tot_sect = (fs->n_fatent - 2) * fs->csize * _MAX_SS;
      fre_sect = fre_clust * fs->csize * _MAX_SS;

      /* Print the free space (assuming 512 bytes/sector) */
      cli_printf("[FatFs] Number of Free Clusters:\r\n");
      cli_printf("\t%8lu KB Used\r\n", (tot_sect - fre_sect)/1024);
      cli_printf("\t%8lu KB Available\r\n", fre_sect/1024);
      cli_printf("\t%8lu KB Total\r\n", tot_sect/1024);
    }
    
    return fr;
}

FRESULT read_file_sector(FIL* file, void* dest, FSIZE_t len, UINT* bytesread, UINT index)
{
  FRESULT fr;

  fr = f_open(file, "index.htm", FA_READ);
  if(fr != FR_OK)
  {
    cli_printf("[FatFs] %s --> f_open: %d\r\n", __FUNCTION__, fr);
  }
  else
  {
    fr = f_lseek(file, (index * len));
    if(fr != FR_OK)
    {
      cli_printf("[FatFs] %s --> f_lseek: %d\r\n", __FUNCTION__, fr);
    }
    else
    {
      fr = f_read(file, dest, len, bytesread);
      if(fr != FR_OK)
      {
        cli_printf("[FatFs] %s --> f_read: %d\r\n", __FUNCTION__, fr);
      }
    }
    
    fr = f_close(file);
    if(fr != FR_OK)
    {
        cli_printf("[FatFs] %s --> f_close: %d\r\n", __FUNCTION__, fr);
    }
  }
  
  return fr;
}

/* Start node to be scanned (***also used as work area***) */
FRESULT scan_files(char* path)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    
                /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "\t%s (%4d bytes)", fno.fname, fno.fsize);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            }
            else 
            {                                       
                /* It is a file. */
                cli_printf("\t%s  (%4d bytes)\r\n", fno.fname, fno.fsize);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}


void Start_HTTP_Client_Task(void const * argument)
{
  /* USER CODE BEGIN Start_HTTP_Client_Task */
  
  /* Queue PV */
  uint8_t queueData = 0;
  
  /* LwIP PV */
  struct netif* netif = (struct netif*)argument;
  struct netconn *conn;
  struct netbuf *netbuf = NULL;
  ip4_addr_t local_ip; 
  ip4_addr_t remote_ip; 
  err_t err;
  
  /* Temp Buffer PV */
  char* temp_buf_data = NULL;
  uint16_t temp_buf_data_len = 0;
  lwhttp_message_header_t* temp_header_ptr;
  lwhttp_message_header_t* content_type_header_ptr;
  
  /* JSON PV */
  const char* json_data_fmt = "{\"api_key\": \"%s\", \"delta_t\": %d, \"field1\": %d}";
  uint8_t json_data[128];
  uint8_t json_data_len[4];
  
  /* Thingspeak server PV */
  const char* key = "WTN6FH385TCUNDMU";
  const char* server = "thingspeak.com";
  const char* url = "/update.json?headers=false";
  
  static lwhttp_request_t client_request;
  static lwhttp_response_t client_response;

  osDelay(10);  

  cli_printf("[FreeRTOS] Task %s --> INIT OK\r\n", __FUNCTION__);

  /* Infinite loop */
  for(;;)
  {
    if(xQueueReceive(tcpQueueHandle, &queueData, 0) == pdTRUE)
    {
      if (DHCP_state == DHCP_ADDRESS_ASSIGNED)
      {
        cli_printf("[LwIP] %s --> Send TCP (data = %u)\r\n", __FUNCTION__, queueData);

        /* Parse HTTP Body in JSON format */
        sprintf((char *)json_data, json_data_fmt, key, 5, queueData);  
        sprintf((char *)json_data_len, "%d", strlen((char *)json_data));  
              
        /* LwHTTP Inits */
        lwhttp_request_init(&client_request);
        lwhttp_response_init(&client_response);  
              
        /* Build HTTP Request */
        lwhttp_request_put_request_line(&client_request, LwHHTP_POST, url);
        lwhttp_request_put_message_header(&client_request, "Content-Type", "application/json");
        lwhttp_request_put_message_header(&client_request, "Content-Length", (char *)json_data_len);
        lwhttp_request_put_message_body(&client_request, (char*)json_data, strlen((char *)json_data));  
              
        /* Run LwHTTP Response Parser */
        lwhttp_request_parse(&client_request);
              
        /* LwIP Connect TCP */
        conn = netconn_new(NETCONN_TCP);        
        if (conn == NULL)
        {
          err = ERR_CONN;
          cli_printf("[LwIP] %s --> netconn_new (ERR = %d)\r\n", __FUNCTION__, err);
        }
        else
        {
#if 0
          /* Bind a netconn to a specific local IP address and port (optional) */
          err = netconn_bind(conn, &netif->ip_addr, 60);
          if (err != ERR_OK)
          {
            cli_printf("[LwIP] %s --> netconn_bind (ERR = %d)\r\n", __FUNCTION__, err);
          }
          else
#endif
          {
            err  = netconn_gethostbyname(server, &remote_ip);
            if (err != ERR_OK)
            {
              cli_printf("[LwIP] %s --> netconn_gethostbyname (ERR = %d)\r\n", __FUNCTION__, err);
            }
            else
            {
              cli_printf("[LwIP] Server: %s (%s)\r\n", server, ip4addr_ntoa((const ip4_addr_t *)&remote_ip));
              
              /* Connect to server */
              err = netconn_connect(conn, &remote_ip, 80); 
              if (err != ERR_OK)
              {
                cli_printf("[LwIP] %s --> netconn_connect (ERR = %d)\r\n", __FUNCTION__, err);
              }
              else
              {
                cli_printf("[LwIP] Connection: OK\r\n");

                /* Get LwHTTP Request Data */
                lwhttp_request_get(&client_request, &temp_buf_data, &temp_buf_data_len);
                
                /* Write data to server */
                err = netconn_write(conn, temp_buf_data, temp_buf_data_len, NETCONN_NOFLAG);
                if (err != ERR_OK)
                {
                  cli_printf("[LwIP] %s --> netconn_write (ERR = %d)\r\n", __FUNCTION__, err);
                }
                else
                {
                  /* Get pointer to buffer where response data is stored */
                  while (( err = netconn_recv(conn, &netbuf)) == ERR_OK)
                  {
                      do
                      {
                        /* Get pointer to data and length*/
                        netbuf_data(netbuf, (void**)&temp_buf_data, &temp_buf_data_len);
                        
                        /* LwHTTP write response to parser */
                        lwhttp_response_put(&client_response, (char*)temp_buf_data, temp_buf_data_len);
                      }
                      while (netbuf_next(netbuf) >= 0);

                      /* Free data buffer */
                      netbuf_delete(netbuf);
                  }
                  
                  if ((err != ERR_OK) && (err != ERR_CLSD))
                  {
                      cli_printf("[LwIP] %s --> netconn_recv (ERR = %d)\r\n", __FUNCTION__, err);
                  }
                  else
                  {
                    /* Print HTTP Request */
                    lwhttp_request_get_request_line(&client_request, &temp_buf_data, &temp_buf_data_len);
                    cli_printf("[LwHTTP] Request-Line: %.*s\r\n", temp_buf_data_len, temp_buf_data);

                    /* Parse & Print LwHTTP Response */
                    lwhttp_response_parse(&client_response);
                    lwhttp_response_get_status_line(&client_response, &temp_buf_data, &temp_buf_data_len);
                    cli_printf("[LwHTTP] Status-Line: %.*s\r\n\r\n", temp_buf_data_len, temp_buf_data);

                    /* Content-Type */
                    lwhttp_response_get_message_header(&client_response, "Content-Type", &content_type_header_ptr);
                    
                    if ((client_response.start_line.status_line.status_code.data != NULL) && (content_type_header_ptr != NULL))
                    {
                      /* If HTTP Status Code is OK (200) and Content-Type is JSON */
                      if ((0 == strncmp(client_response.start_line.status_line.status_code.data, "200", strlen("200"))) 
                          && (0 == strncmp(content_type_header_ptr->field_value.data, "application/json", strlen("application/json")))
                          && (1 < client_response.message_body.len))
                      {
                        /* Parse LwHTTP Response message body as JSON */        
                        jsmn_parser_example(client_response.message_body.data, client_response.message_body.len);  
                      }                  
                    }
                  }
                }
              }
            }
          }
          
          netconn_delete(conn);
        }
        
        /* Free LwHTTP Request & Response */
        lwhttp_request_free(&client_request);
        lwhttp_response_free(&client_response);        
      }
      else
      {
        cli_printf("[LwIP] %s --> Unable to send TCP (state = %d, data = %u)\r\n", __FUNCTION__, DHCP_state, queueData);
      }
    }
    else
    {
      osDelay(10);  
    }
  }    
  /* USER CODE END Start_HTTP_Client_Task */
}

void Start_HTTP_Server_Task(void const * argument)
{
  /* USER CODE BEGIN Start_HTTP_Server_Task */
  static lwhttp_request_t server_request;
  static lwhttp_response_t server_response;
  
  static uint8_t server_response_buffer[64];
  
  UINT bytes_read = 0;
  FRESULT fr;
  FILINFO fno;

  /* LwIP PV */
  struct netconn *conn, *newconn;
  struct netbuf *netbuf = NULL;
  err_t err, accept_err;
  
  /* Temp Buffer PV */
  char* temp_buf_data = NULL;
  uint16_t temp_buf_data_len = 0;
  
  const char* hello_world = "Hello World!"; 
  
  uint8_t content_length[6];
  
  /* Wait for DHCP */
  osDelay(250);
  while (DHCP_state != DHCP_ADDRESS_ASSIGNED)
  {
    // cli_printf("[LwIP] %s --> Waiting for DHCP (state = %d)\r\n", __FUNCTION__, DHCP_state);
    osDelay(500);
  }
  cli_printf("[FreeRTOS] Task %s --> INIT OK\r\n", __FUNCTION__);
  
  /* Create a new TCP connection handle */
  conn = netconn_new(NETCONN_TCP);
  if (conn!= NULL)
  {
    /* Bind to port 80 (HTTP) with default IP address */
    err = netconn_bind(conn, NULL, 80);
    
    if (err == ERR_OK)
    {
      /* Put the connection into LISTEN state */
      netconn_listen(conn);
  
      /* Infinite loop */
      for(;;)
      {
        /* accept any icoming connection */
        accept_err = netconn_accept(conn, &newconn);
        if(accept_err == ERR_OK)
        {
          /* LwHTTP Inits */
          lwhttp_request_init(&server_request);
          lwhttp_response_init(&server_response);  

#if 0
          /* Get pointer to buffer where response data is stored */
          for (err = ERR_OK; err == ERR_OK;)
#endif
          {
            /* Note: if this is executed 2 times it blocks the execution */
            err = netconn_recv(newconn, &netbuf);
            if (err != ERR_OK)
            {
              cli_printf("[LwIP] %s --> netconn_recv (ERR = %d)\r\n", __FUNCTION__, err);
            }
            else
            {
              err = netconn_err(newconn);
              if (err != ERR_OK) 
              {
                cli_printf("[LwIP] %s --> netconn_err (ERR = %d)\r\n", __FUNCTION__, err);
              }
              else
              {
                do
                {                  
                  /* Get pointer to data and length*/
                  err = netbuf_data(netbuf, (void**)&temp_buf_data, &temp_buf_data_len);
                  if (err != ERR_OK) 
                  {
                    cli_printf("[LwIP] %s --> netconn_err (ERR = %d)\r\n", __FUNCTION__, err);
                  }
                  else
                  {
                    /* LwHTTP write request to parser */
                    lwhttp_request_put(&server_request, (char*)temp_buf_data, temp_buf_data_len);
                  }                  
                }
                while (netbuf_next(netbuf) >= 0);
                
                /* Free data buffer */
                netbuf_delete(netbuf);       
              }
            }
          }

          /* Parse HTTP Request */
          lwhttp_request_parse(&server_request);
          
          /* If HTTP Status Code is OK (200) and Content-Type is JSON */
          if ((0 == strncmp(server_request.start_line.request_line.method.data, "GET", strlen("GET"))) 
              && (0 == strncmp(server_request.start_line.request_line.request_uri.data, "/", strlen("/")))
              && (server_request.start_line.request_line.request_uri.len == strlen("/")))
          {
            /* Get content length */
            sprintf((char *)content_length, "%d", strlen((char *)hello_world));  

            /* Build HTTP Response */
            lwhttp_response_put_status_line(&server_response, "200", "OK");
            lwhttp_response_put_message_header(&server_response, "Content-Type", "text/html");
            lwhttp_response_put_message_header(&server_response, "Content-Length", (char*)content_length);
            lwhttp_response_put_message_body(&server_response, hello_world, strlen(hello_world));     
            
            /* Send Response */
            lwhttp_response_get(&server_response, &temp_buf_data, &temp_buf_data_len);
            netconn_write(newconn, temp_buf_data, temp_buf_data_len, NETCONN_COPY);   
          }
          else if ((0 == strncmp(server_request.start_line.request_line.method.data, "GET", strlen("GET"))) 
              && (0 == strncmp(server_request.start_line.request_line.request_uri.data, "/index.html", strlen("/index.html")))
              && (server_request.start_line.request_line.request_uri.len == strlen("/index.html")))
          {
            /* Get content length */
            f_stat("index.htm", &fno);
            sprintf((char *)content_length, "%d", fno.fsize);  

            /* Build HTTP Response */
            lwhttp_response_put_status_line(&server_response, "200", "OK");
            lwhttp_response_put_message_header(&server_response, "Content-Type", "text/html");
            lwhttp_response_put_message_header(&server_response, "Content-Length", (char*)content_length);
            lwhttp_response_put_eol(&server_response);     
            
            /* Send Response Start-Line + Message-Headers + <CR><LF> */
            lwhttp_response_get(&server_response, &temp_buf_data, &temp_buf_data_len);
            netconn_write(newconn, temp_buf_data, temp_buf_data_len, NETCONN_COPY);
            
            /* Send Response Body-Message */
            bytes_read = sizeof(server_response_buffer);
            for(UINT idx = 0; bytes_read >= sizeof(server_response_buffer);idx++)
            {
              fr = read_file_sector(&HTTP_File, (void*)server_response_buffer, sizeof(server_response_buffer), &bytes_read, idx);
              if (FR_OK != fr)
              {
                cli_printf("[FatFs] read_file_sector: %d\r\n", fr);
                bytes_read = 0;
              }
              else
              {
                netconn_write(newconn, &server_response_buffer[0], bytes_read, NETCONN_COPY);
              }
            }
          }
          else
          {
            /* Build HTTP Response */
            lwhttp_response_put_status_line(&server_response, "404", "NOT FOUND");
            lwhttp_response_put_message_header(&server_response, "Content-Type", "text/html");
            lwhttp_response_put_message_header(&server_response, "Content-Length", "0");
            lwhttp_response_put_message_body(&server_response, NULL, 0);             
            
            /* Send Response */
            lwhttp_response_get(&server_response, &temp_buf_data, &temp_buf_data_len);
            netconn_write(newconn, temp_buf_data, temp_buf_data_len, NETCONN_COPY);   
          }
          
          /* Close & Delete connection */
          netconn_close(newconn);
          netconn_delete(newconn);
        
          /* Print HTTP Request */
          lwhttp_request_get_request_line(&server_request, &temp_buf_data, &temp_buf_data_len);
          cli_printf("[LwHTTP] Request-Line: %.*s\r\n", temp_buf_data_len, temp_buf_data);
          
          /* Print HTTP Response */
          lwhttp_response_parse(&server_response);
          lwhttp_response_get_status_line(&server_response, &temp_buf_data, &temp_buf_data_len);
          cli_printf("[LwHTTP] Status-Line: %.*s\r\n\r\n", temp_buf_data_len, temp_buf_data);
            
          /* Free LwHTTP Request & Response */
          lwhttp_request_free(&server_request);
          lwhttp_response_free(&server_response);
        }
        
        osDelay(10);
      }
    }
  }

  /* USER CODE END Start_HTTP_Server_Task */
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  
  cli_printf("[FreeRTOS] Task %s --> INIT OK\r\n", __FUNCTION__);
  
  /* Graphic application */  
  GRAPHICS_MainTask();
        
  /* USER CODE BEGIN 5 */
    /* Infinite loop */
    for (;;)
    {
        osDelay(1);
    }
  /* USER CODE END 5 */ 
}

/* USER CODE BEGIN Header_StartButtonTask */
/**
* @brief Function implementing the buttonTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartButtonTask */
void StartButtonTask(void const * argument)
{
  /* USER CODE BEGIN StartButtonTask */
  
  cli_printf("[FreeRTOS] Task %s --> INIT OK\r\n", __FUNCTION__);

  uint32_t buttonState = 0;
  
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
  
  /* Infinite loop */
  for(;;)
  {
    buttonState = BSP_PB_GetState(BUTTON_KEY);
    if (buttonState == 1)
    {
      if(xQueueSend(buttonQueueHandle, (uint8_t*)&buttonState, 0) == pdTRUE)
      {
        cli_printf("[Queue] HW -> Model (%d)\r\n", buttonState);
      }
    }
    
    osDelay(100);
  }
  /* USER CODE END StartButtonTask */
}

/* USER CODE BEGIN Header_StartLEDTask */
/**
* @brief Function implementing the ledTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLEDTask */
void StartLEDTask(void const * argument)
{
  /* USER CODE BEGIN StartLEDTask */
  
  cli_printf("[FreeRTOS] Task %s --> INIT OK\r\n", __FUNCTION__);
  
  uint8_t led_state = 0;

  BSP_LED_Init(LED_GREEN);
  BSP_LED_Off(LED_GREEN);
      
  /* Infinite loop */
  for(;;)
  {
    if(xQueueReceive(ledQueueHandle, &led_state, 0) == pdTRUE)
    {
      if (led_state == 1)
      {
        BSP_LED_On(LED_GREEN);
      }
      else
      {
        BSP_LED_Off(LED_GREEN);
      }
      
      cli_printf("[Queue] Model -> HW (led = %d)\r\n", led_state);
    }
    
    osDelay(20);
  }
  /* USER CODE END StartLEDTask */
}

/* USER CODE BEGIN Header_StartSDTask */
/**
* @brief Function implementing the sdTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSDTask */
void StartSDTask(void const * argument)
{
  
  cli_printf("[FreeRTOS] Task %s --> INIT OK\r\n", __FUNCTION__);

  /* USER CODE BEGIN StartSDTask */
  
#if 1
  FRESULT fr;                                               /* FatFs function common result code */
  uint32_t byteswritten, bytesread = 0;                     /* File write/read counts */
  uint8_t wtext[] = "This is STM32 working with FatFs\r\n"; /* File write buffer */
  uint8_t rtext[100];                                       /* File read buffer */
  uint32_t rng_value = 0;
  uint8_t file_name_buff[16];
  
  /* init code for FATFS */
  MX_FATFS_Init();  

  /* Start filename */
  strcpy((char*)file_name_buff, "filen.TXT");
    
  cli_printf("\r\n[FatFs] Thread START\r\n");
  /*##-1- Link the micro SD disk I/O driver ##################################*/
  // if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
  {  
    /*##-2- Register the file system object to the FatFs module ##############*/
    fr = f_mount(&SDFatFs, (TCHAR const*)SDPath, 0);
    if(fr != FR_OK)
    {
      cli_printf("[FatFs] f_mount: %d\r\n", fr);
    }
    else
    {
#if 0      
      /*##-3- Create a FAT file system (format) on the logical drive #########*/
      /* WARNING: Formatting the uSD card will delete all content on the device */
      fr = f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, workBuffer, sizeof(workBuffer));
      if(fr != FR_OK)
      {
        cli_printf("[FatFs] f_mkfs: %d\r\n", fr);
      }
      else
#endif
      {
        HAL_RNG_GenerateRandomNumber(&hrng, &rng_value);
        file_name_buff[4] = '0' + (uint8_t)(rng_value % 10);
        
        /* 10 - 99 files
        HAL_RNG_GenerateRandomNumber(&hrng, &rng_value);
        file_name_buff[5] = '0' + (uint8_t)(rng_value % 10);
        */
        
        cli_printf("[FatFs] File name: %s\r\n", file_name_buff);

        /*##-4- Create and Open a new text file object with write access #####*/
        fr = f_open(&MyFile, (char*)file_name_buff, FA_OPEN_ALWAYS | FA_WRITE);
        if(fr != FR_OK)
        {
          cli_printf("[FatFs] f_open: %d\r\n", fr);
        }
        else
        {
#if 1
          /*##-5- Prepare to append data to the text file ################################*/
          fr = f_lseek(&MyFile, f_size(&MyFile));
          if((byteswritten == 0) || (fr != FR_OK))
          {
            /* 'STM32.TXT' file Write or EOF Error */
            cli_printf("[FatFs] f_lseek: %d\r\n", fr);
            cli_printf("[FatFs] byteswritten: %d\r\n", byteswritten);
          }
          else
#endif
          {
            /*##-5- Write data to the text file ################################*/
            fr = f_write(&MyFile, wtext, sizeof(wtext), (UINT *)&byteswritten);
            if((byteswritten == 0) || (fr != FR_OK))
            {
              cli_printf("[FatFs] f_write: %d\r\n", fr);
              cli_printf("[FatFs] byteswritten: %d\r\n", byteswritten);
            }
#if 0
            else
            {
              /*##-6- Close the open text file #################################*/
              fr = f_close(&MyFile);
              if(fr != FR_OK)
              {
                cli_printf("[FatFs] f_close: %d\r\n", fr);
              }
              
              /*##-7- Open the text file object with read access ###############*/
              fr = f_open(&MyFile, (char*)file_name_buff, FA_READ);
              if(fr != FR_OK)
              {
                cli_printf("[FatFs] f_open: %d\r\n", fr);
              }
              else
              {
                /*##-8- Read data from the text file ###########################*/
                fr = f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);
                if((bytesread == 0) || (fr != FR_OK))
                {
                  cli_printf("[FatFs] f_read: %d\r\n", fr);
                }
                else
                { 
                  /*##-10- Compare read data with the expected data ############*/
                  if ((bytesread != byteswritten))
                  {                
                    cli_printf("[FatFs] bytesread: %d\r\n", bytesread);
                    cli_printf("[FatFs] byteswritten: %d\r\n", byteswritten);
                  }
                  else
                  {
                    cli_printf("[FatFs] SUCCESS!\r\n");
                  }
                }
              }
            }
#endif
          }
          
          /*##-6- Close the open text file #################################*/
          fr = f_close(&MyFile);
          if(fr != FR_OK)
          {
            cli_printf("[FatFs] f_close: %d\r\n", fr);
          }
        }
        
#if 0
        /*##-4- Create and Open a new text file object with write access #####*/
        cli_printf("[FatFs] File name: %s\r\n", "index.html");
        fr = f_open(&MyFile, "index.htm", FA_CREATE_ALWAYS | FA_WRITE);
        if(fr != FR_OK)
        {
          cli_printf("[FatFs] f_open: %d\r\n", fr);
        }
        else
        {
          /*##-5- Write data to the text file ################################*/
          fr = f_write(&MyFile, index_html, sizeof(index_html), (UINT *)&byteswritten);
          if((byteswritten == 0) || (fr != FR_OK))
          {
            cli_printf("[FatFs] f_write: %d\r\n", fr);
            cli_printf("[FatFs] byteswritten: %d\r\n", byteswritten);
          }
          else
          {
            cli_printf("[FatFs] Bytes written: %d\r\n", byteswritten);
          }
          
          /*##-6- Close the open text file #################################*/
          fr = f_close(&MyFile);
          if(fr != FR_OK)
          {
            cli_printf("[FatFs] f_close: %d\r\n", fr);
          }
        }
#endif
      }
      
      /*##-11 - Scan files in the device ###############################*/
      strcpy((char*)rtext, "/");
      rtext[2] = 0x00;
      cli_printf("[FatFs] Files scan:\r\n");
      fr = scan_files((char*)rtext);
      if(fr != FR_OK)
      {
        cli_printf("[FatFs] scan_files: %d\r\n", fr);
      }
    }
  }
  
  /*##-12 - Scan files in the device ###############################*/
  fr = get_free_clusters(&SDFatFs);
  if(fr != FR_OK)
  {
    cli_printf("[FatFs] get_free_clusters: %d\r\n", fr);
  }

  /*##-13- Unlink the micro SD disk I/O driver ###############################*/
  cli_printf("[FatFs] FINISHED\r\n\r\n");
  
#endif
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }

  FATFS_UnLinkDriver(SDPath);

  /* USER CODE END StartSDTask */
}

/* USER CODE BEGIN Header_StartLWIPTask */
/**
* @brief Function implementing the lwipTask thread.
* @param argument: Not used
* @retval None
*/
portCHAR PAGE_BODY[256];

/* USER CODE END Header_StartLWIPTask */
void StartLWIPTask(void const * argument)
{
  
  cli_printf("[FreeRTOS] Task %s --> INIT OK\r\n", __FUNCTION__);

  /* USER CODE BEGIN StartLWIPTask */

  /* LwIP PV */
  struct netif* netif = (struct netif*)argument;
  
  /* init code for LWIP */
  MX_LWIP_Init();
  
  /* Check interface status */
  if (netif_is_up(netif))
  {
    DHCP_state = DHCP_START;
  }
  else
  {  
    DHCP_state = DHCP_LINK_DOWN;
    cli_printf("[LWIP] %s --> DHCP Link down (state = %d)\r\n", __FUNCTION__, DHCP_state);
  }

  /* Infinite loop */
  for(;;)
  {
    osDelay(10);
  }
  /* USER CODE END StartLWIPTask */
}

/* USER CODE BEGIN Header_StartDHCPTask */
/**
* @brief Function implementing the dhcpTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDHCPTask */
void StartDHCPTask(void const * argument)
{
  
  cli_printf("[FreeRTOS] Task %s --> INIT OK\r\n", __FUNCTION__);

  /* USER CODE BEGIN StartDHCPTask */

  struct netif* netif = (struct netif*)argument;
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
  struct dhcp *dhcp;
  
  /* Get DHCP instance */
  dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
  
  for (;;)
  {
    switch (DHCP_state)
    {
    case DHCP_START:
      {
        ip_addr_set_zero_ip4(&netif->ip_addr);
        ip_addr_set_zero_ip4(&netif->netmask);
        ip_addr_set_zero_ip4(&netif->gw);       
        dhcp_start(netif);
        DHCP_state = DHCP_WAIT_ADDRESS;
        cli_printf("[DHCP] Looking for server ...\r\n");
      }
      break;
      
    case DHCP_WAIT_ADDRESS:
      {                
        if (dhcp_supplied_address(netif)) 
        {
          DHCP_state = DHCP_ADDRESS_ASSIGNED;	
          cli_printf("[DHCP] Supplied address: %s\r\n", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));
        }
        else
        {
          dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
    
          /* DHCP timeout */
          if (dhcp->tries > MAX_DHCP_TRIES)
          {
            DHCP_state = DHCP_TIMEOUT;
            
            /* Stop DHCP */
            dhcp_stop(netif);
            
            /* Static address used */
            IP_ADDR4(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
            IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
            IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
            netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));
            
            cli_printf("[DHCP] Timeout\r\n");
            cli_printf("[DHCP] Static IP address: %s\r\n", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));
          }
        }
      }
      break;
    case DHCP_LINK_DOWN:
      {
        /* Stop DHCP */
        dhcp_stop(netif);
        DHCP_state = DHCP_OFF; 
      }
      break;
    default: 
      break;
    }
    
    /* wait 250 ms */
    osDelay(250);
  }
  
#if 0  
  /* Infinite loop */
  for(;;)
  {
    memset(PAGE_BODY, 0,256);
    /* The list of tasks and their status */
    osThreadList((unsigned char *)PAGE_BODY);

    cli_printf("\r\nName          State  Priority  Stack   Num\r\n" );
    cli_printf("---------------------------------------------\r\n");
    cli_write((char*)PAGE_BODY, strlen((char*)PAGE_BODY));
    cli_printf("---------------------------------------------\r\n");
    cli_printf("B : Blocked, R : Ready, D : Deleted, S : Suspended\r\n");
    osDelay(1000);
  }
#endif

  /* USER CODE END StartDHCPTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */

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
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
