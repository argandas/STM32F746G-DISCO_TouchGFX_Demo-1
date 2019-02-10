#include <gui/main_screen/MainView.hpp>
#include <gui/main_screen/MainPresenter.hpp>
#include <touchgfx/Utils.hpp>

MainPresenter::MainPresenter(MainView& v)
    : view(v)
{
}

void MainPresenter::activate()
{
}

void MainPresenter::deactivate()
{
}

void MainPresenter::m2p_ButtonPressed()
{
  view.hw_ButtonPressed();
}

void MainPresenter::v2p_SetLEDState(bool state)
{
  touchgfx_printf("MainPresenter::v2p_SetLEDState: %d\r\n", state);
  model->p2m_SetLEDState(state);
}

void MainPresenter::v2p_SendTCPData(int data)
{
  touchgfx_printf("MainPresenter::v2p_SendTCPData: %d\r\n", data);
  model->p2m_SendTCPData(data);
}


