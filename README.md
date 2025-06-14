# ESP32-CAM+ONVIF: Together at last!

A professional firmware implementation for ESP32-CAM that provides ONVIF compatibility, RTSP streaming, and a web-based configuration interface with WiFi network scanning capabilities.

![ESP32-CAM](https://esphome.io/_images/ESP32-CAM_board.jpg)

## Features

- **ONVIF Protocol Support**: Enables integration with NVR/DVR systems and surveillance software
- **RTSP MJPEG Streaming**: Stream video on port 554
- **WiFi Network Scanning**: Scan for available networks and connect without hardcoding credentials
- **Web Configuration Interface**: Configure camera settings through a browser
- **SD Card Recording**: Save video recordings directly to microSD card
- **Motion Detection**: Basic motion detection capability
- **Credential Storage**: Securely store WiFi credentials in SPIFFS
- **Access Point Fallback**: Automatically creates an AP if unable to connect to WiFi

## Hardware Requirements

- ESP32-CAM board (AI-Thinker or compatible)
- MicroSD card (optional, for recording)
- 5V power supply
- FTDI programmer/adapter (for initial flashing)

## Software Dependencies

- Arduino IDE
- ESP32 Arduino Core
- Required Libraries:
  - ArduinoJson
  - ESP32 Camera Driver
  - SPIFFS file system

## Installation

### Setting Up the Arduino IDE

1. Install Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
2. Add ESP32 board support:
   - Go to **File > Preferences**
   - Add `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` to the "Additional Board Manager URLs" field
   - Go to **Tools > Board > Boards Manager**
   - Search for "ESP32" and install the latest version

3. Install required libraries:
   - Go to **Tools > Manage Libraries**
   - Search for and install "ArduinoJson"

### Flashing the Firmware

1. Connect the ESP32-CAM to your computer using an FTDI adapter:
   - FTDI GND -> ESP32-CAM GND
   - FTDI 5V/3.3V -> ESP32-CAM 5V
   - FTDI TX -> ESP32-CAM RX
   - FTDI RX -> ESP32-CAM TX
   - Connect GPIO0 to GND (puts ESP32-CAM in flash mode)

2. Select the correct board and port:
   - **Tools > Board > ESP32 Arduino > AI-Thinker ESP32-CAM**
   - **Tools > Port > [Select your FTDI port]**

3. Upload the firmware:
   - Click the Upload button or use **Sketch > Upload**
   - After uploading, disconnect GPIO0 from GND and reset the ESP32-CAM

## Usage

### Setting Up Platform.IO

1. Install Visual Studio Code from [code.visualstudio.com](https://code.visualstudio.com/)
2. Install the PlatformIO extension:
   - Open VS Code
   - Go to Extensions (or press Ctrl+Shift+X)
   - Search for "PlatformIO IDE"
   - Click Install

3. Create a new project:
   - Click the PlatformIO icon in the sidebar
   - Select "New Project"
   - Enter a project name (e.g., "ESP32-CAM-ONVIF")
   - Select "AI Thinker ESP32-CAM" as the board
   - Select "Arduino" as the framework
   - Choose a location for your project
   - Click "Finish"

4. Configure your project:
   - Open the `platformio.ini` file in the project root
   - Add the following configuration:

```ini
[env:esp32cam]
platform = espressif32
board = esp32cam
framework = arduino
monitor_speed = 115200
lib_deps =
    bblanchon/ArduinoJson @ ^6.21.3
    # Add other dependencies as needed
upload_speed = 921600
build_flags = 
    -DCORE_DEBUG_LEVEL=5
```

5. Import the project files:
   - Copy all files from the ESP32-CAM+ONVIF repository to the `src` directory of your PlatformIO project
   - Copy the files from the `data` directory to the `data` directory in your PlatformIO project

### Flashing with PlatformIO

1. Connect your ESP32-CAM as described in the Arduino section (with GPIO0 connected to GND)
2. Click the PlatformIO upload button or run the Upload task
3. To upload the SPIFFS data (web interface files):
   - Click on the PlatformIO tasks menu
   - Select "Upload File System image"
4. Disconnect GPIO0 from GND and reset the ESP32-CAM

### First-Time Setup

1. On first boot, the ESP32-CAM will create an access point named "ESP32-CAM+ONVIF"
2. Connect to this access point with password "esp32cam"
3. Open a web browser and navigate to `http://192.168.4.1`
4. Log in with the default credentials:
   - Username: `admin`
   - Password: `esp123`

### WiFi Configuration

1. Once logged in, click on the "WiFi" tab
2. Click "Scan Networks" to see available WiFi networks
3. Select your network from the list
4. Enter your WiFi password when prompted
5. The ESP32-CAM will connect to your network and restart
6. After restart, access the camera using its new IP address on your network

### Using the Camera

1. Access the web interface by entering the ESP32-CAM's IP address in a browser
2. The interface provides tabs for:
   - **Live**: View the live camera stream
   - **Camera**: Adjust camera settings (resolution, quality, etc.)
   - **WiFi**: Configure WiFi connections
   - **SD Card**: Manage recordings on the SD card
   - **System**: View system information and reboot/reset options

### Connecting to NVR/DVR Systems

The camera provides ONVIF and RTSP services:
- ONVIF discovery service on port 8000
- RTSP stream available at `rtsp://[camera-ip]:554/mjpeg/1`

Add the camera to your NVR/DVR system using these details.

## Project Structure

- `ESP32-CAM+ONVIF.ino`: Main sketch file
- `camera_control.cpp/.h`: Camera initialization functions
- `rtsp_server.cpp/.h`: RTSP streaming implementation
- `onvif_server.cpp/.h`: ONVIF protocol implementation
- `web_config.cpp/.h`: Web interface implementation
- `wifi_manager.cpp/.h`: WiFi scanning and connection management
- `sd_recorder.cpp/.h`: SD card recording functionality
- `motion_detection.cpp/.h`: Motion detection implementation
- `data/`: Web interface files (HTML, CSS, JavaScript)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Original implementation by J0X
- WiFi scanning functionality added by BitVentures USA
- Thanks to the ESP32, Arduino, and PlatformIO communities

## Contributing

Contributions to improve the firmware are welcome. Please feel free to submit a pull request or open an issue to discuss proposed changes.