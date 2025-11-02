# Addressable LED Strip Animations

[![Build (master)](https://github.com/sedlakmate/digital-rgb-led-strip/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/szedlakmate/digital-rgb-led-strip/actions/workflows/build.yml)

## About This Project

This is the personal love-project of **Máté Sedlák** ([@sedlakmate](https://github.com/sedlakmate)).

The goal is to create a highly customizable, interactive LED strip controller for addressable LEDs, with a focus on modularity, real-time control, and fun experimentation. In the future, I plan to add support for controlling the LEDs with phones and other smart devices.

## Overview

This project controls a digital (addressable) LED strip, such as WS2812B (NeoPixel), to display a variety of animated patterns. It is designed for Arduino Mega and supports real-time parameter control via potentiometer knobs and optional interaction with an ultrasonic distance sensor.

## Features

### Core Features
- Multiple color palettes and smooth animation effects
- Real-time adjustment of brightness, animation speed (BPM), and pattern length via analog knobs
- Optional ultrasound sensor for interactive effects (e.g., proximity-based brightness)
- Modular configuration via `config.h` and local overrides in `config_override.h`
- Debug logging (enable/disable in config)

### ESP32-Specific Features
- **WiFi Management**: Auto-connect with WiFiManager library and captive portal for easy setup
- **mDNS Discovery**: Device broadcasts as `ledstrip.local` for easy network discovery
- **Web Server**: Control LEDs remotely via REST API or web interface
- **Request Logging**: All HTTP requests echoed to Serial for debugging
- **Remote Control**: Adjust brightness, BPM, palette, and wave length via HTTP endpoints
- **Status Monitoring**: Real-time status display via web interface

## Hardware Requirements

### Supported Microcontrollers
- Arduino Mega 2560 (or compatible)
- ESP32 (any variant with WiFi support)

### Common Components
- WS2812B (NeoPixel) or compatible addressable LED strip
- Potentiometers (for brightness, BPM, and pattern length control; optional)
- HC-SR04 or compatible ultrasonic distance sensor (optional)

## Setup

1. **Install Dependencies**

   **Required for all boards:**
   - [FastLED](https://github.com/FastLED/FastLED) (avoid version 3.9.18, which is incompatible)
   - [NewPing](https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home)

   **Additional dependencies for ESP32 (WiFi/Web features):**
   - [WiFiManager](https://github.com/tzapu/WiFiManager) by tzapu
   - [ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer) (v3.6.0+)
   - [AsyncTCP](https://github.com/ESP32Async/AsyncTCP) (v3.3.2+)

2. **Configure Hardware**
   - Edit `config.h` to set pin assignments and animation defaults.
   - For local or per-device changes, copy `config_override_template.h` to `config_override.h` and override only the needed settings.
   - See comments in `config.h` and module headers for wiring details.
   - For ESP32, configure WiFi settings in `config.h` or `config_override.h` (AP name, password, timeout).

3. **Build and Upload**
   - Open the project in the Arduino IDE or use Arduino CLI.
   - Select the correct board (Arduino Mega 2560 or ESP32).
   - Compile and upload the sketch.

## Usage

### Basic Control (All Boards)
- Adjust the connected knobs to control brightness, animation speed (BPM), and pattern length in real time.
- If an ultrasound sensor is connected, approach or move away to interactively change effects (e.g., brightness).
- Enable debug output by setting `#define DEBUG 1` in your `config_override.h`.

### ESP32 WiFi Setup
1. **First Boot**: Device creates a WiFi access point named `LED-Strip-Setup` (password: `ledstrip123`)
2. Connect to this AP with your phone or computer
3. A captive portal opens automatically (or navigate to `192.168.4.1`)
4. Select your WiFi network and enter the password
5. Device saves credentials and connects automatically on future boots

### Web Interface (ESP32 Only)
- Once connected to WiFi, the device broadcasts its presence on the network via mDNS
- Access the device at **`http://ledstrip.local`** (or use the IP address shown in Serial monitor)
- The device is discoverable as "LED Strip Controller" on your network
- **Web Dashboard**: View current LED settings and system status
- **REST API Endpoints**:
  - `GET /api/status` - Get current settings as JSON
  - `POST /api/led` - Control LED parameters (supports multiple parameters in one request)
    - Parameters: `brightness` (0-255), `bpm`, `palette` (0-based index), `wavelength`
    - Examples:
      - `/api/led?brightness=200`
      - `/api/led?brightness=200&bpm=10`
      - `/api/led?palette=3&wavelength=2.5`
- **Request Logging**: All HTTP requests are echoed to Serial with full details (method, URL, headers, parameters)

## File Structure

- `Digital_RGB_LED.ino` — Main entry point
- `animation.*` — Animation logic
- `palette.*` — Color palette definitions
- `knob.h` — Analog knob mapping utilities
- `ultrasound.*` — Ultrasound sensor driver
- `debug.h` — Debug logging utilities
- `config.h` — Main configuration
- `config_override_template.h` — Template for local overrides
- `wifi_manager.*` — WiFi connection management (ESP32 only)
- `web_server.*` — Async web server and REST API (ESP32 only)

## Troubleshooting

### mDNS Not Resolving (ESP32)

If `ledstrip.local` doesn't resolve:

1. **Check mDNS support on your system:**
   - **Linux**: Install `avahi-daemon` and ensure it's running: `sudo systemctl start avahi-daemon`
   - **macOS**: mDNS (Bonjour) is built-in and should work automatically
   - **Windows**: Windows 10+ has native mDNS support, but it may require a reboot after first setup

2. **Test mDNS resolution:**
   ```bash
   # Linux/macOS
   ping ledstrip.local

   # Or check with getent (Linux)
   getent hosts ledstrip.local
   ```

3. **Check device logs:** Enable debug mode and verify mDNS started successfully in the Serial monitor

4. **Use IP address as fallback:** The device IP is always displayed in the Serial monitor

5. **Ensure devices are on the same network:** mDNS only works within the same local network/subnet

## Notes

- Only define the pins/features you use in your configuration; unused features are automatically stubbed out.
- Do not commit `config_override.h` to version control (see `.gitignore`).
- WiFi and web server features are automatically enabled for ESP32 builds and disabled for Arduino Mega builds.
- The code is fully backward compatible with Arduino Mega 2560.
- The mDNS hostname can be changed in `config.h` or `config_override.h` by modifying `MDNS_HOSTNAME`.

## License

See [LICENSE](LICENSE) for details.
