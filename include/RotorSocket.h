#ifndef ROTORSOCKET_H
#define ROTORSOCKET_H

#include <ESPAsyncWebServer.h>

namespace RotorSocket {
  // Number of connected socket clients
  extern int clients_connected;

  // => Initialise websocket (add event handler)
  void initWebsocket();
}

// Expose global socket instance
extern AsyncWebSocket websocket;

#endif //ROTORSOCKET_H
