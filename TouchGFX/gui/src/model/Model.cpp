#include <gui/model/Model.hpp>
#include <stdio.h>
#include <stdint.h>
#include <gui/model/ModelListener.hpp>

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
    model_btnPressed();
  }
#endif

}
        
void Model::model_btnPressed()
{
  modelListener->btnPressed();
}

void Model::model_ledSetState(bool led_state)
{
#ifndef SIMULATOR
  xQueueSend(&ledQueueHandle, &led_state, 0);
#else
  printf("led_state: %d", led_state);
#endif
}