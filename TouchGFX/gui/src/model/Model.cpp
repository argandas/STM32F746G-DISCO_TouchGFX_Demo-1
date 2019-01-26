#include <gui/model/Model.hpp>
#include <stdint.h>
#include <gui/model/ModelListener.hpp>
#include "cmsis_os.h"

extern osMessageQId buttonQueueHandle;

Model::Model() : modelListener(0)
{
}

void Model::tick()
{
  uint8_t buttonStateReceived = 0;
  if (xQueueReceive(&buttonQueueHandle, &buttonStateReceived, 0) == pdTRUE)
  {
    model_btnPressed();
  }
}
        
void Model::model_btnPressed()
{
  modelListener->btnPressed();
}
