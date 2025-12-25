#include <Arduino.h>
#include "onvif_server.h"
#include "rtsp_server.h"
#include "camera_control.h"
#include <WiFiUdp.h>
#include <WebServer.h>
#include <time.h>
#include "mbedtls/sha1.h"
#include "mbedtls/base64.h"
#include "config.h"

WebServer onvifServer(ONVIF_PORT);
WiFiUDP onvifUDP;

const char PROGMEM PART_HEADER[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" ";
const char PROGMEM PART_BODY[] = "<SOAP-ENV:Body>";
const char PROGMEM PART_END[] = "</SOAP-ENV:Body></SOAP-ENV:Envelope>";

// --- PROGMEM Templates ---
const char PROGMEM TPL_CAPABILITIES[] = 
    "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
    "<SOAP-ENV:Body>"
    "<tds:GetCapabilitiesResponse>"
    "<tds:Capabilities>"
    "<tds:Media><tds:XAddr>http://%s:%d/onvif/device_service</tds:XAddr></tds:Media>"
    "</tds:Capabilities>"
    "</tds:GetCapabilitiesResponse>"
    "</SOAP-ENV:Body></SOAP-ENV:Envelope>";

const char PROGMEM TPL_DEV_INFO[] = 
    "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
    "<SOAP-ENV:Body>"
    "<tds:GetDeviceInformationResponse>"
    "<tds:Manufacturer>" DEVICE_MANUFACTURER "</tds:Manufacturer>"
    "<tds:Model>" DEVICE_MODEL "</tds:Model>"
    "<tds:FirmwareVersion>" DEVICE_VERSION "</tds:FirmwareVersion>"
    "<tds:SerialNumber>" DEVICE_SERIAL "</tds:SerialNumber>"
    "<tds:HardwareId>" DEVICE_HARDWARE_ID "</tds:HardwareId>"
    "</tds:GetDeviceInformationResponse>"
    "</SOAP-ENV:Body></SOAP-ENV:Envelope>";

const char PROGMEM TPL_STREAM_URI[] = 
    "xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
    "<SOAP-ENV:Body>"
    "<trt:GetStreamUriResponse>"
    "<trt:MediaUri>"
    "<tt:Uri>rtsp://%s:%d/mjpeg/1</tt:Uri>"
    "<tt:InvalidAfterConnect>false</tt:InvalidAfterConnect>"
    "<tt:InvalidAfterReboot>false</tt:InvalidAfterReboot>"
    "<tt:Timeout>PT0S</tt:Timeout>"
    "</trt:MediaUri>"
    "</trt:GetStreamUriResponse>"
    "</SOAP-ENV:Body></SOAP-ENV:Envelope>";
    
const char PROGMEM TPL_TIME[] =
    "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
    "<SOAP-ENV:Body>"
    "<tds:GetSystemDateAndTimeResponse>"
        "<tds:SystemDateAndTime>"
            "<tt:DateTimeType>Manual</tt:DateTimeType>"
            "<tt:DaylightSavings>false</tt:DaylightSavings>"
            "<tt:TimeZone><tt:TZ>GMT+00:00</tt:TZ></tt:TimeZone>"
            "<tt:UTCDateTime>"
                "<tt:Time><tt:Hour>12</tt:Hour><tt:Minute>00</tt:Minute><tt:Second>00</tt:Second></tt:Time>"
                "<tt:Date><tt:Year>2025</tt:Year><tt:Month>1</tt:Month><tt:Day>1</tt:Day></tt:Date>"
            "</tt:UTCDateTime>"
        "</tds:SystemDateAndTime>"
    "</tds:GetSystemDateAndTimeResponse>"
    "</SOAP-ENV:Body></SOAP-ENV:Envelope>";
    
// Time Sync: Minimal 'Ok' response template for SetSystemDateAndTime
const char PROGMEM TPL_SET_TIME_RES[] = 
    "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
    "<SOAP-ENV:Body>"
    "<tds:SetSystemDateAndTimeResponse/>"
    "</SOAP-ENV:Body></SOAP-ENV:Envelope>";

// Helper to base64 decode
int base64_decode(String input, uint8_t *output) {
    size_t olen;
    mbedtls_base64_decode(output, 64, &olen, (const unsigned char *)input.c_str(), input.length());
    return olen;
}

// Helper to base64 encode
String base64_encode(uint8_t *data, size_t length) {
    unsigned char output[128];
    size_t olen;
    mbedtls_base64_encode(output, sizeof(output), &olen, data, length);
    return String((char *)output);
}

// WS-UsernameToken Verification
bool verify_soap_header(String &soapReq) {
    // 1. Check if Security Header exists
    int secIdx = soapReq.indexOf("Security");
    if (secIdx < 0) return false; // No security header

    // 2. Extract Username
    int userStart = soapReq.indexOf("<wsse:Username>", secIdx);
    if (userStart < 0) return false;
    userStart += 15;
    int userEnd = soapReq.indexOf("</wsse:Username>", userStart);
    String username = soapReq.substring(userStart, userEnd);
    
    if (username != WEB_USER) {
        Serial.println("[AUTH] User mismatch: " + username);
        return false;
    }

    // 3. Extract Password (Digest)
    int passStart = soapReq.indexOf("<wsse:Password", secIdx); // Handle attributes potentially
    passStart = soapReq.indexOf(">", passStart) + 1;
    int passEnd = soapReq.indexOf("</wsse:Password>", passStart);
    String digestBase64 = soapReq.substring(passStart, passEnd);

    // 4. Extract Nonce
    int nonceStart = soapReq.indexOf("<wsse:Nonce", secIdx);
    nonceStart = soapReq.indexOf(">", nonceStart) + 1;
    int nonceEnd = soapReq.indexOf("</wsse:Nonce>", nonceStart);
    String nonceBase64 = soapReq.substring(nonceStart, nonceEnd);

    // 5. Extract Created
    int createdStart = soapReq.indexOf("<wsu:Created>", secIdx);
    createdStart += 13;
    int createdEnd = soapReq.indexOf("</wsu:Created>", createdStart);
    String created = soapReq.substring(createdStart, createdEnd);
    
    // 6. Verify Digest = Base64(SHA1(Base64Decode(Nonce) + Created + Password))
    uint8_t nonce[64];
    int nonceLen = base64_decode(nonceBase64, nonce);
    
    // Concatenate buffer
    uint8_t buffer[128]; 
    memcpy(buffer, nonce, nonceLen);
    memcpy(buffer + nonceLen, created.c_str(), created.length());
    memcpy(buffer + nonceLen + created.length(), WEB_PASS, strlen(WEB_PASS));
    
    size_t totalLen = nonceLen + created.length() + strlen(WEB_PASS);
    
    uint8_t sha1Result[20];
    mbedtls_sha1(buffer, totalLen, sha1Result);
    
    String calculatedDigest = base64_encode(sha1Result, 20);
    
    // Trim potential whitespace or padding issues (simple exact match first)
    // Serial.println("Calc: " + calculatedDigest);
    // Serial.println("Recv: " + digestBase64);
    
    if (calculatedDigest.indexOf(digestBase64) >= 0 || digestBase64.indexOf(calculatedDigest) >= 0) {
         return true;
    }
    
    Serial.println("[AUTH] Digest verification failed");
    return false;
}

// Handle SetSystemDateAndTime
void handle_SetSystemDateAndTime(String &req) {
    // <tt:UTCDateTime>
    // <tt:Time><tt:Hour>12</tt:Hour><tt:Minute>34</tt:Minute><tt:Second>56</tt:Second></tt:Time>
    // <tt:Date><tt:Year>2025</tt:Year><tt:Month>1</tt:Month><tt:Day>2</tt:Day></tt:Date>
    // </tt:UTCDateTime>
    
    int container = req.indexOf("UTCDateTime");
    if (container > 0) {
        int year = 2024; int month = 1; int day = 1;
        int hour = 0; int min = 0; int sec = 0;
        
        // Simple Tag parsing
        if(req.indexOf("<tt:Year>", container) > 0) year = req.substring(req.indexOf("<tt:Year>", container)+9, req.indexOf("</tt:Year>", container)).toInt();
        if(req.indexOf("<tt:Month>", container) > 0) month = req.substring(req.indexOf("<tt:Month>", container)+10, req.indexOf("</tt:Month>", container)).toInt();
        if(req.indexOf("<tt:Day>", container) > 0) day = req.substring(req.indexOf("<tt:Day>", container)+8, req.indexOf("</tt:Day>", container)).toInt();
        
        if(req.indexOf("<tt:Hour>", container) > 0) hour = req.substring(req.indexOf("<tt:Hour>", container)+9, req.indexOf("</tt:Hour>", container)).toInt();
        if(req.indexOf("<tt:Minute>", container) > 0) min = req.substring(req.indexOf("<tt:Minute>", container)+11, req.indexOf("</tt:Minute>", container)).toInt();
        if(req.indexOf("<tt:Second>", container) > 0) sec = req.substring(req.indexOf("<tt:Second>", container)+11, req.indexOf("</tt:Second>", container)).toInt();
        
        // Set ESP32 Time
        struct tm tm;
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = min;
        tm.tm_sec = sec;
        tm.tm_isdst = 0;
        time_t t = mktime(&tm);
        struct timeval now = { .tv_sec = t, .tv_usec = 0 };
        settimeofday(&now, NULL);
        
        Serial.printf("[INFO] Time Synced to: %04d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, min, sec);
    }
}

void sendPROGMEM(WebServer &server, const char* content) {
    server.send_P(200, "application/soap+xml", content);
}

// Helper to send SOAP Fault
void send_soap_fault(WebServer &server, const char* code, const char* subcode, const char* reason) {
    String fault = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    fault += "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:ter=\"http://www.onvif.org/ver10/error\">";
    fault += "<SOAP-ENV:Body><SOAP-ENV:Fault>";
    fault += "<SOAP-ENV:Code><SOAP-ENV:Value>" + String(code) + "</SOAP-ENV:Value>";
    fault += "<SOAP-ENV:Subcode><SOAP-ENV:Value>" + String(subcode) + "</SOAP-ENV:Value></SOAP-ENV:Subcode>";
    fault += "</SOAP-ENV:Code>";
    fault += "<SOAP-ENV:Reason><SOAP-ENV:Text xml:lang=\"en\">" + String(reason) + "</SOAP-ENV:Text></SOAP-ENV:Reason>";
    fault += "</SOAP-ENV:Fault></SOAP-ENV:Body></SOAP-ENV:Envelope>";
    
    server.send(500, "application/soap+xml", fault);
}

// Optimized heap-less send for dynamic content
void sendDynamicPROGMEM(WebServer &server, const char* tpl, const char* ip, int port) {
    char buffer[2048]; // Increased buffer for safety
    snprintf_P(buffer, sizeof(buffer), PART_HEADER); 
    size_t len = strlen(buffer);
    snprintf_P(buffer + len, sizeof(buffer) - len, tpl, ip, port);
    server.send(200, "application/soap+xml", buffer);
}
// Overload for just sending fixed PROGMEM with header
void sendFixedPROGMEM(WebServer &server, const char* tpl) {
    char buffer[2048]; // Increased buffer for safety
    snprintf_P(buffer, sizeof(buffer), PART_HEADER);
    strncat_P(buffer, tpl, sizeof(buffer) - strlen(buffer) - 1);
    server.send(200, "application/soap+xml", buffer);
}

// --- New Handlers ---

// Mandatory for many NVRs to link Profile to Source
const char PROGMEM TPL_VIDEO_SOURCES[] = 
    "xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
    "<SOAP-ENV:Body>"
    "<trt:GetVideoSourcesResponse>"
        "<trt:VideoSources token=\"VideoSource_1\">"
            "<tt:Framerate>25.0</tt:Framerate>"
            "<tt:Resolution><tt:Width>1024</tt:Width><tt:Height>768</tt:Height></tt:Resolution>"
            "<tt:Imaging><tt:BacklightCompensation><tt:Mode>OFF</tt:Mode></tt:BacklightCompensation>"
            "<tt:Exposure><tt:Mode>AUTO</tt:Mode></tt:Exposure>"
            "</tt:Imaging>"
        "</trt:VideoSources>"
    "</trt:GetVideoSourcesResponse>"
    "</SOAP-ENV:Body></SOAP-ENV:Envelope>";

// Tells NVR valid ranges (resolutions, quality, etc)
const char PROGMEM TPL_VIDEO_OPTIONS[] = 
    "xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
    "<SOAP-ENV:Body>"
    "<trt:GetVideoEncoderConfigurationOptionsResponse>"
        "<trt:Options>"
            "<tt:QualityRange><tt:Min>0</tt:Min><tt:Max>63</tt:Max></tt:QualityRange>"
            "<tt:JPEG><tt:ResolutionsAvailable><tt:Width>1024</tt:Width><tt:Height>768</tt:Height></tt:ResolutionsAvailable>"
            "<tt:ResolutionsAvailable><tt:Width>800</tt:Width><tt:Height>600</tt:Height></tt:ResolutionsAvailable>"
            "<tt:FrameRateRange><tt:Min>1</tt:Min><tt:Max>30</tt:Max></tt:FrameRateRange>"
            "<tt:EncodingIntervalRange><tt:Min>1</tt:Min><tt:Max>1</tt:Max></tt:EncodingIntervalRange>"
            "</tt:JPEG>"
        "</trt:Options>"
    "</trt:GetVideoEncoderConfigurationOptionsResponse>"
    "</SOAP-ENV:Body></SOAP-ENV:Envelope>";

void handle_GetCapabilities() {
    sendDynamicPROGMEM(onvifServer, TPL_CAPABILITIES, WiFi.localIP().toString().c_str(), ONVIF_PORT);
}

void handle_GetStreamUri() {
    sendDynamicPROGMEM(onvifServer, TPL_STREAM_URI, WiFi.localIP().toString().c_str(), RTSP_PORT);
}

void handle_GetSystemDateAndTime() {
    sendFixedPROGMEM(onvifServer, TPL_TIME);
}

void handle_GetDeviceInformation() {
    sendFixedPROGMEM(onvifServer, TPL_DEV_INFO);
}


String getServicesResponse() {
  String ip = WiFi.localIP().toString();
  String xaddr = "http://" + ip + ":" + String(ONVIF_PORT) + "/onvif/device_service";
  return
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
    "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
    "xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
    "<SOAP-ENV:Body>"
    "<tds:GetServicesResponse>"
        "<tds:Service>"
            "<tds:Namespace>http://www.onvif.org/ver10/device/wsdl</tds:Namespace>"
            "<tds:XAddr>" + xaddr + "</tds:XAddr>"
            "<tds:Version><tt:Major>2</tt:Major><tt:Minor>5</tt:Minor></tds:Version>"
        "</tds:Service>"
        "<tds:Service>"
            "<tds:Namespace>http://www.onvif.org/ver10/media/wsdl</tds:Namespace>"
            "<tds:XAddr>" + xaddr + "</tds:XAddr>"
            "<tds:Version><tt:Major>2</tt:Major><tt:Minor>5</tt:Minor></tds:Version>"
        "</tds:Service>"
        "<tds:Service>"
            "<tds:Namespace>http://www.onvif.org/ver20/ptz/wsdl</tds:Namespace>"
            "<tds:XAddr>" + xaddr + "</tds:XAddr>"
            "<tds:Version><tt:Major>2</tt:Major><tt:Minor>5</tt:Minor></tds:Version>"
        "</tds:Service>"
    "</tds:GetServicesResponse>"
    "</SOAP-ENV:Body>"
    "</SOAP-ENV:Envelope>";
}

String getProfilesResponse() {
  return
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
    "xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" "
    "xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
    "<SOAP-ENV:Body>"
    "<trt:GetProfilesResponse>"
        "<trt:Profiles token=\"Profile_1\">"
            "<tt:Name>MainStream</tt:Name>"
            "<tt:VideoSourceConfiguration token=\"VideoSourceToken\">"
                "<tt:Name>VideoSourceConfig</tt:Name>"
                "<tt:UseCount>1</tt:UseCount>"
                "<tt:SourceToken>VideoSource_1</tt:SourceToken>"
                "<tt:Bounds x=\"0\" y=\"0\" width=\"1024\" height=\"768\"/>"
            "</tt:VideoSourceConfiguration>"
            "<tt:VideoEncoderConfiguration token=\"VideoEncoderToken\">"
                "<tt:Name>VideoEncoderConfig</tt:Name>"
                "<tt:UseCount>1</tt:UseCount>"
                "<tt:Encoding>JPEG</tt:Encoding>"
                "<tt:Resolution><tt:Width>1024</tt:Width><tt:Height>768</tt:Height></tt:Resolution>"
                "<tt:Quality>12</tt:Quality>"
            "</tt:VideoEncoderConfiguration>"
            "<tt:PTZConfiguration token=\"PTZToken\">"
                "<tt:Name>PTZConfig</tt:Name>"
                "<tt:UseCount>1</tt:UseCount>"
                "<tt:NodeToken>PTZNodeToken</tt:NodeToken>"
                "<tt:DefaultContinuousPanTiltVelocitySpace>http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace</tt:DefaultContinuousPanTiltVelocitySpace>"
                "<tt:DefaultPTZSpeed>"
                    "<tt:PanTilt x=\"1.0\" y=\"1.0\" space=\"http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace\"/>"
                "</tt:DefaultPTZSpeed>"
            "</tt:PTZConfiguration>"
        "</trt:Profiles>"
    "</trt:GetProfilesResponse>"
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

// Note: Some NVRs will fail Probe/Discovery if authentication is required for simple gets.
// Usually basic GetCapabilities/GetServices is open, but others require Auth.
// Hikvision is STRICT.
void handle_onvif_soap() {
  String req = onvifServer.arg(0);
  
  // Auth Check for Sensitivity Actions
  // Note: GetCapabilities/GetSystemDateAndTime often must be open for initial handshake.
  bool authRequired = (req.indexOf("GetStreamUri") > 0 || req.indexOf("GetProfiles") > 0 || req.indexOf("SetSystemDateAndTime") > 0);
  
  // If request contains Security header, we MUST verify it regardless
  bool hasSecurity = (req.indexOf("Security") > 0);
  
  if (hasSecurity || authRequired) {
      if (!verify_soap_header(req)) {
          // Send Fault (Unauthenticated) if validation fails but auth was present or required
          // However, for simplicity/compatibility, if Auth is just MISSING on a req that 'should' have it but isn't critical (like GetProfiles during scan),
          // some devices respond openly. But for Hikvision, we want to prove we handle auth.
          Serial.println("[WARN] SOAP Auth Failed");
          send_soap_fault(onvifServer, "env:Sender", "ter:NotAuthorized", "Sender not authorized");
          return;
      } else {
           // Serial.println("[INFO] SOAP Auth OK"); // Reduce noise, only log action below
      }
  }

  // Debug: Identify Action
  String action = "Unknown";
  if (req.indexOf("GetSystemDateAndTime") > 0) action = "GetSystemDateAndTime";
  else if (req.indexOf("GetCapabilities") > 0) action = "GetCapabilities";
  else if (req.indexOf("GetServices") > 0) action = "GetServices";
  else if (req.indexOf("GetDeviceInformation") > 0) action = "GetDeviceInformation";
  else if (req.indexOf("GetProfiles") > 0) action = "GetProfiles";
  else if (req.indexOf("GetStreamUri") > 0) action = "GetStreamUri";
  else if (req.indexOf("GetSnapshotUri") > 0) action = "GetSnapshotUri";
  else if (req.indexOf("GetVideoSources") > 0) action = "GetVideoSources";
  else if (req.indexOf("GetVideoEncoderConfigurationOptions") > 0) action = "GetVideoOptions";
  
  Serial.println("[INFO] ONVIF Action: " + action);

  if (action == "Unknown") {
      // Print the request to help debug what the NVR wants
      Serial.println("[DEBUG] Unknown Request Body (Truncated):");
      Serial.println(req.substring(0, 600)); 
  }

  if (req.indexOf("GetCapabilities") > 0) {
    handle_GetCapabilities();
  } else if (req.indexOf("GetStreamUri") > 0) {
    handle_GetStreamUri();
  } else if (req.indexOf("GetSnapshotUri") > 0) {
    // Send dynamic Snapshot URI pointing to /snapshot
    const char PROGMEM TPL_SNAPSHOT_URI[] = 
    "xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
    "<SOAP-ENV:Body>"
    "<trt:GetSnapshotUriResponse>"
    "<trt:MediaUri>"
    "<tt:Uri>http://%s:%d/snapshot</tt:Uri>"
    "<tt:InvalidAfterConnect>false</tt:InvalidAfterConnect>"
    "<tt:InvalidAfterReboot>false</tt:InvalidAfterReboot>"
    "<tt:Timeout>PT0S</tt:Timeout>"
    "</trt:MediaUri>"
    "</trt:GetSnapshotUriResponse>"
    "</SOAP-ENV:Body></SOAP-ENV:Envelope>";
    
    sendDynamicPROGMEM(onvifServer, TPL_SNAPSHOT_URI, WiFi.localIP().toString().c_str(), WEB_PORT);
  } else if (req.indexOf("GetDeviceInformation") > 0) {
    handle_GetDeviceInformation();
  } else if (req.indexOf("GetSystemDateAndTime") > 0) {
    handle_GetSystemDateAndTime();
  } else if (req.indexOf("GetServices") > 0) {
     const char PROGMEM TPL_SERVICES[] = 
    "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
    "<SOAP-ENV:Body>"
    "<tds:GetServicesResponse>"
        // Simplified service list to fit in stack buffer
        "<tds:Service><tds:Namespace>http://www.onvif.org/ver10/device/wsdl</tds:Namespace><tds:XAddr>http://%s:%d/onvif/device_service</tds:XAddr><tds:Version><tt:Major>2</tt:Major><tt:Minor>5</tt:Minor></tds:Version></tds:Service>"
        "<tds:Service><tds:Namespace>http://www.onvif.org/ver10/media/wsdl</tds:Namespace><tds:XAddr>http://%s:%d/onvif/device_service</tds:XAddr><tds:Version><tt:Major>2</tt:Major><tt:Minor>5</tt:Minor></tds:Version></tds:Service>"
    "</tds:GetServicesResponse>"
    "</SOAP-ENV:Body></SOAP-ENV:Envelope>";
    
    char buffer[1536]; // Needs larger buffer
    snprintf_P(buffer, sizeof(buffer), PART_HEADER); 
    size_t len = strlen(buffer);
    snprintf_P(buffer + len, sizeof(buffer) - len, TPL_SERVICES, WiFi.localIP().toString().c_str(), ONVIF_PORT, WiFi.localIP().toString().c_str(), ONVIF_PORT);
    onvifServer.send(200, "application/soap+xml", buffer);

  } else if (req.indexOf("GetProfiles") > 0) {
     // Massive string, handle carefully
     const char PROGMEM TPL_PROFILES[] = 
    "xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
    "<SOAP-ENV:Body>"
    "<trt:GetProfilesResponse>"
        "<trt:Profiles token=\"Profile_1\">"
            "<tt:Name>MainStream</tt:Name>"
            "<tt:VideoSourceConfiguration token=\"VideoSourceToken\">"
                "<tt:Name>VideoSourceConfig</tt:Name><tt:UseCount>1</tt:UseCount><tt:SourceToken>VideoSource_1</tt:SourceToken><tt:Bounds x=\"0\" y=\"0\" width=\"1024\" height=\"768\"/></tt:VideoSourceConfiguration>"
            "<tt:VideoEncoderConfiguration token=\"VideoEncoderToken\">"
                "<tt:Name>VideoEncoderConfig</tt:Name><tt:UseCount>1</tt:UseCount><tt:Encoding>JPEG</tt:Encoding><tt:Resolution><tt:Width>1024</tt:Width><tt:Height>768</tt:Height></tt:Resolution><tt:Quality>12</tt:Quality></tt:VideoEncoderConfiguration>"
            "<tt:PTZConfiguration token=\"PTZToken\">"
                "<tt:Name>PTZConfig</tt:Name><tt:UseCount>1</tt:UseCount><tt:NodeToken>PTZNodeToken</tt:NodeToken><tt:DefaultContinuousPanTiltVelocitySpace>http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace</tt:DefaultContinuousPanTiltVelocitySpace><tt:DefaultPTZSpeed><tt:PanTilt x=\"1.0\" y=\"1.0\" space=\"http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace\"/></tt:DefaultPTZSpeed></tt:PTZConfiguration>"
        "</trt:Profiles>"
    "</trt:GetProfilesResponse>"
    "</SOAP-ENV:Body></SOAP-ENV:Envelope>";
    sendFixedPROGMEM(onvifServer, TPL_PROFILES);

  } else if (req.indexOf("GetVideoSources") > 0) {
    sendFixedPROGMEM(onvifServer, TPL_VIDEO_SOURCES);
  } else if (req.indexOf("GetVideoEncoderConfigurationOptions") > 0) {
    sendFixedPROGMEM(onvifServer, TPL_VIDEO_OPTIONS);
  } else if (req.indexOf("SetSystemDateAndTime") > 0) {
    handle_SetSystemDateAndTime(req);
    sendFixedPROGMEM(onvifServer, TPL_SET_TIME_RES);
  } else if (req.indexOf("SetImagingSettings") > 0) {
    handle_set_imaging_settings(req);
    onvifServer.send(200, "application/soap+xml", "<ok/>"); 
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
      
      // WS-Discovery Probe Match Response
      // This is the CRITICAL packet for NVRs to find the camera.
      // - RelatesTo: Should match the MessageID of the Probe (omitted here for simplicity as UDP allows multicast broadcast).
      // - Types: dn:NetworkVideoTransmitter (Tells NVR this is a Camera).
      // - XAddrs: The URL to the implementation of the device service (http://<IP>:8000/onvif/device_service).
      // - Scopes: onvif://www.onvif.org/Profile/Streaming (Capabilities).
      
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
