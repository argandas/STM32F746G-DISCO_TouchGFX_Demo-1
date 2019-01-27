#include <gui/model/Model.hpp>
#include <stdio.h>
#include <stdint.h>
#include <gui/model/ModelListener.hpp>
#include <touchgfx/Utils.hpp>

#ifndef SIMULATOR
#include "cmsis_os.h"
extern osMessageQId buttonQueueHandle;
extern osMessageQId ledQueueHandle;
#endif

Model::Model() : modelListener(0)
{
}

void Model::tick()
{
#ifndef SIMULATOR
  uint8_t buttonStateReceived = 0;
  if (xQueueReceive(&buttonQueueHandle, &buttonStateReceived, 0) == pdTRUE)
  {
    modelListener->m2p_ButtonPressed();
  }
#endif
} 

void Model::p2m_SetLEDState(bool led_state)
{
  touchgfx_printf("Model::p2m_SetLEDState: %d\r\n", led_state);

#ifndef SIMULATOR
  xQueueSend(ledQueueHandle, &led_state, 0);
#endif
}