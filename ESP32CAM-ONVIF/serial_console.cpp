#include "serial_console.h"
#include "config.h"
#include "wifi_manager.h"
#include "camera_control.h"
#include "SD_MMC.h"

void process_command(String cmd) {
    cmd.trim();
    if (cmd.length() == 0) return;
    
    Serial.println("> " + cmd);
    
    if (cmd == "help" || cmd == "?") {
        Serial.println("--- Serial Console Help ---");
        Serial.println("status       : Show system status settings");
        Serial.println("ip           : Show IP address");
        Serial.println("reboot       : Restart device");
        Serial.println("flash on     : Turn Flash LED ON");
        Serial.println("flash off    : Turn Flash LED OFF");
        Serial.println("ls           : List files on SD card");
    } 
    else if (cmd == "status") {
        Serial.println("--- System Status ---");
        Serial.printf("SSID: %s\n", wifiManager.getSSID().c_str());
        Serial.printf("IP: %s\n", wifiManager.getLocalIP().toString().c_str());
        Serial.printf("Uptime: %lu s\n", millis() / 1000);
        Serial.printf("Heap: %u bytes\n", ESP.getFreeHeap());
        Serial.printf("PSRAM: %u bytes\n", ESP.getFreePsram());
        
        if (FLASH_LED_ENABLED) Serial.println("Flash: Enabled");
        else Serial.println("Flash: Disabled");
    }
    else if (cmd == "ip") {
        Serial.println(wifiManager.getLocalIP());
    }
    else if (cmd == "reboot") {
        Serial.println("Rebooting...");
        delay(100);
        ESP.restart();
    }
    else if (cmd == "flash on") {
        set_flash_led(true);
        Serial.println("Flash set to ON");
    }
    else if (cmd == "flash off") {
        set_flash_led(false);
        Serial.println("Flash set to OFF");
    }
    else if (cmd == "ls") {
        if (!SD_MMC.begin()) { // Re-init if needed or check status logic
             // Actually, SD_MMC.begin should handle multiple calls or use logic from sd_recorder
             // But simple ls on root:
        }
        File root = SD_MMC.open("/");
        if (!root) {
            Serial.println("Failed to open root");
            return;
        }
        File file = root.openNextFile();
        while (file) {
             Serial.printf("  %s (%u bytes)\n", file.name(), file.size());
             file = root.openNextFile();
        }
    }
    else {
        Serial.println("Unknown command. Type 'help'.");
    }
}

void serial_console_loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        process_command(input);
    }
}
