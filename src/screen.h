#pragma once
#include "Arduino.h"
#include "U8g2lib.h"
#include <SPI.h>

class screen
{
public:
    screen(byte sck, byte sid, byte cs) : disp(U8G2_R0, cs, U8X8_PIN_NONE) // disp(U8G2_R0, sck, sid, cs, U8X8_PIN_NONE)
    {
        this->sck = sck;
        this->sid = sid;
        this->cs = cs;
    }

    void screenBegin()
    {
        disp.begin();
        setFont(1);
    }
    // void topRightJustifiedText(char text[256], byte yOffset);
    // void centerJustifiedText(char text[256], byte yOffset);
    void topRightJustifiedText(String text, byte yOffset);
    void centerJustifiedText(String text, byte yOffset);
    void barGraph(byte percent, byte offset);
    void bitmap(byte x, byte y, byte w, byte h, uint8_t *bmp) { disp.drawXBMP(x, y, w, h, bmp); }

    void setFont(byte font)
    {
        switch (font)
        {
        case 1:
            disp.setFont(u8g2_font_t0_11_mf);
            break;
        default:
            disp.setFont(u8g2_font_t0_11_mf);
            break;
        }
    };

    void clearScreen()
    {
        disp.clearDisplay();
    };

    void clearBox(int x, int y, int xSize, int ySize)
    {
        disp.setDrawColor(0); // transparent draw color
        disp.drawBox(x, y, xSize, ySize);
        disp.setDrawColor(1); // transparent draw color
    };

    void writeScreen()
    {
        // disp.sendBuffer();
        disp.updateDisplay();
    };

private:
    U8G2_ST7920_128X64_F_HW_SPI disp;
    byte centerX = disp.getDisplayWidth() / 2;
    byte centerY = disp.getDisplayHeight() / 2;
    byte lastPercent = 0;
    byte sck;
    byte sid;
    byte cs;
};
