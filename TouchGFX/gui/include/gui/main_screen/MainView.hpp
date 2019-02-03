#ifndef MAIN_VIEW_HPP
#define MAIN_VIEW_HPP

#include <gui_generated/main_screen/MainViewBase.hpp>
#include <gui/main_screen/MainPresenter.hpp>

class MainView : public MainViewBase
{
public:
    MainView();
    virtual ~MainView(){}
    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void repeatButtonPressed();
    virtual void zeroButtonPressed();
    virtual void touchButtonPressed();
    virtual void toggleButtonPressed();
    virtual void ledButtonPressed();
    virtual void hw_ButtonPressed();

    bool led_state;

private:
  void inc_count();
  void dec_count();
};

#endif // MAIN_VIEW_HPP
