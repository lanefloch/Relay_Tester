#include <Arduino.h>
#include "screen.h"
#include <SPI.h>
#include "U8g2lib.h"

/*void screen::topRightJustifiedText(char text[256], byte yOffset)
{
    disp.setFontPosBottom();
    byte height = disp.getAscent() - disp.getDescent();
    disp.drawUTF8(0, height + yOffset, text);
}*/

void screen::topRightJustifiedText(String text, byte yOffset)
{
    disp.setFontPosBottom();
    byte height = disp.getAscent() - disp.getDescent();
    disp.setCursor(0, height + yOffset);
    disp.print(text);
}

/*void screen::centerJustifiedText(char text[256], byte yOffset)
{
    disp.setFontPosBottom();
    byte height = disp.getAscent() - disp.getDescent();
    byte width = disp.getUTF8Width(text);
    byte textCenterX = centerX - (width / 2);
    byte textCenterY = centerY + (height / 2);
    disp.drawUTF8(textCenterX, textCenterY + yOffset, text);
}*/

void screen::centerJustifiedText(String text, byte yOffset)
{
    disp.setFontPosBottom();
    byte height = disp.getAscent() - disp.getDescent();
    byte width = disp.getStrWidth(text.c_str());
    byte textCenterX = centerX - (width / 2);
    byte textCenterY = centerY + (height / 2);
    disp.setCursor(textCenterX, textCenterY + yOffset);
    disp.print(text);
}

void screen::barGraph(byte percent, byte offset)
{
    if (percent != lastPercent)
    {
        int i = 0;
        disp.drawFrame(8, centerY - 12 + offset, disp.getDisplayWidth() - 8, 24);
        if (percent > 100)
            percent = 100;
        else if (percent < 0)
            percent = 0;
        i = map(percent, 0, 100, 0, disp.getDisplayWidth() - 12);
        disp.drawBox(10, centerY - 10 + offset, i, 20);
        lastPercent = percent;
    }
    else
        return;
}