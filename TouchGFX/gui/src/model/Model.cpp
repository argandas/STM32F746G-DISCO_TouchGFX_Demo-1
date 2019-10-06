#include <gui/model/Model.hpp>
#include <stdio.h>
#include <stdint.h>
#include <gui/model/ModelListener.hpp>
#include <touchgfx/Utils.hpp>

#ifndef SIMULATOR
#include "cmsis_os.h"
extern osMessageQId buttonQueueHandle;
extern osMessageQId ledQueueHandle;
extern osMessageQId tcpQueueHandle;
extern osMessageQId logQueueHandle;
extern osMessageQId dumpQueueHandle;
#endif

Model::Model() : modelListener(0)
{
}

void Model::tick()
{
#ifndef SIMULATOR
  uint8_t buttonStateReceived = 0;
  if (xQueueReceive(buttonQueueHandle, &buttonStateReceived, 0) == pdTRUE)
  {
    modelListener->m2p_ButtonPressed();
  }
#endif
} 

void Model::p2m_SetLEDState(bool led_state)
{
#ifdef SIMULATOR
  touchgfx_printf("Model::%s: %d\r\n\r\n", __FUNCTION__, led_state);
#else
  xQueueSend(ledQueueHandle, &led_state, 0);
#endif
}

void Model::p2m_SendTCPData(int data)
{
  uint8_t u8Data = 0;
  u8Data = (uint8_t)data;
#ifdef SIMULATOR
  touchgfx_printf("Model::%s: %u\r\n\r\n", __FUNCTION__, u8Data);
#else
  xQueueSend(tcpQueueHandle, &u8Data, 0);
#endif
}

void Model::p2m_LogData(int data)
{
  uint8_t u8Data = 0;
  u8Data = (uint8_t)data;
#ifdef SIMULATOR
  touchgfx_printf("Model::%s: %u\r\n\r\n", __FUNCTION__, u8Data);
#else
  xQueueSend(logQueueHandle, &u8Data, 0);
#endif
}

void Model::p2m_DumpData(void)
{
  uint8_t u8Data = 0;
#ifdef SIMULATOR
  touchgfx_printf("Model::%s: %u\r\n\r\n", __FUNCTION__, u8Data);
#else
  xQueueSend(dumpQueueHandle, &u8Data, 0);
#endif
}