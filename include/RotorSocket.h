#ifndef ROTORSOCKET_H
#define ROTORSOCKET_H

#include <ESPAsyncWebServer.h>

#define MSG_ID_ROTOR "ROTOR"
#define MSG_ID_CALIBRATION "CALIBRATION"
#define MSG_ID_SETTINGS "SETTINGS"
#define MSG_ID_FAVORITES "FAVORITES"
#define MSG_ID_LOCK "LOCK"

// Expose global socket instance
extern AsyncWebSocket websocket;

namespace RotorSocket {
  // Number of connected socket clients
  extern uint8_t clients_connected;

  // => Initialise websocket (add event handler)
  void initWebsocket();
}

#endif //ROTORSOCKET_H
