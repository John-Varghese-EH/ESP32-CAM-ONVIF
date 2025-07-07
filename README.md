# ESP32-CAM | ONVIF | DVR/NVR Stream and Recording | Ultimate Feature Packed Firmware 
## ESP32-CAM ONVIF RTSP Camera 

[![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

---

> [!NOTE]
> **🚧 Work in Progress:**  
> The project is still evolving! Help make it better and faster—contributions, feedback, and ideas are warmly welcome.  
> *Star the repo and join the project!*

---

## Overview

**ESP32-CAM ONVIF RTSP Camera** is an open-source firmware project that transforms your affordable ESP32-CAM module into a network camera compatible with professional NVR/DVR systems, including Hikvision, Dahua, and other ONVIF-compliant solutions.

This project brings together:
- **RTSP streaming** (MJPEG) for real-time video
- **Minimal ONVIF support** for device discovery, stream URI reporting, and basic integration with security recorders

The goal is to make ESP32-CAM modules plug-and-play with mainstream video surveillance systems, while remaining lightweight and efficient.

---

## Features

- 📡 **ONVIF Discovery:** Your ESP32-CAM will appear as a discoverable camera on ONVIF-compatible NVRs/DVRs.
- 🎥 **RTSP Streaming:** Real-time MJPEG streaming for live view and recording.
- ⚡ **Lightweight:** Designed for ESP32-CAM’s limited resources.
- 🛠️ **Easy Setup:** Simple Wi-Fi configuration and deployment.
- 🔒 **Open Source:** MIT-licensed for personal and commercial use.

---

## Roadmap

- [x] Minimal ONVIF WS-Discovery responder
- [x] ONVIF `/onvif/device_service` endpoint (GetStreamUri, GetCapabilities)
- [x] RTSP video streaming (MJPEG)
- [ ] Web-based configuration interface
- [ ] Optional SD card recording
- [ ] Motion detection (future)
- [ ] Support for more ONVIF features (profiles, device info, etc.)

---

## Quick Start

1. **Clone this repository**
2. **Edit Wi-Fi credentials** in the source code
3. **Flash to your ESP32-CAM** using Arduino IDE or PlatformIO
4. **Connect your NVR/DVR** and discover the camera via ONVIF, or add the RTSP stream manually

---

## Compatibility

- **Tested hardware:** AI-Thinker ESP32-CAM
- **NVR/DVR compatibility:** Hikvision, Dahua, and most ONVIF-compliant recorders (MJPEG stream)
- **Limitations:** MJPEG only (no H.264); some recorders may require H.264 for recording

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

- [Micro-RTSP](https://github.com/geeksville/Micro-RTSP) for RTSP streaming on ESP32
- ONVIF community for protocol documentation and inspiration

---

> **Stay tuned for updates, features, and documentation as the project evolves!**

