#include "web_config.h"
#include <WebServer.h>
#include <WiFi.h>
#include "rtsp_server.h"    // For getRTSPUrl()
#include "onvif_server.h"   // For ONVIF URL if needed
#include "motion_detection.h" // For motion_detected()
#include <FS.h>
#include <SPIFFS.h> // or LittleFS.h

WebServer webConfigServer(80);

void web_config_start() {
    // Serve static files (HTML/JS/CSS) from SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS/LittleFS Mount Failed");
        return;
    }
    webConfigServer.serveStatic("/", SPIFFS, "/index.html").setDefaultFile("index.html");

    // === API ENDPOINTS ===
    webConfigServer.on("/api/status", HTTP_GET, []() {
        String json = "{";
        json += "\"status\":\"Online\",";
        json += "\"rtsp\":\"" + getRTSPUrl() + "\",";
        json += "\"onvif\":\"http://" + WiFi.localIP().toString() + ":8000/onvif/device_service\",";
        json += "\"motion\":" + String(motion_detected() ? "true" : "false");
        json += "}";
        webConfigServer.send(200, "application/json", json);
    });

    webConfigServer.on("/api/config", HTTP_POST, []() {
        // Parse JSON and apply camera settings (resolution, brightness, etc.)
        webConfigServer.send(200, "application/json", "{\"ok\":1}");
    });

    webConfigServer.on("/api/record", HTTP_POST, []() {
        // Start/stop SD recording logic
        webConfigServer.send(200, "application/json", "{\"ok\":1}");
    });

    webConfigServer.on("/api/reboot", HTTP_POST, []() {
        webConfigServer.send(200, "application/json", "{\"ok\":1}");
        ESP.restart();
    });

    webConfigServer.on("/api/factory_reset", HTTP_POST, []() {
        // Reset settings logic
        webConfigServer.send(200, "application/json", "{\"ok\":1}");
        ESP.restart();
    });

    // === STREAM ENDPOINT ===
    webConfigServer.on("/stream", HTTP_GET, []() {
        WiFiClient client = webConfigServer.client();
        String response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
        client.print(response);

        while (client.connected()) {
            camera_fb_t *fb = esp_camera_fb_get();
            if (!fb) continue;
            client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
            client.write(fb->buf, fb->len);
            client.print("\r\n");
            esp_camera_fb_return(fb);
            delay(100); // ~10 fps
        }
    });

    // Optional: Snapshot endpoint
    webConfigServer.on("/snapshot", HTTP_GET, []() {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            webConfigServer.send(500, "text/plain", "Camera Error");
            return;
        }
        webConfigServer.sendHeader("Content-Type", "image/jpeg");
        webConfigServer.send_P(200, "image/jpeg", (char*)fb->buf, fb->len);
        esp_camera_fb_return(fb);
    });

    webConfigServer.begin();
    Serial.println("[INFO] Web config server started.");
}

void web_config_loop() {
    webConfigServer.handleClient();
}
