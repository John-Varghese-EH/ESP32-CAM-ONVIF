// ==============================================================================
//   H.264 Video Encoder Implementation
// ==============================================================================
// Implements H.264 encoding using Espressif's esp_h264 component.
// - ESP32-P4: Uses hardware encoder (30fps at 1080p, 140KB RAM)
// - ESP32-S3: Uses software encoder (~17fps at 320x192, ~1MB RAM)
//
// IMPORTANT: Add esp_h264 component to your project:
//   idf.py add-dependency "espressif/esp_h264^1.2.0"
// ==============================================================================

#include "h264_encoder.h"
#include "config.h"
#include "board_config.h"

#ifdef VIDEO_CODEC_H264

// Only compile this file if H.264 is enabled and supported
#ifdef H264_CAPABLE

#include <Arduino.h>
#include <string.h>

// ESP-IDF H.264 component headers
// Note: These require the esp_h264 component to be installed
#if __has_include("esp_h264_enc.h")
    #include "esp_h264_enc.h"
    #include "esp_h264_types.h"
    #define ESP_H264_AVAILABLE 1
#else
    #define ESP_H264_AVAILABLE 0
    #warning "esp_h264 component not found. H.264 encoding will not work."
    #warning "Run: idf.py add-dependency \"espressif/esp_h264^1.2.0\""
#endif

// Encoder state
static struct {
    bool initialized;
    bool use_hw_encoder;
    bool idr_requested;
    h264_encoder_config_t config;
    
    #if ESP_H264_AVAILABLE
    esp_h264_enc_t *encoder;
    esp_h264_enc_in_frame_t in_frame;
    esp_h264_enc_out_frame_t out_frame;
    uint8_t *input_buffer;
    size_t input_buffer_size;
    #endif
    
    // SPS/PPS storage
    uint8_t sps[64];
    size_t sps_size;
    uint8_t pps[64];
    size_t pps_size;
    
    // Frame counter for GOP
    uint32_t frame_count;
} g_encoder = {0};

// Helper to detect chip type at runtime
static bool is_esp32_p4(void) {
    #if defined(CONFIG_IDF_TARGET_ESP32P4) || defined(H264_HW_ENCODER)
        return true;
    #else
        return false;
    #endif
}

static bool is_esp32_s3(void) {
    #if defined(CONFIG_IDF_TARGET_ESP32S3)
        return true;
    #else
        return false;
    #endif
}

h264_status_t h264_encoder_init(const h264_encoder_config_t *config) {
    if (!config) {
        return H264_ERR_INVALID_PARAM;
    }
    
    if (g_encoder.initialized) {
        h264_encoder_destroy();
    }
    
    memcpy(&g_encoder.config, config, sizeof(h264_encoder_config_t));
    g_encoder.frame_count = 0;
    g_encoder.idr_requested = true; // First frame should be IDR
    
    #if !ESP_H264_AVAILABLE
        Serial.println("[ERROR] H.264: esp_h264 component not available!");
        Serial.println("[ERROR] Run: idf.py add-dependency \"espressif/esp_h264^1.2.0\"");
        return H264_ERR_NOT_SUPPORTED;
    #else
    
    // Determine encoder type
    g_encoder.use_hw_encoder = is_esp32_p4();
    
    Serial.printf("[INFO] H.264: Initializing %s encoder\n", 
                  g_encoder.use_hw_encoder ? "HARDWARE" : "SOFTWARE");
    Serial.printf("[INFO] H.264: Resolution %dx%d, FPS %d, GOP %d, Bitrate %d\n",
                  config->width, config->height, config->fps, config->gop, config->bitrate);
    
    // Check resolution limits for software encoder
    if (!g_encoder.use_hw_encoder) {
        if (config->width > H264_SW_MAX_WIDTH || config->height > H264_SW_MAX_HEIGHT) {
            Serial.printf("[WARN] H.264: Resolution exceeds SW encoder limits (%dx%d max)\n",
                          H264_SW_MAX_WIDTH, H264_SW_MAX_HEIGHT);
            Serial.println("[WARN] H.264: Performance may be poor or encoding may fail");
        }
    }
    
    esp_err_t ret;
    
    if (g_encoder.use_hw_encoder) {
        // Hardware encoder configuration (ESP32-P4)
        #if defined(CONFIG_IDF_TARGET_ESP32P4)
        esp_h264_enc_cfg_hw_t hw_cfg = {0};
        hw_cfg.gop = config->gop;
        hw_cfg.fps = config->fps;
        hw_cfg.res.width = config->width;
        hw_cfg.res.height = config->height;
        hw_cfg.rc.bitrate = config->bitrate;
        hw_cfg.rc.qp_min = config->qp_min;
        hw_cfg.rc.qp_max = config->qp_max;
        hw_cfg.pic_type = ESP_H264_RAW_FMT_O_UYY_E_VYY;
        
        ret = esp_h264_enc_hw_new(&hw_cfg, &g_encoder.encoder);
        #else
        ret = ESP_ERR_NOT_SUPPORTED;
        #endif
    } else {
        // Software encoder configuration (ESP32-S3)
        esp_h264_enc_cfg_sw_t sw_cfg = {0};
        sw_cfg.gop = config->gop;
        sw_cfg.fps = config->fps;
        sw_cfg.res.width = config->width;
        sw_cfg.res.height = config->height;
        sw_cfg.rc.bitrate = config->bitrate;
        sw_cfg.rc.qp_min = config->qp_min;
        sw_cfg.rc.qp_max = config->qp_max;
        sw_cfg.pic_type = ESP_H264_RAW_FMT_I420; // YUV420 for SW encoder
        
        ret = esp_h264_enc_sw_new(&sw_cfg, &g_encoder.encoder);
    }
    
    if (ret != ESP_OK || !g_encoder.encoder) {
        Serial.printf("[ERROR] H.264: Encoder creation failed (err=%d)\n", ret);
        return H264_ERR_INIT_FAILED;
    }
    
    // Allocate input buffer (YUV420: width * height * 1.5)
    g_encoder.input_buffer_size = config->width * config->height * 3 / 2;
    
    // Use aligned allocation for hardware encoder
    if (g_encoder.use_hw_encoder) {
        g_encoder.input_buffer = (uint8_t*)esp_h264_aligned_calloc(
            128, 1, g_encoder.input_buffer_size, 
            &g_encoder.input_buffer_size, 
            ESP_H264_MEM_SPIRAM  // Use SPIRAM if available
        );
    } else {
        g_encoder.input_buffer = (uint8_t*)heap_caps_malloc(
            g_encoder.input_buffer_size, 
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
        );
    }
    
    if (!g_encoder.input_buffer) {
        Serial.println("[ERROR] H.264: Failed to allocate input buffer");
        esp_h264_enc_del(g_encoder.encoder);
        g_encoder.encoder = NULL;
        return H264_ERR_NO_MEMORY;
    }
    
    // Open the encoder
    ret = esp_h264_enc_open(g_encoder.encoder);
    if (ret != ESP_OK) {
        Serial.printf("[ERROR] H.264: Failed to open encoder (err=%d)\n", ret);
        free(g_encoder.input_buffer);
        esp_h264_enc_del(g_encoder.encoder);
        return H264_ERR_INIT_FAILED;
    }
    
    g_encoder.initialized = true;
    Serial.printf("[INFO] H.264: Encoder initialized successfully (%s)\n",
                  g_encoder.use_hw_encoder ? "Hardware" : "Software");
    
    return H264_OK;
    #endif // ESP_H264_AVAILABLE
}

h264_status_t h264_encoder_encode(const uint8_t *raw_data, size_t raw_size, h264_frame_t *out_frame) {
    #if !ESP_H264_AVAILABLE
        return H264_ERR_NOT_SUPPORTED;
    #else
    
    if (!g_encoder.initialized || !g_encoder.encoder) {
        return H264_ERR_NOT_INITIALIZED;
    }
    
    if (!raw_data || !out_frame) {
        return H264_ERR_INVALID_PARAM;
    }
    
    // Copy raw data to aligned buffer
    size_t copy_size = (raw_size < g_encoder.input_buffer_size) ? raw_size : g_encoder.input_buffer_size;
    memcpy(g_encoder.input_buffer, raw_data, copy_size);
    
    // Setup input frame
    g_encoder.in_frame.raw_data.buffer = g_encoder.input_buffer;
    g_encoder.in_frame.raw_data.len = copy_size;
    
    // Request IDR if needed (first frame or explicit request)
    if (g_encoder.idr_requested || (g_encoder.frame_count % g_encoder.config.gop == 0)) {
        esp_h264_enc_set_request_idr(g_encoder.encoder);
        g_encoder.idr_requested = false;
    }
    
    // Encode the frame
    esp_err_t ret = esp_h264_enc_process(g_encoder.encoder, &g_encoder.in_frame, &g_encoder.out_frame);
    if (ret != ESP_OK) {
        Serial.printf("[ERROR] H.264: Encoding failed (err=%d)\n", ret);
        return H264_ERR_ENCODE_FAILED;
    }
    
    // Fill output structure
    out_frame->data = g_encoder.out_frame.raw_data.buffer;
    out_frame->size = g_encoder.out_frame.raw_data.len;
    out_frame->timestamp = millis();
    
    // Check if this is an IDR frame
    if (g_encoder.out_frame.raw_data.len > 4) {
        uint8_t nal_type = out_frame->data[4] & 0x1F;
        out_frame->type = (nal_type == 5 || nal_type == 7) ? H264_FRAME_TYPE_IDR : H264_FRAME_TYPE_P;
        out_frame->contains_sps_pps = (nal_type == 7); // SPS
    } else {
        out_frame->type = H264_FRAME_TYPE_UNKNOWN;
        out_frame->contains_sps_pps = false;
    }
    
    g_encoder.frame_count++;
    
    return H264_OK;
    #endif // ESP_H264_AVAILABLE
}

h264_status_t h264_encoder_encode_jpeg(const uint8_t *jpeg_data, size_t jpeg_size, h264_frame_t *out_frame) {
    // JPEG to H.264 conversion would require JPEG decoding first
    // This is expensive and not recommended for real-time use
    // For now, return not supported - users should capture raw frames
    (void)jpeg_data;
    (void)jpeg_size;
    (void)out_frame;
    
    Serial.println("[WARN] H.264: JPEG to H.264 encoding not implemented");
    Serial.println("[WARN] Configure camera for YUV output for H.264 encoding");
    
    return H264_ERR_NOT_SUPPORTED;
}

void h264_encoder_request_idr(void) {
    g_encoder.idr_requested = true;
}

h264_status_t h264_encoder_get_sps(uint8_t *sps_data, size_t *sps_size) {
    #if !ESP_H264_AVAILABLE
        return H264_ERR_NOT_SUPPORTED;
    #else
    if (!g_encoder.initialized) {
        return H264_ERR_NOT_INITIALIZED;
    }
    
    if (g_encoder.sps_size > 0 && sps_data && sps_size) {
        size_t copy_size = (*sps_size < g_encoder.sps_size) ? *sps_size : g_encoder.sps_size;
        memcpy(sps_data, g_encoder.sps, copy_size);
        *sps_size = g_encoder.sps_size;
        return H264_OK;
    }
    
    return H264_ERR_INVALID_PARAM;
    #endif
}

h264_status_t h264_encoder_get_pps(uint8_t *pps_data, size_t *pps_size) {
    #if !ESP_H264_AVAILABLE
        return H264_ERR_NOT_SUPPORTED;
    #else
    if (!g_encoder.initialized) {
        return H264_ERR_NOT_INITIALIZED;
    }
    
    if (g_encoder.pps_size > 0 && pps_data && pps_size) {
        size_t copy_size = (*pps_size < g_encoder.pps_size) ? *pps_size : g_encoder.pps_size;
        memcpy(pps_data, g_encoder.pps, copy_size);
        *pps_size = g_encoder.pps_size;
        return H264_OK;
    }
    
    return H264_ERR_INVALID_PARAM;
    #endif
}

bool h264_encoder_is_hw(void) {
    return g_encoder.use_hw_encoder;
}

void h264_encoder_destroy(void) {
    #if ESP_H264_AVAILABLE
    if (g_encoder.encoder) {
        esp_h264_enc_close(g_encoder.encoder);
        esp_h264_enc_del(g_encoder.encoder);
        g_encoder.encoder = NULL;
    }
    
    if (g_encoder.input_buffer) {
        if (g_encoder.use_hw_encoder) {
            esp_h264_free(g_encoder.input_buffer);
        } else {
            free(g_encoder.input_buffer);
        }
        g_encoder.input_buffer = NULL;
    }
    #endif
    
    g_encoder.initialized = false;
    Serial.println("[INFO] H.264: Encoder destroyed");
}

const char* h264_encoder_get_type_string(void) {
    if (!g_encoder.initialized) {
        return "Not Initialized";
    }
    return g_encoder.use_hw_encoder ? "Hardware (ESP32-P4)" : "Software (ESP32-S3)";
}

#else // H264_CAPABLE not defined

// Provide stub implementations if H.264 is requested but not supported
#warning "H.264 encoding is enabled but this board doesn't support it. H.264 functions will return errors."

h264_status_t h264_encoder_init(const h264_encoder_config_t *config) {
    (void)config;
    Serial.println("[ERROR] H.264: Not supported on this chip!");
    Serial.println("[ERROR] H.264: Requires ESP32-P4 or ESP32-S3");
    return H264_ERR_NOT_SUPPORTED;
}

h264_status_t h264_encoder_encode(const uint8_t *raw_data, size_t raw_size, h264_frame_t *out_frame) {
    (void)raw_data; (void)raw_size; (void)out_frame;
    return H264_ERR_NOT_SUPPORTED;
}

h264_status_t h264_encoder_encode_jpeg(const uint8_t *jpeg_data, size_t jpeg_size, h264_frame_t *out_frame) {
    (void)jpeg_data; (void)jpeg_size; (void)out_frame;
    return H264_ERR_NOT_SUPPORTED;
}

void h264_encoder_request_idr(void) {}

h264_status_t h264_encoder_get_sps(uint8_t *sps_data, size_t *sps_size) {
    (void)sps_data; (void)sps_size;
    return H264_ERR_NOT_SUPPORTED;
}

h264_status_t h264_encoder_get_pps(uint8_t *pps_data, size_t *pps_size) {
    (void)pps_data; (void)pps_size;
    return H264_ERR_NOT_SUPPORTED;
}

bool h264_encoder_is_hw(void) { return false; }
void h264_encoder_destroy(void) {}
const char* h264_encoder_get_type_string(void) { return "Not Supported"; }

#endif // H264_CAPABLE

#endif // VIDEO_CODEC_H264
