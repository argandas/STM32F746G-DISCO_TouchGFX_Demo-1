#ifndef NETWORK_VIEW_HPP
#define NETWORK_VIEW_HPP

#include <gui_generated/network_screen/NetworkViewBase.hpp>
#include <gui/network_screen/NetworkPresenter.hpp>

class NetworkView : public NetworkViewBase
{
public:
    NetworkView();
    virtual ~NetworkView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // NETWORK_VIEW_HPP
