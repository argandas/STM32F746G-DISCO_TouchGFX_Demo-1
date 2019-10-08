#include <gui/network_screen/NetworkView.hpp>

NetworkView::NetworkView()
{

}

void NetworkView::setupScreen()
{
    NetworkViewBase::setupScreen();

    /* Set invalid IP and request to get current */
    Unicode::strncpy(&ipAddrTextBuffer[0], "0.0.0.0", IPADDRTEXT_SIZE);
    ipAddrText.invalidate();
    getIPAddress();
}

void NetworkView::tearDownScreen()
{
    NetworkViewBase::tearDownScreen();
}

void NetworkView::tcpButtonPressed()
{
  int tmpVal = 77; //Unicode::atoi(countTxt.getWildcard());
  touchgfx_printf("NetworkView::%s = %d\r\n", __FUNCTION__, tmpVal);
  presenter->v2p_SendTCPData(tmpVal);
}

void NetworkView::getIPAddress()
{
  touchgfx_printf("NetworkView::%s\r\n", __FUNCTION__);
  presenter->v2p_getIPAddress();
}

void NetworkView::p2v_SetIPAddress(char* ip_addr_ptr)
{
    // Unicode::strncpy(&ipAddrTextBuffer[0], "192.168.0.0", IPADDRTEXT_SIZE);
    Unicode::strncpy(&ipAddrTextBuffer[0], ip_addr_ptr, IPADDRTEXT_SIZE);
	ipAddrText.resizeToCurrentText();
    ipAddrText.invalidate();
}