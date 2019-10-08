#include <gui/network_screen/NetworkView.hpp>
#include <gui/network_screen/NetworkPresenter.hpp>

NetworkPresenter::NetworkPresenter(NetworkView& v)
    : view(v)
{
}

void NetworkPresenter::activate()
{

}

void NetworkPresenter::deactivate()
{

}

void NetworkPresenter::v2p_SendTCPData(int data)
{
  touchgfx_printf("NetworkPresenter::%s: %d\r\n", __FUNCTION__, data);
  model->p2m_SendTCPData(data);
}

void NetworkPresenter::v2p_getIPAddress()
{
  touchgfx_printf("NetworkPresenter::%s\r\n", __FUNCTION__);
  model->p2m_getIPAddress();
}

void NetworkPresenter::m2p_SetIPAddress(char* ip_addr_ptr)
{
  view.p2v_SetIPAddress(ip_addr_ptr);
}