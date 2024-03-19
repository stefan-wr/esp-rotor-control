#ifndef SCREEN_H
#define SCREEN_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Timer.h>

#define SCREEN_WIDTH 128
#define SCREEN_HALF_WIDTH 64
#define SCREEN_HEIGHT 64
#define SCREEN_HALF_HEIGHT 32
#define CHAR_W 5
#define CHAR_H 7
#define N_PAGES 3

namespace Screen {
     
    class Screen {
    private:
        Timer *splash_screen_timer;
        bool on_splash_screen = true;

        Timer *alert_timer;
        String alert_txt;
        bool disabled = false;

        int page = 0;

        // Variables for drawing compass
        struct {
            float needle_sin, needle_cos;
            float needle_x1, needle_x2, needle_y1, needle_y2;
            float target_sin, target_cos;
            float target_x1, target_x2, target_y1, target_y2;
            float tick_sin, tick_cos;
            float tick_x1, tick_x2, tick_y1, tick_y2;
        } compass;

        // => Clear screen and reset text configurations and cursor
        void clearScreen();

        // => Move cursor coordinates by (nx, ny)
        void moveCursor(const int16_t &nx, const int16_t &ny);

        // => Print degree char
        void printDegree();

        // => Get width and height of given text on screen
        void getTextDimensions(const String &txt, uint16_t *txt_w, uint16_t  *txt_h);

        // => Set cursor so that text to be printed next is centered on full screen.
        // Needs the width and height of the text on screen from getTextDimenions().
        void setCenteredTextCursor(const uint16_t &txt_w, const uint16_t &txt_h);
        void setCenteredTextCursor(const String &txt);

        // => Set a horizontally centered text
        void setHorizontallyCenteredText(const uint16_t cy, const String &txt, const uint16_t cx_offset = 0);

        // => Set a fullscreen text
        void setFullscreenText(const String &txt);
        
        // => Set a title bar on screen
        void setTitleBar(const int &gap, const int icon, const String &title);

        // => Draw compass with radius r, centered at (cx, cy)
        void drawCompass(const uint16_t &cx, const uint16_t &cy, const float &r, uint16_t color);

        // => Draw sidebar with additional information
        void drawSidebar();

        // => Show a fullscreen centered message for a few seconds
        void showFullscreenAlert();

        // => Set splash-screen
        void showSplashScreen();

        // => Set screen for AP-mode
        void showAPModeScreen();

        // => Set network info screen
        void showNetworkScreen();

        // => Set system info screen
        void showSystemScreen();

        // => Set screen for when WiFi is reconnecting
        void showReconnectingScreen();

        // => Set the default screen when in STATION mode.
        void showDefaultScreen();

        // => Set screen for firmware update
        void showUpdateScreen();
        
    public:
        Adafruit_SSD1306 *screen;

        // Constructor
        Screen() {};
        
        // => Initialise screen
        bool init();

        // => Disable screen
        void disable() {
            disabled = true;
        }

        // => Enable screen
        void enable() {
            disabled = false;
        }

        // => Main draw function, to be called from main loop
        void update();

        // => Set an alert message to be shown full screen for a few seconds
        void setAlert(const String &txt);

        // => Set an alert message and show it on  the screen immediatly
        void setAlertImmediatly(const String &txt);

        // => Toggle through available screens
        void toggleScreens();
    };
}

extern Screen::Screen screen;

#endif // SCREEN_H
