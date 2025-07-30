// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2025 Joe Lesko

#pragma once

#include <Arduino.h>

// Forward declarations for display libraries
class Adafruit_SSD1306;
class Adafruit_ST7789;
class TFT_eSPI;

class DisplayManager
{
public:
    virtual ~DisplayManager() {}
    virtual void init() = 0; // Initialize the display, if needed
    virtual void begin() = 0;
    virtual void clear() = 0;
    virtual void setTextSize(uint8_t size) = 0;
    virtual void setCursor(int16_t x, int16_t y) = 0;
    virtual void setTextColor(uint16_t color) = 0;
    virtual void print(const String &text) = 0;
    virtual void println(const String &text) = 0;
    virtual void display() = 0;
    virtual bool toggle() = 0;
    virtual void redBtn() {}   // Optional method for red button, can be overridden
    virtual void greenBtn() {} // Optional method for red button, can be overridden
};

// ###############################################################################
// SSD1306 display 128x64 .96 Inch OLED Display Screen
// #####################################################################################

#ifdef USE_SSD1306
#include <Adafruit_SSD1306.h>

#define WHITE SSD1306_WHITE
#define BLACK SSD1306_BLACK
#define RED SSD1306_WHITE
#define GREEN SSD1306_WHITE
#define BLUE SSD1306_WHITE
#define ORANGE SSD1306_WHITE
#define SKYBLUE SSD1306_WHITE
#define PURPLE SSD1306_WHITE
#define YELLOW SSD1306_WHITE
#define BROWN SSD1306_WHITE
#define PINK SSD1306_WHITE
#define GREY SSD1306_WHITE

class SSD1306Display : public DisplayManager
{
private:
    bool toggleDisplay = true;

public:
    Adafruit_SSD1306 *disp;

public:
    SSD1306Display(Adafruit_SSD1306 *d) : disp(d) {}
    void init() override
    {
        disp->begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
        disp->clearDisplay();
        toggleDisplay = true;
    }
    void begin() override
    {
        disp->clearDisplay();
        disp->setTextColor(WHITE);
        disp->setTextSize(1);
        disp->setCursor(0, 0);
    }
    void clear() override { disp->clearDisplay(); }
    void setTextSize(uint8_t size) override { disp->setTextSize(size); }
    void setCursor(int16_t x, int16_t y) override { disp->setCursor(x, y); }
    void setTextColor(uint16_t color) override { disp->setTextColor(color); }
    void print(const String &text) override { disp->print(text); }
    void println(const String &text) override { disp->println(text); }
    void display() override
    {
        disp->display();
    }
    bool toggle() override
    {
        if (toggleDisplay)
        {
            toggleDisplay = false;
            disp->ssd1306_command(SSD1306_DISPLAYOFF);
        }
        else
        {
            toggleDisplay = true;
            disp->ssd1306_command(SSD1306_DISPLAYON);
        }
        return (toggleDisplay);
    }
};
#endif

// #####################################################################################
// ST7789 170x320 TFT LCD Display
// #####################################################################################
#ifdef USE_ST7789
#include <Adafruit_ST7789.h>
#define WHITE ST77XX_WHITE
#define BLACK ST77XX_BLACK
#define RED ST77XX_RED
#define GREEN ST77XX_GREEN
#define BLUE ST77XX_BLUE
#define ORANGE ST77XX_ORANGE
#define SKYBLUE ST77XX_SKYBLUE
#define PURPLE ST77XX_PURPLE
#define YELLOW ST77XX_YELLOW
#define BROWN ST77XX_BROWN
#define PINK ST77XX_PINK
#define GREY ST77XX_GREY
#define DARKGREY ST77XX_DARKGREY

#define LCD_MOSI 23
#define LCD_SCLK 18
#define LCD_CS 15
#define LCD_DC 2
#define LCD_RST 4
#define LCD_BL 32 // Backlight pin, if needed

class ST7789Display : public DisplayManager
{
private:
    bool toggleDisplay = true;

public:
    Adafruit_ST7789 *disp;

public:
    ST7789Display(Adafruit_ST7789 *d) : disp(d) {}
    void init() override
    {
        // pinMode(TFT_BL, OUTPUT);
        pinMode(LCD_BL, OUTPUT);
        digitalWrite(LCD_BL, HIGH);
        disp->init(170, 320);
        disp->fillScreen(ST77XX_BLACK);
    }
    void begin() override
    {
        disp->setCursor(0, 0);
        disp->setTextColor(ST77XX_WHITE);
        disp->setTextSize(1);
    }
    void clear() override
    {
        disp->fillScreen(ST77XX_BLACK);
        disp->setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        disp->setTextSize(1);
        disp->setCursor(0, 0);
        disp->setRotation(3);
    }
    void setTextSize(uint8_t size) override { disp->setTextSize(size); }
    void setCursor(int16_t x, int16_t y) override { disp->setCursor(x, y); }
    void setTextColor(uint16_t color) override { disp->setTextColor(color, ST77XX_BLACK); }
    void print(const String &text) override { disp->print(text); }
    void println(const String &text) override { disp->println(text); }
    void display() override {}                // Not needed for ST7789
    // bool toggle() override { return (true); } // Not needed for ST7789
    bool toggle() override
    {
        if (toggleDisplay)
        {
            toggleDisplay = false;
            //disp->writeCommand(ST77XX_DISPOFF); // Send Display OFF command (0x28)
            // Optionally, enter sleep mode for lower power
            //vTaskDelay(200);
            //disp->writeCommand(ST77XX_SLPIN); // Send Sleep In command (0x10)
            digitalWrite(LCD_BL, LOW);
        }
        else
        {
            toggleDisplay = true;
            //disp->writeCommand(ST77XX_SLPOUT); // Exit Sleep Mode (0x11)
            vTaskDelay(200);
            //disp->writeCommand(ST77XX_DISPON); // Display ON (0x29)
            //disp->fillScreen(ST77XX_BLACK);
            //disp->init(170, 320);
            digitalWrite(LCD_BL, HIGH);
        }
        return (toggleDisplay);
    }
};
#endif

// #####################################################################################
// Cheap Yellow Display 2.8
// SP32 Development Board ESP32-2432S028R WiFi Bluetooth 2.8 Inch 240X320
// #####################################################################################
#ifdef USE_TFT
#include <TFT_eSPI.h>

#define BLACK TFT_BLACK
#define WHITE TFT_WHITE
#define SKYBLUE TFT_SKYBLUE
#define ORANGE TFT_ORANGE
#define BLUE TFT_BLUE
#define GREEN TFT_GREEN
#define PURPLE TFT_PURPLE
#define YELLOW TFT_YELLOW
#define BROWN TFT_BROWN
#define RED TFT_RED
#define PINK TFT_PINK
#define GREY TFT_GREY
#define DARKGREY TFT_DARKGREY

// Comment out to stop drawing black spots
#define BLACK_SPOT

// Switch position and size
#define FRAME_X 50
#define FRAME_Y 150
#define FRAME_W 120
#define FRAME_H 50

// Red zone size
#define REDBUTTON_X FRAME_X
#define REDBUTTON_Y FRAME_Y
#define REDBUTTON_W (FRAME_W / 2)
#define REDBUTTON_H FRAME_H

// Green zone size
#define GREENBUTTON_X (REDBUTTON_X + REDBUTTON_W)
#define GREENBUTTON_Y FRAME_Y
#define GREENBUTTON_W (FRAME_W / 2)
#define GREENBUTTON_H FRAME_H

class TFTDisplay : public DisplayManager
{
private:
    bool toggleDisplay = true;

public:
    TFT_eSPI *disp;
    bool SwitchOn = false;

public:
    TFTDisplay(TFT_eSPI *d) : disp(d) {}
    void init() override
    {
        pinMode(TFT_BL, OUTPUT);
        disp->init();
        disp->setSwapBytes(true);
    }
    void begin() override { disp->setTextColor(WHITE, BLACK); } // Not used for TFT_eSPI, initialization is done in init()
    void clear() override
    {
        disp->fillScreen(BLACK);
        disp->setCursor(0, 0);
        disp->setTextColor(WHITE, BLACK);
        disp->setTextSize(1);
        disp->setRotation(1);
    }
    void setTextSize(uint8_t size) override { disp->setTextSize(size); }
    void setCursor(int16_t x, int16_t y) override { disp->setCursor(x, y); }
    void setTextColor(uint16_t color) override { disp->setTextColor(color, BLACK); }
    void print(const String &text) override { disp->print(text); }
    void println(const String &text) override { disp->println(text); }
    void display() override {} // Not needed for TFT_eSPI
    bool toggle() override
    {
        if (toggleDisplay)
        {
            toggleDisplay = false;
            digitalWrite(TFT_BL, LOW);
        }
        else
        {
            toggleDisplay = true;
            digitalWrite(TFT_BL, HIGH);
        }
        return (toggleDisplay);
    }

    void drawFrame()
    {
        disp->drawRect(FRAME_X, FRAME_Y, FRAME_W, FRAME_H, TFT_BLACK);
    }

    // Draw a red button
    void redBtn()
    {
        disp->fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, TFT_RED);
        disp->fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, TFT_DARKGREY);
        drawFrame();
        disp->setTextColor(WHITE, BLACK);
        disp->setTextSize(2);
        disp->setTextDatum(MC_DATUM);
        disp->drawString("ON", GREENBUTTON_X + (GREENBUTTON_W / 2), GREENBUTTON_Y + (GREENBUTTON_H / 2));
        SwitchOn = false;
    }

    // Draw a green button
    void greenBtn()
    {
        disp->fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, TFT_GREEN);
        disp->fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, TFT_DARKGREY);
        drawFrame();
        disp->setTextColor(WHITE, BLACK);
        disp->setTextSize(2);
        disp->setTextDatum(MC_DATUM);
        disp->drawString("OFF", REDBUTTON_X + (REDBUTTON_W / 2) + 1, REDBUTTON_Y + (REDBUTTON_H / 2));
        SwitchOn = true;
    }

    // Check if the switch is on
    bool isSwitchOn() const
    {
        return SwitchOn;
    }
};
#endif
