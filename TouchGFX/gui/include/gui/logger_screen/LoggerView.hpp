#ifndef LOGGER_VIEW_HPP
#define LOGGER_VIEW_HPP

#include <gui_generated/logger_screen/LoggerViewBase.hpp>
#include <gui/logger_screen/LoggerPresenter.hpp>

class LoggerView : public LoggerViewBase
{
public:
    LoggerView();
    virtual ~LoggerView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void logButtonPressed();
    virtual void dumpButtonPressed();
    virtual void sentButtonPressed();
    virtual void clearButtonPressed();

protected:
};

#endif // LOGGER_VIEW_HPP
