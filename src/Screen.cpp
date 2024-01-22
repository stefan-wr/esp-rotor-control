#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#include <globals.h>
#include <Screen.h>
#include <RotorController.h>
#include <WiFiFunctions.h>
#include <Timer.h>
#include <SimpleUpdater.h>

extern Rotor::RotorController rotor_ctrl;

namespace Screen {

    // => Initialise screen
    bool Screen::init() {
        screen = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
        if (!screen->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Serial.println("[SCREEN] Failed to allocate RAM for the screen.");
            return false;
        }

        // Clear screen and configurate charset
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

    // => Draw compass with radius r, centered at (cx, cy)
    void Screen::drawCompass(const uint16_t &cx, const uint16_t &cy, const float &r, uint16_t color) {
        // Draw compass outline and center
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
            compass.target_x2 = round(cx + compass.target_sin * (r - 14));
            compass.target_y2 = round(cy - compass.target_cos * (r - 14));
            screen->drawLine(compass.target_x2, compass.target_y2,
                             compass.target_x1, compass.target_y1, color); 
        }        
    }

    // => Set a fullscreen text to screen RAM
    void Screen::setFullscreenText(const String &txt) {
        uint16_t txt_w, txt_h;
        getTextDimensions(txt, &txt_w, &txt_h);
        setCenteredTextCursor(txt_w, txt_h);
        screen->print(txt);    
    }

    // => Show a fullscreen centered message for a few seconds
    void Screen::showFullscreenAlert() {
        setFullscreenText(alert_txt);
    }

    // => Set splash-screen to screen RAM
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

    // => Set screen for AP-mode to screen RAM
    void Screen::showAPModeScreen() {
        screen->setCursor(0, 0);
        screen->print("WiFi nicht verbunden.");
        screen->drawFastHLine(0, 11, 128, WHITE);
        screen->setCursor(0, 15);
        screen->println("URL:");
        moveCursor(0, 3);
        screen->println(get_ip_url());
        moveCursor(0, 3);
        screen->println("oder");
        moveCursor(0, 4);
        screen->print("http://");
        screen->print(local_url);
        screen->println(".local");
    }    

    // => Set the default screen when in STATION mode.
    void Screen::showDefaultScreen() {
        // Draw compass
        drawCompass(SCREEN_HALF_WIDTH + 8, SCREEN_HALF_HEIGHT, 30, WHITE);

        // Angle label
        screen->setCursor(0, 0);
        screen->printf("%5.0f", round(rotor_ctrl.rotor.last_angle));
        screen->write(0xF8); //° (degree char)

        // Angular speed label
        screen->setCursor(0, SCREEN_HEIGHT - 7);
        screen->printf("%5.1f", rotor_ctrl.angular_speed);
        screen->write(0xF8); //° (degree char)
        screen->print("/s");

        // Sidebar
        // -------
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
        
        //const uint8_t PROGMEM bits [] = {0xF0, 0xC3, 0xC0};
        //screen->drawBitmap(114, 54, bits, 10, 2, BLACK);
    }

    // => Set screen for firmware update
    void Screen::showUpdateScreen() {
        setFullscreenText("Updating Firmware");
        uint16_t txt_w, txt_h;
        getTextDimensions("100%", &txt_w, &txt_h);
        setCenteredTextCursor(txt_w, txt_h);
        moveCursor(0, 16);
        screen->print(update_progress);
        screen->print("%");
    }

    // => Set an alert message to be shown full screen for a few seconds
    void Screen::setAlert(const String &txt) {
        alert_txt = txt;
        alert_timer->start();
    }

    // => Set an alert message and show it on  the screen immediatly
    void Screen::setAlertImmediatly(const String &txt) {
        alert_txt = txt;
        alert_timer->start();
        clearScreen();
        showFullscreenAlert();
        screen->display();
    }

    // => Main draw function, to be called from main loop
    void Screen::update() {
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
        if (alert_txt != "" && !updating_firmware) {
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
            if (updating_firmware) {
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
