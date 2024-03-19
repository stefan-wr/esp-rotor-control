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
#include <Firmware.h>           // Exposes Global: firmware
#include <RotorServer.h>        // Exposes Global: server
#include <RotorSocket.h>        // Expose Global: websocket

namespace Screen {

    const unsigned char rotate_right_icon[] = {0,0,0,0,3,224,7,240,14,56,28,28,24,12,24,12,
                                               24,76,28,220,12,248,0,240,0,252,0,248,0,0,0,0};
    const unsigned char rotate_left_icon[] = {0,0,0,0,3,224,7,240,14,56,28,28,24,12,24,12,
                                              25,12,29,156,15,152,7,128,31,128,15,128,0,0,0,0};

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
        alert_txt.reserve(96);
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

    // => Toggle through available screens
    void Screen::toggleScreens() {
        if (page >= N_PAGES - 1) {
            page = 0;
        } else {
            page++;
        }
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
    // @param txt: some String
    // @param txt_w: pointer to store text width at
    // @param txt_w: pointer to store text height at
    void Screen::getTextDimensions(const String &txt, uint16_t *txt_w, uint16_t  *txt_h) {
        int16_t cx, cy;
        screen->getTextBounds(txt, 0, 0, &cx, &cy, txt_w, txt_h);  
    }

    // => Set cursor so that text to be printed next is centered on full screen.
    // Needs the width and height of the text on screen from getTextDimenions().
    // @param txt_w: width of text
    // @param txt_h: height of text
    void Screen::setCenteredTextCursor(const uint16_t &txt_w, const uint16_t &txt_h) {
        screen->setCursor((SCREEN_WIDTH - txt_w) / 2, (SCREEN_HEIGHT - txt_h) / 2);
    }

    // => Set cursor so that text to be printed next is centered on full screen.
    // @param txt: text to be centered on screen
    void Screen::setCenteredTextCursor(const String &txt) {
        uint16_t txt_w, txt_h;
        getTextDimensions(txt, &txt_w, &txt_h);
        setCenteredTextCursor(txt_w, txt_h);
    }

    // => Set a horizontally centered text
    // @param cy: cursor Y position to set text at
    // @param txt: text to be centered horizontally
    void Screen::setHorizontallyCenteredText(const uint16_t cy, const String &txt, const uint16_t cx_offset) {
        uint16_t txt_w, txt_h;
        getTextDimensions(txt, &txt_w, &txt_h);
        screen->setCursor((SCREEN_WIDTH - txt_w) / 2 + cx_offset - 1, cy);
        screen->println(txt);
    }

    // => Set a fullscreen text
    void Screen::setFullscreenText(const String &txt) {
        setCenteredTextCursor(txt);
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

    // => Set a title bar on screen
    // @param gap: gap in pixels between text and underline
    // @param icon: number of CP437 symbol to be used as icon, -1 for no icon
    // @param title: title text
    void Screen::setTitleBar(const int &gap, const int icon, const String &title) {
        screen->setCursor(0, 0);
        int cx_offset = 0;

        if (icon >= 0) {
            screen->write(icon);
            cx_offset = 4;
        }

        setHorizontallyCenteredText(0, title, cx_offset);
        moveCursor(0, gap);
        screen->drawFastHLine(0, screen->getCursorY(), 128, WHITE);
        moveCursor(0, gap + 3);
    }    

    // => Draw compass with radius r, centered at (cx, cy)
    // ---------------------------------------------------
    void Screen::drawCompass(const uint16_t &cx, const uint16_t &cy, const float &r, uint16_t color) {
        // Compass outline
        screen->fillCircle(cx, cy, r, color);
        screen->fillCircle(cx, cy, r - 2, !color);

        // Reticle
        /*
        screen->drawFastHLine(cx - r + 13, cy, 2 * (r - 13) + 1, color);
        screen->drawFastVLine(cx, cy - r + 13, 2 * (r - 13) + 1, color);
        screen->fillCircle(cx, cy, 13, !color);
        */

        // Ticks
        for (float a = 0; a < 360; a += 30 ) {
            compass.tick_sin = sin(a * deg_to_rad_factor);
            compass.tick_cos = cos(a * deg_to_rad_factor);
            compass.tick_x1 = round(cx + compass.tick_sin * r);
            compass.tick_y1 = round(cy - compass.tick_cos * r);
            compass.tick_x2 = round(cx + compass.tick_sin * (r - 4));
            compass.tick_y2 = round(cy - compass.tick_cos * (r - 4));
            screen->drawLine(compass.tick_x2, compass.tick_y2,
                            compass.tick_x1, compass.tick_y1, color);
        }

        // Draw compass needle
        compass.needle_sin = sin(rotor_ctrl.rotor.last_angle_rad);
        compass.needle_cos = cos(rotor_ctrl.rotor.last_angle_rad);
        compass.needle_x1 = round(cx + compass.needle_sin * (r - 5));
        compass.needle_y1 = round(cy - compass.needle_cos * (r - 5));
        compass.needle_x2 = round(cx - compass.needle_sin * (r * 0.4));
        compass.needle_y2 = round(cy + compass.needle_cos * (r * 0.4));
        screen->drawLine(compass.needle_x2, compass.needle_y2,
                         compass.needle_x1, compass.needle_y1, color);

        // Draw compass target indicator
        if (rotor_ctrl.is_auto_rotating) {
            compass.target_sin = sin(rotor_ctrl.auto_rotation_target_rad);
            compass.target_cos = cos(rotor_ctrl.auto_rotation_target_rad);
            compass.target_x1 = round(cx + compass.target_sin * (r - 4));
            compass.target_y1 = round(cy - compass.target_cos * (r - 4));
            compass.target_x2 = round(cx + compass.target_sin * (r * 0.5));
            compass.target_y2 = round(cy - compass.target_cos * (r * 0.5));
            screen->drawLine(compass.target_x2, compass.target_y2,
                             compass.target_x1, compass.target_y1, color);
        }

        // Center dot
        screen->fillCircle(cx, cy, 5, !color);
        screen->drawCircle(cx, cy, 5, color);
        screen->fillCircle(cx, cy, 2, color);

        // Overlap Indicator
        if (rotor_ctrl.rotor.last_angle > 360.0f) {
            screen->fillCircle(cx + r - 3, cy - r + 3, 2, color);
        }
    }

    // => Draw sidebar with additional information
    // -------------------------------------------
    void Screen::drawSidebar() {
        screen->fillRoundRect(SCREEN_WIDTH - 19, 0, 19, 19, 3, WHITE); // 109,0
        screen->fillRoundRect(SCREEN_WIDTH - 19, 22, 19, 42, 3, WHITE); // 109,0
        screen->setTextColor(BLACK);
        
        // Rotation indicators
        if (rotor_ctrl.is_rotating) {
            if (rotor_ctrl.direction) {
                //screen->fillTriangle(114, 5, 122, 9, 114, 13, BLACK);   // Left
                screen->drawBitmap(110, 2, rotate_right_icon, 16, 16, BLACK);
            } else {
                //screen->fillTriangle(114, 9, 122, 5, 122, 13, BLACK);   // Right
                screen->drawBitmap(110, 2, rotate_left_icon, 16, 16, BLACK);
            }
        } else {
            screen->fillRoundRect(114, 5, 9, 9, 1, BLACK);
        }

        // Overlap indicator
        if (rotor_ctrl.rotor.last_angle > 360.0f) {
            screen->setCursor(113, 27);
            screen->print("OL");
        }

        // Auto rotation indicator
        if (rotor_ctrl.is_auto_rotating) {
            screen->setCursor(113, 40);
            screen->print("AR");
        }

        // N of connected clients
        screen->setCursor(116, 52);
        screen->print(RotorSocket::clients_connected);
    }


    // => Set splash-screen
    // --------------------
    void Screen::showSplashScreen() {
        const String txt = "RotorControl";
        uint16_t txt_w, txt_h;
        getTextDimensions(txt, &txt_w, &txt_h);
        setCenteredTextCursor(txt_w, txt_h);
        screen->drawRect(screen->getCursorX() - 5, screen->getCursorY() - 5,
                         txt_w + 8, txt_h + 9, WHITE);
        screen->print(txt);
        moveCursor(0, 18);
        setHorizontallyCenteredText(screen->getCursorY(), version);
    }


    // => Set screen for AP-mode
    // -------------------------
    void Screen::showAPModeScreen() {
        const int gap = 3;
        screen->setCursor(0, 0);
        setCenteredTextCursor("WiFi not connected.");
        moveCursor(0, screen->getCursorY() * -1);
        screen->println("WiFi not connected.");

        moveCursor(0, gap);
        screen->drawFastHLine(screen->getCursorX(), screen->getCursorY(), 128, WHITE);

        moveCursor(0, gap + 2);
        screen->println("IP:");

        moveCursor(0, gap);
        screen->println(WiFi.softAPIP());

        moveCursor(0, gap + 2);
        screen->println("URL:");

        moveCursor(0, 4);
        screen->print("http://");
        screen->print(local_url);
        screen->println(".local");
    }

    // => Set screen for when WiFi is reconnecting
    // -------------------------------------------
    void Screen::showReconnectingScreen() {
        const int gap = 2;
        setTitleBar(gap, 0x21, "WiFi Disconnected");
        setCenteredTextCursor("Reconnecting...");
        moveCursor(0, gap * 2 + 1);
        screen->println("Reconnecting...");
    }

    // => Set network info screen
    // --------------------------
    void Screen::showNetworkScreen() {
        const int gap = 2;
        setTitleBar(gap, 0xF0, "Network & Server");

        if (WiFi.isConnected()) {
            screen->printf("IP: %s\n", WiFi.localIP().toString().c_str());

            moveCursor(0, gap);
            screen->printf("URL: %s.local\n", local_url);

            moveCursor(0, gap);
            screen->printf("Port: %d\n", rotor_server.config.port);

            moveCursor(0, gap);
            screen->setTextWrap(false);
            screen->printf("SSID: %s\n", WiFiFunctions::wifi_config.ssid.c_str());
            screen->setTextWrap(true);

            moveCursor(0, gap);
            screen->printf("RSSI: %d dBm\n", WiFi.RSSI());
        } else {
            screen->println("WiFi not connected.");
        }
    }

    // => Set system info screen
    // -------------------------
    void Screen::showSystemScreen() {
        const int gap = 2;
        setTitleBar(gap, 0xF0, "System");
        
        screen->printf("ID: %s\n", esp_id);

        moveCursor(0, gap);
        screen->print("ADC Status: ");
        if (rotor_ctrl.rotor.getADCStatus()) {
            screen->println("OK");
        } else {
            screen->println("N/A");
        }

        moveCursor(0, gap);
        screen->printf("Free RAM: %.0f kB\n", ((float) ESP.getFreeHeap()) / 1024.0f);

        moveCursor(0, gap);
        screen->printf("Firmware: %s\n", version.c_str());

        moveCursor(0, gap);
        screen->printf("UI Version: %s\n", "0.9.0" );
    }

    // => Set the default screen when in STATION mode.
    // -----------------------------------------------
    void Screen::showDefaultScreen() {
        drawCompass(SCREEN_HALF_WIDTH + 8, SCREEN_HALF_HEIGHT, 30, WHITE);
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
        if (true) {
            // Current Speed
            if (rotor_ctrl.smooth_speed_active) {
                if (rotor_ctrl.current_speed) {
                    screen->printf("S %3d%%", rotor_ctrl.current_speed);
                } else {
                    screen->printf("S %3d%%", 1);
                }
            } else {
                // Max Speed
                if (rotor_ctrl.max_speed) {
                    screen->printf("S %3d%%", rotor_ctrl.max_speed);
                } else {
                    screen->printf("S %3d%%", 1);
                }
            }
        } else {
            // Angular Speed
            screen->printf("R %5.2f/s", rotor_ctrl.angular_speed);
        }
    }


    // => Set screen during firmware update
    // ------------------------------------
    void Screen::showUpdateScreen() {
        setFullscreenText("Updating Firmware");
        moveCursor(0, 16);
        char buffer[4];
        sprintf(buffer, "%d%%", firmware.upload_progress);
        setHorizontallyCenteredText(screen->getCursorY(), (String) buffer);
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
        // If screen is disabled, only clear the screen
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

        if (!in_station_mode) {
            showAPModeScreen();

        } else if (is_reconnecting) {
            showReconnectingScreen();

        } else if (firmware.is_updating) {
            showUpdateScreen();

        } else {
            switch (page) {
                case 0:
                    showDefaultScreen();
                    break;
                case 1:
                    showNetworkScreen();
                    break;
                case 2:
                    showSystemScreen();
                    break;
            }
        }

        // ----------
        
        screen->display();
    }    
}

Screen::Screen screen;
