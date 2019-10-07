#ifndef IOCONTROL_VIEW_HPP
#define IOCONTROL_VIEW_HPP

#include <gui_generated/iocontrol_screen/IOControlViewBase.hpp>
#include <gui/iocontrol_screen/IOControlPresenter.hpp>

class IOControlView : public IOControlViewBase
{
public:
    IOControlView();
    virtual ~IOControlView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void ledButtonPressed();

	bool led_state;

protected:
};

#endif // IOCONTROL_VIEW_HPP
