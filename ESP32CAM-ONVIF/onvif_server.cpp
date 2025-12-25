#include <Arduino.h>
#include "onvif_server.h"
#include "rtsp_server.h"
#include "camera_control.h"
#include <WiFiUdp.h>
#include <WebServer.h>
#include "config.h"

WebServer onvifServer(ONVIF_PORT);
WiFiUDP onvifUDP;

String getCapabilitiesResponse() {
  String ip = WiFi.localIP().toString();
  return
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
    "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
    "<SOAP-ENV:Body>"
    "<tds:GetCapabilitiesResponse>"
    "<tds:Capabilities>"
    "<tds:Media>"
    "<tds:XAddr>http://" + ip + ":" + String(ONVIF_PORT) + "/onvif/device_service</tds:XAddr>"
    "</tds:Media>"
    "</tds:Capabilities>"
    "</tds:GetCapabilitiesResponse>"
    "</SOAP-ENV:Body>"
    "</SOAP-ENV:Envelope>";
}

String getDeviceInfoResponse() {
  return
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
    "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
    "<SOAP-ENV:Body>"
    "<tds:GetDeviceInformationResponse>"
    "<tds:Manufacturer>" DEVICE_MANUFACTURER "</tds:Manufacturer>"
    "<tds:Model>" DEVICE_MODEL "</tds:Model>"
    "<tds:FirmwareVersion>" DEVICE_VERSION "</tds:FirmwareVersion>"
    "<tds:SerialNumber>" DEVICE_SERIAL "</tds:SerialNumber>"
    "<tds:HardwareId>" DEVICE_HARDWARE_ID "</tds:HardwareId>"
    "</tds:GetDeviceInformationResponse>"
    "</SOAP-ENV:Body>"
    "</SOAP-ENV:Envelope>";
}

String getStreamUriResponse() {
  String ip = WiFi.localIP().toString();
  return
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
    "xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\">"
    "<SOAP-ENV:Body>"
    "<trt:GetStreamUriResponse>"
    "<trt:MediaUri>"
    "<tt:Uri>rtsp://" + ip + ":" + String(RTSP_PORT) + "/mjpeg/1</tt:Uri>"
    "<tt:InvalidAfterConnect>false</tt:InvalidAfterConnect>"
    "<tt:InvalidAfterReboot>false</tt:InvalidAfterReboot>"
    "<tt:Timeout>PT0S</tt:Timeout>"
    "</trt:MediaUri>"
    "</trt:GetStreamUriResponse>"
    "</SOAP-ENV:Body>"
    "</SOAP-ENV:Envelope>";
    "</SOAP-ENV:Body>"
    "</SOAP-ENV:Envelope>";
}

// Simple parser for SetImagingSettings
// We look for <tt:IrCutFilterMode>OFF</tt:IrCutFilterMode> to turn on 'Night Mode' (Flash ON)
// and ON or AUTO for 'Day Mode' (Flash OFF)
void handle_set_imaging_settings(String &req) {
    if (!FLASH_LED_ENABLED) return;
    
    // Very basic string parsing as XML parsing is heavy
    if (req.indexOf("IrCutFilterMode") > 0) {
        if (req.indexOf(">OFF<") > 0) {
            // Night mode -> Flash ON
            set_flash_led(true);
            Serial.println("[INFO] ONVIF: Night Mode (Flash ON)");
        } else {
            // Day mode (ON or AUTO) -> Flash OFF
            set_flash_led(false);
            Serial.println("[INFO] ONVIF: Day Mode (Flash OFF)");
        }
    }
}

    }
}

void handle_ptz(String &req) {
   #if PTZ_ENABLED
   // AbsoluteMove
   // <tptz:Vector PanTilt="x" y="0.5"/>
   // Simplified parsing: find PanTilt space x=" and y="
   // This is fragile but suffices for minimal SOAP
   
   if (req.indexOf("AbsoluteMove") > 0) {
       // Look for x="0.5" y="0.5" or similar
       // Or PanTilt x="0.5" y="0.5"
       // Actually ONVIF usually sends: <tt:PanTilt x="0.5" y="0.5" ... />
       
       float x = 0.5f; 
       float y = 0.5f;
       
       int xIdx = req.indexOf("x=\"");
       if (xIdx > 0) {
           int endQ = req.indexOf("\"", xIdx + 3);
           String val = req.substring(xIdx + 3, endQ);
           x = val.toFloat();
       }
       
       int yIdx = req.indexOf("y=\"");
       if (yIdx > 0) {
           int endQ = req.indexOf("\"", yIdx + 3);
           String val = req.substring(yIdx + 3, endQ);
           y = val.toFloat();
       }
       
       // ONVIF uses -1 to 1. Map to 0 to 1.
       // x = (x + 1.0) / 2.0;
       // y = (y + 1.0) / 2.0; 
       // NOTE: Some NVRs assume 0..1, others -1..1. 
       // Let's assume -1..1 for standard ONVIF PTZ vectors.
       
       float finalX = (x + 1.0f) / 2.0f;
       float finalY = (y + 1.0f) / 2.0f;
       
       ptz_set_absolute(finalX, finalY);
       Serial.printf("[INFO] PTZ Move: x=%.2f y=%.2f -> servo=%.2f, %.2f\n", x, y, finalX, finalY);
   }
   #endif
}

void handle_onvif_soap() {
  String req = onvifServer.arg(0);
  if (req.indexOf("GetCapabilities") > 0) {
    onvifServer.send(200, "application/soap+xml", getCapabilitiesResponse());
  } else if (req.indexOf("GetStreamUri") > 0) {
    onvifServer.send(200, "application/soap+xml", getStreamUriResponse());
  } else if (req.indexOf("GetDeviceInformation") > 0) {
    onvifServer.send(200, "application/soap+xml", getDeviceInfoResponse());
    } else if (req.indexOf("SetImagingSettings") > 0) {
    handle_set_imaging_settings(req);
    onvifServer.send(200, "application/soap+xml", "<ok/>"); // Dummy response
  } else if (req.indexOf("AbsoluteMove") > 0 || req.indexOf("ContinuousMove") > 0 || req.indexOf("Stop") > 0) {
    handle_ptz(req);
    onvifServer.send(200, "application/soap+xml", "<ok/>");
  } else {
    onvifServer.send(200, "application/soap+xml", "<ok/>");
  }
}

void handle_onvif_discovery() {
  int packetSize = onvifUDP.parsePacket();
  if (packetSize) {
    char packet[1024];
    int len = onvifUDP.read(packet, 1024);
    packet[len] = 0;
    String pktStr = String(packet);
    if (pktStr.indexOf("Probe") > 0) {
      String ip = WiFi.localIP().toString();
      String resp =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
        "xmlns:SOAP-ENC=\"http://www.w3.org/2003/05/soap-encoding\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
        "<SOAP-ENV:Body>"
        "<ProbeMatches xmlns=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\">"
        "<ProbeMatch>"
        "<EndpointReference><Address>urn:uuid:esp32-cam-onvif</Address></EndpointReference>"
        "<Types>dn:NetworkVideoTransmitter</Types>"
        "<XAddrs>http://" + ip + ":" + String(ONVIF_PORT) + "/onvif/device_service</XAddrs>"
        "<Scopes>onvif://www.onvif.org/Profile/Streaming</Scopes>"
        "<MetadataVersion>1</MetadataVersion>"
        "</ProbeMatch>"
        "</ProbeMatches>"
        "</SOAP-ENV:Body>"
        "</SOAP-ENV:Envelope>";
      onvifUDP.beginPacket(onvifUDP.remoteIP(), onvifUDP.remotePort());
      onvifUDP.write((const uint8_t*)resp.c_str(), resp.length());
      onvifUDP.endPacket();
    }
  }
}

void onvif_server_start() {
  onvifServer.on("/onvif/device_service", HTTP_POST, handle_onvif_soap);
  onvifServer.on("/onvif/ptz_service", HTTP_POST, handle_onvif_soap); // Route PTZ to same handler for now
  onvifServer.begin();
  onvifUDP.beginMulticast(IPAddress(239,255,255,250), 3702); // Fixed: use only 2 args
  Serial.println("[INFO] ONVIF server started.");
}

void onvif_server_loop() {
  onvifServer.handleClient();
  handle_onvif_discovery();
}
