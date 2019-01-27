#include <gui/main_screen/MainView.hpp>
#include <gui/main_screen/MainPresenter.hpp>

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

void MainPresenter::btnPressed()
{
  view.hw_ButtonPressed();
}

void MainPresenter::setLED(bool state)
{
  model->model_ledSetState(state);
}
