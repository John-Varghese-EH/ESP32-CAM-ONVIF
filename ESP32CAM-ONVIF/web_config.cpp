#include "web_config.h"
#include <WebServer.h>
#include <WiFi.h>
#include "rtsp_server.h"       // For getRTSPUrl()
#include "onvif_server.h"      // For ONVIF URL if needed
#include "motion_detection.h"  // For motion_detected()
#include <FS.h>
#include <SPIFFS.h>
#include <SD_MMC.h>
#include <ArduinoJson.h>
#include "esp_camera.h"

WebServer webConfigServer(80);

// --- Helper: HTTP Basic Auth ---
const char* WEB_USER = "admin";
const char* WEB_PASS = "esp123";

bool isAuthenticated(WebServer &server) {
    if (!server.authenticate(WEB_USER, WEB_PASS)) {
        server.requestAuthentication();
        return false;
    }
    return true;
}

void web_config_start() {
    // Mount SPIFFS for static files
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS/LittleFS Mount Failed");
        return;
    }
    // Protect static files with authentication
    webConfigServer.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setAuthentication(WEB_USER, WEB_PASS);

    // === API ENDPOINTS ===
    webConfigServer.on("/api/status", HTTP_GET, []() {
        if (!isAuthenticated(webConfigServer)) return;
        String json = "{";
        json += "\"status\":\"Online\",";
        json += "\"rtsp\":\"" + getRTSPUrl() + "\",";
        json += "\"onvif\":\"http://" + WiFi.localIP().toString() + ":8000/onvif/device_service\",";
        json += "\"motion\":" + String(motion_detected() ? "true" : "false");
        json += "}";
        webConfigServer.send(200, "application/json", json);
    });

    // --- Change Camera Settings ---
    webConfigServer.on("/api/config", HTTP_POST, []() {
        if (!isAuthenticated(webConfigServer)) return;
        StaticJsonDocument<256> doc;
        DeserializationError err = deserializeJson(doc, webConfigServer.arg("plain"));
        if (err) {
            webConfigServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }
        sensor_t * s = esp_camera_sensor_get();
        if (doc.containsKey("brightness")) s->set_brightness(s, doc["brightness"]);
        if (doc.containsKey("contrast"))   s->set_contrast(s, doc["contrast"]);
        if (doc.containsKey("saturation")) s->set_saturation(s, doc["saturation"]);
        if (doc.containsKey("flip"))       s->set_vflip(s, doc["flip"] ? 1 : 0);
        if (doc.containsKey("rotate"))     s->set_hmirror(s, doc["rotate"] ? 1 : 0);
        if (doc.containsKey("resolution")) {
            String res = doc["resolution"].as<String>();
            if (res == "VGA")       s->set_framesize(s, FRAMESIZE_VGA);
            else if (res == "QVGA") s->set_framesize(s, FRAMESIZE_QVGA);
            else if (res == "CIF")  s->set_framesize(s, FRAMESIZE_CIF);
        }
        webConfigServer.send(200, "application/json", "{\"ok\":1}");
    });

    // --- SD Card File List ---
    webConfigServer.on("/api/sd/list", HTTP_GET, []() {
        if (!isAuthenticated(webConfigServer)) return;
        String json = "[";
        File root = SD_MMC.open("/");
        File file = root.openNextFile();
        bool first = true;
        while(file){
            if (!first) json += ",";
            json += "\"" + String(file.name()) + "\"";
            file = root.openNextFile();
            first = false;
        }
        json += "]";
        webConfigServer.send(200, "application/json", json);
    });

    // --- SD Card Download ---
    webConfigServer.on("/api/sd/download", HTTP_GET, []() {
        if (!isAuthenticated(webConfigServer)) return;
        if (!webConfigServer.hasArg("file")) {
            webConfigServer.send(400, "text/plain", "Missing file param");
            return;
        }
        String filename = "/" + webConfigServer.arg("file");
        File file = SD_MMC.open(filename, "r");
        if (!file) {
            webConfigServer.send(404, "text/plain", "File not found");
            return;
        }
        webConfigServer.streamFile(file, "application/octet-stream");
        file.close();
    });

    // --- SD Card Delete ---
    webConfigServer.on("/api/sd/delete", HTTP_POST, []() {
        if (!isAuthenticated(webConfigServer)) return;
        StaticJsonDocument<128> doc;
        DeserializationError err = deserializeJson(doc, webConfigServer.arg("plain"));
        if (err || !doc.containsKey("file")) {
            webConfigServer.send(400, "application/json", "{\"error\":\"Invalid request\"}");
            return;
        }
        String filename = "/" + doc["file"].as<String>();
        if (SD_MMC.remove(filename)) {
            webConfigServer.send(200, "application/json", "{\"ok\":1}");
        } else {
            webConfigServer.send(404, "application/json", "{\"error\":\"Delete failed\"}");
        }
    });

    // --- SD Recording Trigger (stub) ---
    webConfigServer.on("/api/record", HTTP_POST, []() {
        if (!isAuthenticated(webConfigServer)) return;
        // Start/stop SD recording logic here
        webConfigServer.send(200, "application/json", "{\"ok\":1}");
    });

    // --- Reboot ---
    webConfigServer.on("/api/reboot", HTTP_POST, []() {
        if (!isAuthenticated(webConfigServer)) return;
        webConfigServer.send(200, "application/json", "{\"ok\":1}");
        ESP.restart();
    });

    // --- Factory Reset (stub) ---
    webConfigServer.on("/api/factory_reset", HTTP_POST, []() {
        if (!isAuthenticated(webConfigServer)) return;
        // Reset settings logic here
        webConfigServer.send(200, "application/json", "{\"ok\":1}");
        ESP.restart();
    });

    // === STREAM ENDPOINT ===
    webConfigServer.on("/stream", HTTP_GET, []() {
        if (!isAuthenticated(webConfigServer)) return;
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

    // --- Snapshot endpoint ---
    webConfigServer.on("/snapshot", HTTP_GET, []() {
        if (!isAuthenticated(webConfigServer)) return;
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
