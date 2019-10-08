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
    
    virtual void tcpButtonPressed();
    virtual void getIPAddress();
    virtual void p2v_SetIPAddress(char* ip_addr_ptr);

#if 0
    virtual void setIPAddress(char* ip_addr_ptr);
#endif
protected:
};

#endif // NETWORK_VIEW_HPP
