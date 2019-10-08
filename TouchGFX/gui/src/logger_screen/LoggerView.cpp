#include <gui/logger_screen/LoggerView.hpp>

LoggerView::LoggerView()
{

}

void LoggerView::setupScreen()
{
    LoggerViewBase::setupScreen();
}

void LoggerView::tearDownScreen()
{
    LoggerViewBase::tearDownScreen();
}

void LoggerView::logButtonPressed()
{
  int tmpVal = Unicode::atoi(countTxt.getWildcard());
  touchgfx_printf("MainView::%s = %d\r\n", __FUNCTION__, tmpVal);
  presenter->v2p_LogData(tmpVal);
}

void LoggerView::dumpButtonPressed()
{
  touchgfx_printf("MainView::%s\r\n", __FUNCTION__);
  presenter->v2p_DumpData();
}