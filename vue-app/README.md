# RotorControl UI
This is the Vue.js based web UI, hosted by the ESP32 that runs the RotorControl firmware.

## Project Setup
Use Docker to run the development server and to build the app.

### Development Server
Run development server:
```sh
docker compose up dev
```
For your development UI to be able to connect to an ESP32 running the RotorControl firmware, change the `VITE_SOCKET_GATEWAY` environment variable in the `.env` file to point to your ESP32's IP and port.
### Build
To build the app into a single gzipped file, use the following command. This also creates a C++ header file `AppIndex.h` which includes the gzipped app as a byte array. This file will automatically be copied to `../lib/App/AppIndex.h` after the build process, where it will be included in the ESP32 firmware when compiled.
```sh
docker compose up build
```

### Demo Mode
Use the following commands to serve/build the app in a demo mode, which disables these features:
- disconnecting from the network
- OTA firmware updates

This is intended to be used when the RotorControl ESP32 is connected to another ESP32 running [RotorEmulator](https://github.com/stefan-wr/rotor-emulator).
```sh
# Run development server in demo mode
docker compose up dev-demo

# Build app in demo mode
docker compose up demo
```
