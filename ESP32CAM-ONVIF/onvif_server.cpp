#include <Arduino.h>
#include "onvif_server.h"
#include "rtsp_server.h"
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
  onvifServer.begin();
  onvifUDP.beginMulticast(IPAddress(239,255,255,250), 3702); // Fixed: use only 2 args
  Serial.println("[INFO] ONVIF server started.");
}

void onvif_server_loop() {
  onvifServer.handleClient();
  handle_onvif_discovery();
}
