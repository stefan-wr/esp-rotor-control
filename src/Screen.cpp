#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#include <globals.h>
#include <Screen.h>
#include <RotorController.h>    // Exposes Global: rotor_ctrl
#include <WiFiFunctions.h>
#include <Timer.h>
#include <Firmware.h>

namespace Screen {

    // => Initialise screen
    bool Screen::init() {
        screen = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
        if (!screen->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Serial.println("[Screen] Failed to allocate RAM for the screen.");
            disabled = true;
            return false;
        }
       
        // Clear screen and configure charset
        clearScreen();
        screen->display();
        screen->dim(true);
        screen->cp437(true);

        // Show splash screen at ESP setup until timer expires
        showSplashScreen();
        splash_screen_timer = new Timer(2000);
        screen->display();

        // Setup alert messages
        alert_txt.reserve(32);
        alert_timer = new Timer(4000);

        return true;
    }

    // => Clear screen and reset text configurations and cursor
    void Screen::clearScreen() {
        screen->clearDisplay();
        screen->setFont();
        screen->setTextSize(1);
        screen->setTextColor(WHITE);
        screen->setCursor(0, 0);
    }

    // => Move cursor coordinates by (nx, ny)
    void Screen::moveCursor(const int16_t &nx, const int16_t &ny) {
        screen->setCursor(screen->getCursorX() + nx, screen->getCursorY() + ny);
    }

    // => Print degree char
    void Screen::printDegree() {
        screen->write(0xF8);
    }

    // => Get width and height of given text on screen
    void Screen::getTextDimensions(const String &txt, uint16_t *txt_w, uint16_t  *txt_h) {
        int16_t cx, cy;
        screen->getTextBounds(txt, 0, 0, &cx, &cy, txt_w, txt_h);  
    }

    // => Set cursor so that text to be printed next is centered on full screen.
    // Needs the width and height of the text on screen from getTextDimenions().
    void Screen::setCenteredTextCursor(const uint16_t &txt_w, const uint16_t &txt_h) {
        screen->setCursor((SCREEN_WIDTH - txt_w) / 2, (SCREEN_HEIGHT - txt_h) / 2);
    }

    // => Set a fullscreen text to screen RAM
    void Screen::setFullscreenText(const String &txt) {
        uint16_t txt_w, txt_h;
        getTextDimensions(txt, &txt_w, &txt_h);
        setCenteredTextCursor(txt_w, txt_h);
        screen->print(txt);    
    }

    // => Set an alert message to be shown full screen for a few seconds
    void Screen::setAlert(const String &txt) {
        alert_txt = txt;
        alert_timer->start();
    }

    // => Set an alert message and show it on  the screen immediatly
    void Screen::setAlertImmediatly(const String &txt) {
        if (!disabled) {
            alert_txt = txt;
            alert_timer->start();
            clearScreen();
            showFullscreenAlert();
            screen->display();
        }
    }


    // => Draw compass with radius r, centered at (cx, cy)
    // ---------------------------------------------------
    void Screen::drawCompass(const uint16_t &cx, const uint16_t &cy, const float &r, uint16_t color) {
        // Draw compass outline and center dot
        screen->drawCircle(cx, cy, r, color);
        screen->fillCircle(cx, cy, 2, color);

        // Draw compass needle
        compass.needle_sin = sin(rotor_ctrl.rotor.last_angle_rad);
        compass.needle_cos = cos(rotor_ctrl.rotor.last_angle_rad);
        compass.needle_x1 = round(cx + compass.needle_sin * (r - 4));
        compass.needle_y1 = round(cy - compass.needle_cos * (r - 4));
        compass.needle_x2 = round(cx - compass.needle_sin * (r / 3));
        compass.needle_y2 = round(cy + compass.needle_cos * (r / 3));
        screen->drawLine(compass.needle_x2, compass.needle_y2,
                         compass.needle_x1, compass.needle_y1, color);

        // Draw compass target indicator
        if (rotor_ctrl.is_auto_rotating) {
            compass.target_sin = sin(rotor_ctrl.auto_rotation_target_rad);
            compass.target_cos = cos(rotor_ctrl.auto_rotation_target_rad);
            compass.target_x1 = round(cx + compass.target_sin * (r - 4));
            compass.target_y1 = round(cy - compass.target_cos * (r - 4));
            compass.target_x2 = round(cx + compass.target_sin * (r / 2));
            compass.target_y2 = round(cy - compass.target_cos * (r / 2));
            screen->drawLine(compass.target_x2, compass.target_y2,
                             compass.target_x1, compass.target_y1, color); 
        }        
    }


    // => Draw sidebar with additional information
    // -------------------------------------------
    void Screen::drawSidebar() {
        screen->fillRoundRect(SCREEN_WIDTH - 19, 0, 19, SCREEN_HEIGHT, 4, WHITE); // 109,0
        screen->setTextColor(BLACK);
        
        // Rotation indicators
        if (rotor_ctrl.is_rotating) {
            if (rotor_ctrl.direction) {
                screen->fillTriangle(114, 5, 122, 9, 114, 13, BLACK); 
            } else {
                screen->fillTriangle(114, 9, 122, 5, 122, 13, BLACK);
            }
        } else {
            screen->fillRoundRect(114, 5, 9, 9, 2, BLACK);
        }

        // Overlap indicator
        if (rotor_ctrl.rotor.last_angle > 360.0f) {
            screen->setCursor(113, 19);
            screen->print("OL");
        }

        // N of connected clients
        screen->setCursor(116, 52);
        screen->print(clients_connected);
    }


    // => Set splash-screen
    // --------------------
    void Screen::showSplashScreen() {
        String txt = "RotorControl";
        uint16_t txt_w, txt_h;
        getTextDimensions(txt, &txt_w, &txt_h);
        setCenteredTextCursor(txt_w, txt_h);
        screen->drawRect(screen->getCursorX() - 5, screen->getCursorY() - 5,
                         txt_w + 8, txt_h + 9, WHITE);
        screen->print(txt);
        getTextDimensions(version, &txt_w, &txt_h);
        setCenteredTextCursor(txt_w, txt_h);
        moveCursor(0, 18);
        screen->print(version);
    }


    // => Set screen for AP-mode
    // -------------------------
    void Screen::showAPModeScreen() {
        screen->setCursor(0, 0);
        screen->print("WiFi not connected.");
        screen->drawFastHLine(0, 11, 128, WHITE);
        screen->setCursor(0, 15);
        screen->println("URL:");
        moveCursor(0, 3);
        screen->println(get_ip_url());
        moveCursor(0, 3);
        screen->println("or");
        moveCursor(0, 4);
        screen->print("http://");
        screen->print(local_url);
        screen->println(".local");
    }    


    // => Set the default screen when in STATION mode.
    // -----------------------------------------------
    void Screen::showDefaultScreen() {
        // Draw compass
        drawCompass(SCREEN_HALF_WIDTH + 8, SCREEN_HALF_HEIGHT, 30, WHITE);

        // Sidebar
        drawSidebar();

        // Draw labels
        // ***********
        int16_t ly = 1;
        int16_t gap = 4;
        screen->setTextColor(WHITE);

        // Angle
        ly += gap;
        screen->setCursor(0, ly);
        screen->printf("%3.0f", round(rotor_ctrl.rotor.last_angle));
        printDegree();
        
        // ---
        ly += CHAR_H + gap;
        screen->drawFastHLine(0, ly, 41, WHITE);

        // Volts
        ly += 1 + gap;
        screen->setCursor(0, ly);
        screen->printf("%4.2f V", rotor_ctrl.rotor.last_adc_volts * rotor_ctrl.rotor.calibration.volt_div_factor);

        // ---
        ly += CHAR_H + gap;
        screen->drawFastHLine(0, ly, 38, WHITE);

        // Target
        ly += 1 + gap;
        screen->setCursor(0, ly);
        if (rotor_ctrl.is_auto_rotating) {
            screen->printf("T %3.0f", round(rotor_ctrl.auto_rotation_target));
            printDegree();
        } else {
            screen->print("T ---");
        }

        // ---
        ly += CHAR_H + gap;
        screen->drawFastHLine(0, ly, 41, WHITE);

        // Speed
        ly += 1 + gap;
        screen->setCursor(0, ly);
        if (rotor_ctrl.max_speed) {
            screen->printf("S %3d%%", rotor_ctrl.max_speed);
        } else {
            screen->printf("S %3d%%", rotor_ctrl.max_speed + 1);
        }

        // Angular speed label
        if (false) {
            screen->setCursor(0, SCREEN_HEIGHT - CHAR_H);
            screen->printf("%5.1f", rotor_ctrl.angular_speed);
            printDegree();
            screen->print("/s");
        }
    }


    // => Set screen during firmware update
    // ------------------------------------
    void Screen::showUpdateScreen() {
        setFullscreenText("Updating Firmware");
        uint16_t txt_w, txt_h;
        getTextDimensions("100%", &txt_w, &txt_h);
        setCenteredTextCursor(txt_w, txt_h);
        moveCursor(0, 16);
        screen->print(firmware.upload_progress);
        screen->print("%");
    }

    // => Show a fullscreen alert
    // --------------------------
    void Screen::showFullscreenAlert() {
        setFullscreenText(alert_txt);
    }


    // **************************************************
    // => Main draw function, to be called from main loop
    // **************************************************
    void Screen::update() {
        // Only clear screen if it is disabled
        if (disabled) {
            screen->clearDisplay();
            screen->display();
            return;
        }

        // The splash screen is shown during setup, set by init().
        // Show splash screen, until timer expires.
        if (on_splash_screen) {
            if (splash_screen_timer->passed()) {
                on_splash_screen = false;
            } else {
                return;
            }
        }

        // Clear screen
        clearScreen();

        // Alert message, full screen, until timed out
        if (alert_txt != "" && !firmware.is_updating) {
            if (alert_timer->passed()) {
                alert_txt = "";
            } else {
                showFullscreenAlert();
                screen->display();
                return;
            }
        }

        // ----------

        if (in_station_mode) {
            if (firmware.is_updating) {
                showUpdateScreen();
            } else {
                showDefaultScreen();
            }
        } else {
            showAPModeScreen();
        }

        // ----------
        
        screen->display();
    }    
}

Screen::Screen screen;
