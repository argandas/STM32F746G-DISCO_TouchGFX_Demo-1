#include <gui/iocontrol_screen/IOControlView.hpp>
#include <gui/iocontrol_screen/IOControlPresenter.hpp>

IOControlPresenter::IOControlPresenter(IOControlView& v)
    : view(v)
{
}

void IOControlPresenter::activate()
{

}

void IOControlPresenter::deactivate()
{

}

void IOControlPresenter::v2p_SetLEDState(bool state)
{
  touchgfx_printf("MainPresenter::%s: %d\r\n", __FUNCTION__, state);
  model->p2m_SetLEDState(state);
}