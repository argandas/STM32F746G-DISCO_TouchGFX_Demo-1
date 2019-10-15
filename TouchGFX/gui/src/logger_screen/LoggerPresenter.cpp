#include <gui/logger_screen/LoggerView.hpp>
#include <gui/logger_screen/LoggerPresenter.hpp>

LoggerPresenter::LoggerPresenter(LoggerView& v)
    : view(v)
{
}

void LoggerPresenter::activate()
{

}

void LoggerPresenter::deactivate()
{

}

void LoggerPresenter::v2p_LogData(int data)
{
  touchgfx_printf("LoggerPresenter::%s: %d\r\n", __FUNCTION__, data);
  model->p2m_LogData(data);
}

void LoggerPresenter::v2p_DumpData(void)
{
  touchgfx_printf("LoggerPresenter::%s: %d\r\n", __FUNCTION__, 0);
  model->p2m_DumpData();
}

void LoggerPresenter::v2p_SentData(void)
{
  touchgfx_printf("LoggerPresenter::%s: %d\r\n", __FUNCTION__, 0);
  model->p2m_SentData();
}

void LoggerPresenter::v2p_ClearData(void)
{
  touchgfx_printf("LoggerPresenter::%s: %d\r\n", __FUNCTION__, 0);
  model->p2m_ClearData();
}
