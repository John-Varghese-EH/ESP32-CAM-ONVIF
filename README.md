# ESP32-CAM ONVIF RTSP Camera

**Professional, Feature-Rich, and Network Camera Firmware for ESP32-CAM**

[![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)  

![ESP32-CAM-ONVIF(Image-Inspiration: ₿itVenturesUSA)](/ESP32-CAM-ONVIF.jpg)  

Transform your affordable ESP32-CAM module into a powerful ONVIF-compatible network camera, ready for integration with professional NVR/DVR systems (Hikvision, Dahua, and more). This firmware provides RTSP streaming, ONVIF discovery, and a roadmap for advanced features like web configuration, SD card recording, and motion detection.  

> [!NOTE]
> **🚧 Work in Progress:**  
> This project is evolving rapidly. Contributions, feedback, and feature requests are welcome!  
> -*Star the repo and join the project!*

---

## Features

- 📡 **ONVIF Discovery:**  
  Appears as a discoverable camera on ONVIF-compatible NVRs/DVRs for easy integration.
- 🎥 **RTSP Streaming (MJPEG):**  
  Real-time video streaming for live view and recording.
- ⚡ **Lightweight:**  
  Optimized for ESP32-CAM’s limited resources.
- 🛠️ **Easy Setup:**  
  Simple Wi-Fi configuration (web-based setup coming soon).
- 🌐 **Web Configuration Interface:**  
  *(Planned)* Configure camera, Wi-Fi, and storage via browser.
- **PTZ Control**: Support for Pan/Tilt servos via ONVIF.
- **Flash Control**: Toggle Flash LED via ONVIF "Day/Night" mode.
- **Robust Recording**: Loop recording to SD card even without WiFi.
- **Static IP**: Configurable static IP for stable DVR connections.
- **Web Config**: Manage settings via browser.
- 🗂️ **SD Card Recording:**  
  *(Planned)* Record video directly to microSD card.
- ↔️ **Motion Detection:**  
  *(Planned)* Basic motion detection for event-based recording.
- 🔐 **Secure Credential Storage:**  
  *(Planned)* Store Wi-Fi credentials securely in SPIFFS.
- 🌏 **Access Point Fallback:**  
  *(Planned)* Automatically creates an AP if unable to connect to Wi-Fi.
- 🔒 **Open Source:**  
  MIT-licensed for personal and commercial use.

---

## Hardware Requirements

- **ESP32-CAM board** (AI-Thinker or compatible)
- **MicroSD card** (optional, for recording)
- **5V power supply**
- **FTDI programmer/adapter** (for initial flashing)

---

## Software Dependencies

- **Arduino IDE** or **PlatformIO**
- **ESP32 Arduino Core**
- **Required Libraries:**
  - ArduinoJson
  - ESP32 Camera Driver
  - SPIFFS file system

---

## Installation

### PlatformIO (VS Code) - Recommended

1. **Install Visual Studio Code** and the **PlatformIO** extension.
2. **Clone this repository** or download the zip.
3. **Open the folder** in VS Code. PlatformIO should automatically detect the project.
4. **Build and Upload**:
   - The included `platformio.ini` is already configured with the correct dependencies and board settings.
   - Click the PlatformIO icon > Project Tasks > esp32cam > Upload.

### Arduino IDE

1. **Install Arduino IDE** from [arduino.cc](https://www.arduino.cc/)
2. **Add ESP32 board support:**
   - File > Preferences > Add `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` to "Additional Board Manager URLs"
   - Tools > Board > Boards Manager > Search "ESP32" > Install.
3. **Install required libraries:**
   - **Micro-RTSP**: Do NOT install the default library from the manager. instead, download [this fork](https://github.com/geeksville/Micro-RTSP) as a ZIP and add it via Sketch > Include Library > Add .ZIP Library.
   - **ArduinoJson**: Tools > Manage Libraries > Search and install "ArduinoJson".
4. **Select Board/Port:**
   - Board: AI Thinker ESP32-CAM
5. **Upload Firmware** using an FTDI adapter (GPIO0 to GND).

---

## Quick Start

1. **Clone this repository**
2. **Edit Wi-Fi credentials** in the source code (web setup coming soon)
3. **Flash to your ESP32-CAM**
4. **Connect your NVR/DVR** and discover the camera via ONVIF, or add the RTSP stream manually:
```
rtsp://[camera-ip]:554/mjpeg/1
```

---

## Usage

- Access the RTSP stream using compatible NVR/DVR software or VLC.
- *(Planned)* Access the web interface for live view, configuration, and SD card management.
- *(Planned)* On first boot, ESP32-CAM will create an access point ("ESP32-CAM-ONVIF") for initial setup.

---

## Compatibility

- **Hardware Support:** ESP32-CAM (AI-Thinker or compatible)
- **NVR/DVR Compatibility:** Hikvision, Dahua, and most ONVIF-compliant recorders (MJPEG stream)
- **Limitations:** MJPEG only (no H.264); some recorders may require H.264 for recording

---

## Project Structure

| File/Folder             | Description                                |
|-------------------------|--------------------------------------------|
| `ESP32-CAM-ONVIF.ino`   | Main firmware sketch                       |
| `camera_control.*`      | Camera initialization and settings         |
| `rtsp_server.*`         | RTSP streaming implementation              |
| `onvif_server.*`        | ONVIF protocol implementation              |
| `web_config.*`          | *(Planned)* Web interface                  |
| `wifi_manager.*`        | *(Planned)* Wi-Fi setup and AP fallback    |
| `sd_recorder.*`         | *(Planned)* SD card recording              |
| `motion_detection.*`    | *(Planned)* Motion detection               |
| `data/`                 | *(Planned)* Web UI files (HTML, CSS, JS)   |

---

## Roadmap

- [x] ONVIF WS-Discovery responder
- [x] `/onvif/device_service` endpoint (GetStreamUri, GetCapabilities)
- [x] RTSP video streaming (MJPEG)
- [ ] Web-based configuration interface
- [ ] SD card recording and management
- [ ] Motion detection
- [ ] Advanced ONVIF features (profiles, device info, etc.)
- [ ] Secure credential storage (SPIFFS)
- [ ] Access point fallback for Wi-Fi setup

---

## Screenshots

*Coming soon!*

---

## Contributing

Pull requests, issues, and feature suggestions are welcome!  
See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

---

  [![Buy me a Coffee](https://img.shields.io/badge/Buy_Me_A_Coffee-FFDD00?style=for-the-badge&logo=buy-me-a-coffee&logoColor=black)](https://buymeacoffee.com/CyberTrinity)
  [![Patreon](https://img.shields.io/badge/Patreon-F96854?style=for-the-badge&logo=patreon&logoColor=white)](https://patreon.com/CyberTrinity)
  [![Sponsor](https://img.shields.io/badge/sponsor-30363D?style=for-the-badge&logo=GitHub-Sponsors&logoColor=#white)](https://github.com/sponsors/John-Varghese-EH)

---


## License

This project is licensed under the [MIT License](LICENSE).

---

## Acknowledgments

- WiFi scanning functionality added by BitVentures USA
- Micro-RTSP for RTSP streaming on ESP32
- ONVIF community for protocol documentation and inspiration
- Thanks to all contributors and the open-source community!

---

*Stay tuned for updates, new features, and documentation as the project evolves! Star the repo to follow progress and contribute to making ESP32-CAM ONVIF the ultimate DIY network camera solution.*
