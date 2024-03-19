#ifndef ROTORSOCKET_H
#define ROTORSOCKET_H

#include <ESPAsyncWebServer.h>

// Expose global socket instance
extern AsyncWebSocket websocket;

namespace RotorSocket {
  // Number of connected socket clients
  extern int clients_connected;

  // => Initialise websocket (add event handler)
  void initWebsocket();
}

#endif //ROTORSOCKET_H
