#include <gui/main_screen/MainView.hpp>
#include "BitmapDatabase.hpp"
#include <touchgfx/Color.hpp>
#include <texts/TextKeysAndLanguages.hpp>

MainView::MainView()
{
    // Support of larger displays for this example
    // is handled by showing a black box in the
    // unused part of the display.
    if (HAL::DISPLAY_WIDTH > backgroundImage.getWidth() ||
            HAL::DISPLAY_HEIGHT > backgroundImage.getHeight())
    {
        backgroundBox.setVisible(true);
    }
}

void MainView::setupScreen()
{
}

void MainView::tearDownScreen()
{
}

void MainView::zeroButtonPressed()
{
    Unicode::snprintf(countTxtBuffer, COUNTTXT_SIZE, "%d", 0);
    countTxt.invalidate();

    repeatButton.setAlpha(100);
    repeatButton.setTouchable(false);
    repeatButton.setPressed(false);
    repeatButton.invalidate();
}

void MainView::touchButtonPressed()
{
    int tmpVal = Unicode::atoi(countTxt.getWildcard());
    if (tmpVal <= 50)
    {
        tmpVal++;
        Unicode::snprintf(countTxtBuffer, COUNTTXT_SIZE, "%d", tmpVal);
        countTxt.invalidate();
        if (repeatButton.getAlpha() == 100)
        {
            repeatButton.setAlpha(255);
            repeatButton.setTouchable(true);
            repeatButton.invalidate();
        }

        if (tmpVal == 50)
        {
//            clickButton.setAlpha(100);
//            clickButton.setTouchable(false);
//            clickButton.setPressed(false);
//            clickButton.invalidate();

            touchButton.setAlpha(100);
            touchButton.setTouchable(false);
            touchButton.setPressed(false);
            touchButton.invalidate();
        }
    }
}

void MainView::repeatButtonPressed()
{
    int tmpVal = Unicode::atoi(countTxt.getWildcard());
    if (tmpVal >= 0)
    {
        tmpVal--;
        Unicode::snprintf(countTxtBuffer, COUNTTXT_SIZE, "%d", tmpVal);
        countTxt.invalidate();
        if (touchButton.getAlpha() == 100)
        {
 #if 0
            clickButton.setAlpha(255);
            clickButton.setTouchable(true);
            clickButton.invalidate();
#endif

            touchButton.setAlpha(255);
            touchButton.setTouchable(true);
            touchButton.invalidate();
        }

        if (tmpVal <= 0)
        {
            repeatButton.setAlpha(100);
            repeatButton.setTouchable(false);
            repeatButton.setPressed(false);
            repeatButton.invalidate();
        }
    }
}

void MainView::toggleButtonPressed()
{
    if (countTxt.getColor() == Color::getColorFrom24BitRGB(222, 222, 222))
    {
        countTxt.setColor(Color::getColorFrom24BitRGB(231, 154, 9));
        toggleButton.setText(TypedText(T_TOGGLEBUTTONORANGE));
    }
    else
    {
        countTxt.setColor(Color::getColorFrom24BitRGB(222, 222, 222));
        toggleButton.setText(TypedText(T_TOGGLEBUTTONWHITE));
    }
    countTxt.invalidate();
    toggleButton.invalidate();
}
