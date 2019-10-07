#ifndef LOGGER_PRESENTER_HPP
#define LOGGER_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class LoggerView;

class LoggerPresenter : public Presenter, public ModelListener
{
public:
    LoggerPresenter(LoggerView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();
    
    void v2p_LogData(int data);
    void v2p_DumpData(void);

    virtual ~LoggerPresenter() {};

private:
    LoggerPresenter();

    LoggerView& view;
};


#endif // LOGGER_PRESENTER_HPP
