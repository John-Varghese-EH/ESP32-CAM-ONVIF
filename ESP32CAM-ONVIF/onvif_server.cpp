#include <Arduino.h>
#include "onvif_server.h"
#include "rtsp_server.h"
#include <WiFiUdp.h>
#include <WebServer.h>
#include "config.h"

WebServer onvifServer(8000);
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
    "<tds:XAddr>http://" + ip + ":8000/onvif/device_service</tds:XAddr>"
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
    "<tds:Manufacturer>ESP32-CAM-J0X</tds:Manufacturer>"
    "<tds:Model>ONVIF-ESPCAM</tds:Model>"
    "<tds:FirmwareVersion>1.0</tds:FirmwareVersion>"
    "<tds:SerialNumber>J0X-00001</tds:SerialNumber>"
    "<tds:HardwareId>ESP32CAM-J0X</tds:HardwareId>"
    "</tds:GetDeviceInformationResponse>"
    "</SOAP-ENV:Body>"
    "</SOAP-ENV:Envelope>";
}

void handle_onvif_soap() {
  String req = onvifServer.arg(0);
  if (req.indexOf("GetCapabilities") > 0) {
    onvifServer.send(200, "application/soap+xml", getCapabilitiesResponse());
  } else if (req.indexOf("GetStreamUri") > 0) {
    onvifServer.send(200, "application/soap+xml", getStreamUriResponse());
  } else if (req.indexOf("GetDeviceInformation") > 0) {
    onvifServer.send(200, "application/soap+xml", getDeviceInfoResponse());
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
        "<XAddrs>http://" + ip + ":8000/onvif/device_service</XAddrs>"
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
  onvifServer.begin();
  onvifUDP.beginMulticast(WiFi.localIP(), IPAddress(239,255,255,250), 3702);
  Serial.println("[INFO] ONVIF server started.");
}

void onvif_server_loop() {
  onvifServer.handleClient();
  handle_onvif_discovery();
}
