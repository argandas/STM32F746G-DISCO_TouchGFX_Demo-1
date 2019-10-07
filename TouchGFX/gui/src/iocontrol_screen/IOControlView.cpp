#include <gui/iocontrol_screen/IOControlView.hpp>

IOControlView::IOControlView()
{
	led_state = 0;
}

void IOControlView::setupScreen()
{
    IOControlViewBase::setupScreen();
}

void IOControlView::tearDownScreen()
{
    IOControlViewBase::tearDownScreen();
}

void IOControlView::ledButtonPressed()
{
  led_state = !led_state;
  touchgfx_printf("IOControlView::%s = %d\r\n", __FUNCTION__, led_state);
  presenter->v2p_SetLEDState(led_state);
}