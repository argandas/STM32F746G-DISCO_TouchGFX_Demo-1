/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#include <gui_generated/main_screen/MainViewBase.hpp>
#include <touchgfx/Color.hpp>
#include "BitmapDatabase.hpp"
#include <texts/TextKeysAndLanguages.hpp>

MainViewBase::MainViewBase() :
    flexButtonCallback(this, &MainViewBase::flexButtonCallbackHandler)
{
    backgroundBox.setPosition(0, 0, 800, 480);
    backgroundBox.setVisible(false);
    backgroundBox.setColor(touchgfx::Color::getColorFrom24BitRGB(0, 0, 0));

    backgroundImage.setXY(0, 0);
    backgroundImage.setBitmap(Bitmap(BITMAP_BG_ID));

    counterBackgroundImage.setXY(121, 68);
    counterBackgroundImage.setBitmap(Bitmap(BITMAP_COUNTER_BOX_ID));

    countTxt.setPosition(-14, 189, 152, 90);
    countTxt.setColor(touchgfx::Color::getColorFrom24BitRGB(222, 222, 222));
    countTxt.setLinespacing(0);
    Unicode::snprintf(countTxtBuffer, COUNTTXT_SIZE, "%s", TypedText(T_SINGLEUSEID1).getText());
    countTxt.setWildcard(countTxtBuffer);
    countTxt.setTypedText(TypedText(T_NUMBERTEXT));

    repeatButton.setDelay(6);
    repeatButton.setInterval(6);
    repeatButton.setBoxWithBorderPosition(0, 0, 100, 45);
    repeatButton.setBorderSize(5);
    repeatButton.setBoxWithBorderColors(touchgfx::Color::getColorFrom24BitRGB(222, 222, 222), touchgfx::Color::getColorFrom24BitRGB(48, 58, 64), touchgfx::Color::getColorFrom24BitRGB(54, 62, 65), touchgfx::Color::getColorFrom24BitRGB(128, 136, 143));
    repeatButton.setIconBitmaps(Bitmap(BITMAP_BLACK_ARROW_ID), Bitmap(BITMAP_ORANGE_ARROW_ID));
    repeatButton.setIconXY(34, 14);
    repeatButton.setPosition(12, 173, 100, 45);
    repeatButton.setAction(flexButtonCallback);

    zeroButton.setBitmaps(Bitmap(BITMAP_SMALL_BTN_ID), Bitmap(BITMAP_SMALL_BTN_PRESSED_ID));
    zeroButton.setBitmapXY(0, 0);
    zeroButton.setText(TypedText(T_SINGLEUSEID3));
    zeroButton.setTextPosition(-2, 12, 100, 56);
    zeroButton.setTextColors(touchgfx::Color::getColorFrom24BitRGB(70, 70, 70), touchgfx::Color::getColorFrom24BitRGB(231, 154, 9));
    zeroButton.setPosition(12, 117, 100, 56);
    zeroButton.setAction(flexButtonCallback);

    touchButton.setBoxWithBorderPosition(0, 0, 100, 45);
    touchButton.setBorderSize(5);
    touchButton.setBoxWithBorderColors(touchgfx::Color::getColorFrom24BitRGB(222, 222, 222), touchgfx::Color::getColorFrom24BitRGB(40, 58, 64), touchgfx::Color::getColorFrom24BitRGB(54, 62, 65), touchgfx::Color::getColorFrom24BitRGB(95, 103, 109));
    touchButton.setIconBitmaps(Bitmap(BITMAP_BLACK_ARROW_UP_ID), Bitmap(BITMAP_ORANGE_ARROW_UP_ID));
    touchButton.setIconXY(34, 14);
    touchButton.setPosition(12, 68, 100, 45);
    touchButton.setAction(flexButtonCallback);

    toggleButton.setText(TypedText(T_TOGGLEBUTTONWHITE));
    toggleButton.setTextPosition(0, 0, 152, 36);
    toggleButton.setTextColors(touchgfx::Color::getColorFrom24BitRGB(222, 222, 222), touchgfx::Color::getColorFrom24BitRGB(231, 154, 9));
    toggleButton.setPosition(293, 41, 152, 36);
    toggleButton.setAction(flexButtonCallback);

    toggleLabel.setXY(283, 20);
    toggleLabel.setColor(touchgfx::Color::getColorFrom24BitRGB(222, 222, 222));
    toggleLabel.setLinespacing(0);
    toggleLabel.setTypedText(TypedText(T_SINGLEUSEID8));

    ledButton.setBitmaps(Bitmap(BITMAP_SMALL_BTN_ID), Bitmap(BITMAP_SMALL_BTN_PRESSED_ID));
    ledButton.setBitmapXY(0, 0);
    ledButton.setText(TypedText(T_SINGLEUSEID9));
    ledButton.setTextPosition(-2, 12, 100, 56);
    ledButton.setTextColors(touchgfx::Color::getColorFrom24BitRGB(70, 70, 70), touchgfx::Color::getColorFrom24BitRGB(231, 154, 9));
    ledButton.setPosition(320, 117, 100, 56);
    ledButton.setAction(flexButtonCallback);

    add(backgroundBox);
    add(backgroundImage);
    add(counterBackgroundImage);
    add(countTxt);
    add(repeatButton);
    add(zeroButton);
    add(touchButton);
    add(toggleButton);
    add(toggleLabel);
    add(ledButton);
}

void MainViewBase::setupScreen()
{

}

void MainViewBase::flexButtonCallbackHandler(const touchgfx::AbstractButtonContainer& src)
{
    if (&src == &repeatButton)
    {
        //repeatButtonInteraction
        //When repeatButton clicked call virtual function
        //Call repeatButtonPressed
        repeatButtonPressed();
    }
    else if (&src == &zeroButton)
    {
        //zeroButtonInteraction
        //When zeroButton clicked call virtual function
        //Call zeroButtonPressed
        zeroButtonPressed();
    }
    else if (&src == &touchButton)
    {
        //touchButtonInteraction
        //When touchButton clicked call virtual function
        //Call touchButtonPressed
        touchButtonPressed();
    }
    else if (&src == &toggleButton)
    {
        //toggleButtonInteraction
        //When toggleButton clicked call virtual function
        //Call toggleButtonPressed
        toggleButtonPressed();
    }
    else if (&src == &ledButton)
    {
        //ledButtonInteraction
        //When ledButton clicked call virtual function
        //Call ledButtonPressed
        ledButtonPressed();
    }
}
