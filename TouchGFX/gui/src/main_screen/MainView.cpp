#include <gui/main_screen/MainView.hpp>
#include "BitmapDatabase.hpp"
#include <touchgfx/Color.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Utils.hpp>

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

    /* Disable button */
    repeatButton.setAlpha(100);
    repeatButton.setTouchable(false);
    repeatButton.setPressed(false);
    repeatButton.invalidate();
    
    /* Enable button */
    if (touchButton.getAlpha() == 100)
    {
        touchButton.setAlpha(255);
        touchButton.setTouchable(true);
        touchButton.invalidate();
    }
}

void MainView::hw_ButtonPressed()
{
    inc_count();
}

void MainView::touchButtonPressed()
{
    inc_count();
}

void MainView::inc_count()
{
    int tmpVal = Unicode::atoi(countTxt.getWildcard());
    if (tmpVal < 50)
    {
      /* Update count text */
      tmpVal++;
      Unicode::snprintf(countTxtBuffer, COUNTTXT_SIZE, "%d", tmpVal);
      countTxt.invalidate();

      /* Enable button */
      if (repeatButton.getAlpha() == 100)
      {
          repeatButton.setAlpha(255);
          repeatButton.setTouchable(true);
          repeatButton.invalidate();
      }
    }
    
    if (tmpVal >= 50)
    {
      /* Disable button */
      touchButton.setAlpha(100);
      touchButton.setTouchable(false);
      touchButton.setPressed(false);
      touchButton.invalidate();
      
      tmpVal = 50;
    }
}

void MainView::dec_count()
{
    int tmpVal = Unicode::atoi(countTxt.getWildcard());
    if (tmpVal > 0)
    {
      /* Update count text */
      tmpVal--;
      Unicode::snprintf(countTxtBuffer, COUNTTXT_SIZE, "%d", tmpVal);
      countTxt.invalidate();

      /* Enable button */
      if (touchButton.getAlpha() == 100)
      {
          touchButton.setAlpha(255);
          touchButton.setTouchable(true);
          touchButton.invalidate();
      }
    }
    
    if (tmpVal <= 0)
    {
      /* Disable button */
      repeatButton.setAlpha(100);
      repeatButton.setTouchable(false);
      repeatButton.setPressed(false);
      repeatButton.invalidate();
      
      tmpVal = 0;
    }
}

void MainView::repeatButtonPressed()
{
    dec_count();
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

void MainView::tcpButtonPressed()
{
  int tmpVal = Unicode::atoi(countTxt.getWildcard());
  touchgfx_printf("MainView::%s = %d\r\n", __FUNCTION__, tmpVal);
  presenter->v2p_SendTCPData(tmpVal);
}
