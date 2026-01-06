#pragma once
// ==============================================================================
//   H.264 Video Encoder Interface
// ==============================================================================
// Provides H.264 encoding support for ESP32-P4 (hardware) and ESP32-S3 (software).
// This module is only active when VIDEO_CODEC_H264 is defined in config.h.
//
// Usage:
//   1. Call h264_encoder_init() once at startup
//   2. Call h264_encoder_encode() for each frame
//   3. Call h264_encoder_destroy() on shutdown
// ==============================================================================

#include "config.h"
#include "board_config.h"
#include <stdint.h>
#include <stddef.h>

// Encoder status codes
typedef enum {
    H264_OK = 0,
    H264_ERR_NOT_SUPPORTED,
    H264_ERR_INIT_FAILED,
    H264_ERR_ENCODE_FAILED,
    H264_ERR_NO_MEMORY,
    H264_ERR_INVALID_PARAM,
    H264_ERR_NOT_INITIALIZED
} h264_status_t;

// Frame type enumeration
typedef enum {
    H264_FRAME_TYPE_UNKNOWN = 0,
    H264_FRAME_TYPE_IDR,      // Keyframe (I-frame)
    H264_FRAME_TYPE_P         // Predicted frame
} h264_frame_type_t;

// Encoded frame output structure
typedef struct {
    uint8_t *data;            // Pointer to encoded H.264 data (NAL units)
    size_t   size;            // Size of encoded data in bytes
    h264_frame_type_t type;   // Frame type (IDR or P)
    uint32_t timestamp;       // Presentation timestamp
    bool     contains_sps_pps; // True if frame includes SPS/PPS
} h264_frame_t;

// Encoder configuration
typedef struct {
    uint16_t width;           // Frame width
    uint16_t height;          // Frame height
    uint8_t  fps;             // Target framerate
    uint32_t bitrate;         // Target bitrate in bps
    uint8_t  gop;             // GOP size (keyframe interval)
    uint8_t  qp_min;          // Minimum QP
    uint8_t  qp_max;          // Maximum QP
} h264_encoder_config_t;

#ifdef VIDEO_CODEC_H264

/**
 * @brief Initialize the H.264 encoder
 * @param config Encoder configuration parameters
 * @return H264_OK on success, error code otherwise
 */
h264_status_t h264_encoder_init(const h264_encoder_config_t *config);

/**
 * @brief Encode a raw frame to H.264
 * @param raw_data Pointer to raw pixel data (YUV420 or YUYV format)
 * @param raw_size Size of raw data in bytes
 * @param out_frame Output structure for encoded frame
 * @return H264_OK on success, error code otherwise
 */
h264_status_t h264_encoder_encode(const uint8_t *raw_data, size_t raw_size, h264_frame_t *out_frame);

/**
 * @brief Encode a JPEG frame to H.264 (converts JPEG to YUV first)
 * @param jpeg_data Pointer to JPEG data
 * @param jpeg_size Size of JPEG data in bytes
 * @param out_frame Output structure for encoded frame
 * @return H264_OK on success, error code otherwise
 * @note This is slower due to JPEG decode step. Use raw frames if possible.
 */
h264_status_t h264_encoder_encode_jpeg(const uint8_t *jpeg_data, size_t jpeg_size, h264_frame_t *out_frame);

/**
 * @brief Request an IDR (keyframe) for the next encode
 */
void h264_encoder_request_idr(void);

/**
 * @brief Get the current SPS (Sequence Parameter Set)
 * @param sps_data Output buffer for SPS data
 * @param sps_size Input: buffer size, Output: actual SPS size
 * @return H264_OK on success
 */
h264_status_t h264_encoder_get_sps(uint8_t *sps_data, size_t *sps_size);

/**
 * @brief Get the current PPS (Picture Parameter Set)
 * @param pps_data Output buffer for PPS data
 * @param pps_size Input: buffer size, Output: actual PPS size
 * @return H264_OK on success
 */
h264_status_t h264_encoder_get_pps(uint8_t *pps_data, size_t *pps_size);

/**
 * @brief Check if hardware encoder is available
 * @return true if HW encoder is available (ESP32-P4), false otherwise
 */
bool h264_encoder_is_hw(void);

/**
 * @brief Destroy the encoder and free resources
 */
void h264_encoder_destroy(void);

/**
 * @brief Get encoder type string for logging
 * @return "Hardware" or "Software"
 */
const char* h264_encoder_get_type_string(void);

#else // VIDEO_CODEC_H264 not defined - stub implementations

// Stub functions that compile to nothing when H.264 is disabled
static inline h264_status_t h264_encoder_init(const h264_encoder_config_t *config) { 
    (void)config;
    return H264_ERR_NOT_SUPPORTED; 
}
static inline h264_status_t h264_encoder_encode(const uint8_t *raw_data, size_t raw_size, h264_frame_t *out_frame) { 
    (void)raw_data; (void)raw_size; (void)out_frame;
    return H264_ERR_NOT_SUPPORTED; 
}
static inline h264_status_t h264_encoder_encode_jpeg(const uint8_t *jpeg_data, size_t jpeg_size, h264_frame_t *out_frame) { 
    (void)jpeg_data; (void)jpeg_size; (void)out_frame;
    return H264_ERR_NOT_SUPPORTED; 
}
static inline void h264_encoder_request_idr(void) {}
static inline h264_status_t h264_encoder_get_sps(uint8_t *sps_data, size_t *sps_size) { 
    (void)sps_data; (void)sps_size;
    return H264_ERR_NOT_SUPPORTED; 
}
static inline h264_status_t h264_encoder_get_pps(uint8_t *pps_data, size_t *pps_size) { 
    (void)pps_data; (void)pps_size;
    return H264_ERR_NOT_SUPPORTED; 
}
static inline bool h264_encoder_is_hw(void) { return false; }
static inline void h264_encoder_destroy(void) {}
static inline const char* h264_encoder_get_type_string(void) { return "Disabled"; }

#endif // VIDEO_CODEC_H264
