/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <driver/gpio.h>
#include <fcntl.h>
#include <inttypes.h>
#include <memory>
#include <mutex>
#include <mooncake_log.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <vector>

#include "../utils/task_controller/task_controller.h"
#include "bsp/esp-bsp.h"
#include "driver/i2c_master.h"
#include "driver/ppa.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_video_device.h"
#include "esp_video_init.h"
#include "frame_buffer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "hal/hal_esp32.h"
#include "imlib.h"
#include "linux/videodev2.h"

#define CAMERA_WIDTH  1280
#define CAMERA_HEIGHT 720

static lv_obj_t* camera_canvas;
// extern uint8_t* frame_buf;
static QueueHandle_t queue_camera_ctrl = NULL;
// 定义任务控制标志
#define TASK_CONTROL_PAUSE  0
#define TASK_CONTROL_RESUME 1
#define TASK_CONTROL_EXIT   2

static bool       is_camera_capturing = false;
static std::mutex camera_mutex;

static const char* TAG = "camera";

#define EXAMPLE_VIDEO_BUFFER_COUNT 2
#define MEMORY_TYPE                V4L2_MEMORY_MMAP
#define CAM_DEV_PATH               ESP_VIDEO_MIPI_CSI_DEVICE_NAME
#ifndef ARRAY_SIZE
#    define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

typedef struct cam
{
    int      fd;
    uint32_t width;
    uint32_t height;
    uint32_t pixel_format;
    uint8_t* buffer[EXAMPLE_VIDEO_BUFFER_COUNT];
} cam_t;

/*
 * The image format type definition used in the example.
 */
typedef enum
{
    EXAMPLE_VIDEO_FMT_RAW8   = V4L2_PIX_FMT_SBGGR8,
    EXAMPLE_VIDEO_FMT_RAW10  = V4L2_PIX_FMT_SBGGR10,
    EXAMPLE_VIDEO_FMT_GREY   = V4L2_PIX_FMT_GREY,
    EXAMPLE_VIDEO_FMT_RGB565 = V4L2_PIX_FMT_RGB565,
    EXAMPLE_VIDEO_FMT_RGB888 = V4L2_PIX_FMT_RGB24,
    EXAMPLE_VIDEO_FMT_YUV422 = V4L2_PIX_FMT_YUV422P,
    EXAMPLE_VIDEO_FMT_YUV420 = V4L2_PIX_FMT_YUV420,
} example_fmt_t;

static bool set_sensor_control(int fd, uint32_t control_id, int32_t value, const char* control_name)
{
    struct v4l2_control control = {};
    control.id                  = control_id;
    control.value               = value;

    if (ioctl(fd, VIDIOC_S_CTRL, &control) == 0)
    {
        ESP_LOGI(TAG,
                 "Set %s (0x%08" PRIx32 ") to %" PRId32,
                 control_name != NULL ? control_name : "control",
                 control_id,
                 value);
        return true;
    }

    ESP_LOGW(TAG,
             "Failed to set %s (0x%08" PRIx32 ") to %" PRId32 ": %s",
             control_name != NULL ? control_name : "control",
             control_id,
             value,
             strerror(errno));
    return false;
}

/**
 * @brief   Open the video device and initialize the video device to use `init_fmt` as the output
 * format.
 * @note    When the sensor outputs data in RAW format, the ISP module can interpolate its data into
 * RGB or YUV format. However, when the sensor works in RGB or YUV format, the output data can only
 * be in RGB or YUV format.
 * @param dev device name(eg, "/dev/video0")
 * @param init_fmt output format.
 *
 * @return
 *     - Device descriptor   Success
 *     - -1 error
 */
int app_video_open(char* dev, example_fmt_t init_fmt)
{
    struct v4l2_format     default_format;
    struct v4l2_capability capability;
    const int              type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int fd = open(dev, O_RDONLY);
    if (fd < 0)
    {
        ESP_LOGE(TAG, "Open video failed");
        return -1;
    }

    if (ioctl(fd, VIDIOC_QUERYCAP, &capability))
    {
        ESP_LOGE(TAG, "failed to get capability");
        goto exit_0;
    }

    ESP_LOGI(TAG,
             "version: %d.%d.%d",
             (uint16_t)(capability.version >> 16),
             (uint8_t)(capability.version >> 8),
             (uint8_t)capability.version);
    ESP_LOGI(TAG, "driver:  %s", capability.driver);
    ESP_LOGI(TAG, "card:    %s", capability.card);
    ESP_LOGI(TAG, "bus:     %s", capability.bus_info);

    memset(&default_format, 0, sizeof(struct v4l2_format));
    default_format.type = type;
    if (ioctl(fd, VIDIOC_G_FMT, &default_format) != 0)
    {
        ESP_LOGE(TAG, "failed to get format");
        goto exit_0;
    }

    ESP_LOGI(TAG,
             "width=%" PRIu32 " height=%" PRIu32,
             default_format.fmt.pix.width,
             default_format.fmt.pix.height);

    if (default_format.fmt.pix.pixelformat != init_fmt)
    {
        struct v4l2_format format = {.type = type,
                                     .fmt  = {.pix = {.width       = default_format.fmt.pix.width,
                                                      .height      = default_format.fmt.pix.height,
                                                      .pixelformat = init_fmt}}};

        if (ioctl(fd, VIDIOC_S_FMT, &format) != 0)
        {
            ESP_LOGE(TAG, "failed to set format");
            goto exit_0;
        }
    }

    return fd;
exit_0:
    close(fd);
    return -1;
}

static esp_err_t new_cam(int cam_fd, cam_t** ret_wc)
{
    int                        ret;
    struct v4l2_format         format;
    int                        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    struct v4l2_requestbuffers req;
    cam_t*                     wc;

    memset(&format, 0, sizeof(struct v4l2_format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(cam_fd, VIDIOC_G_FMT, &format) != 0)
    {
        ESP_LOGE(TAG, "Failed get fmt");
        return ESP_FAIL;
    }

    wc = (cam_t*)malloc(sizeof(cam_t));
    if (!wc)
    {
        return ESP_ERR_NO_MEM;
    }

    wc->fd           = cam_fd;
    wc->width        = format.fmt.pix.width;
    wc->height       = format.fmt.pix.height;
    wc->pixel_format = format.fmt.pix.pixelformat;

    memset(&req, 0, sizeof(req));
    req.count  = ARRAY_SIZE(wc->buffer);
    req.type   = type;
    req.memory = MEMORY_TYPE;
    if (ioctl(wc->fd, VIDIOC_REQBUFS, &req) != 0)
    {
        ESP_LOGE(TAG, "failed to req buffers");
        ret = ESP_FAIL;
        goto errout;
    }

    for (int i = 0; i < ARRAY_SIZE(wc->buffer); i++)
    {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type   = type;
        buf.memory = MEMORY_TYPE;
        buf.index  = i;
        if (ioctl(wc->fd, VIDIOC_QUERYBUF, &buf) != 0)
        {
            ESP_LOGE(TAG, "failed to query buffer");
            ret = ESP_FAIL;
            goto errout;
        }

        wc->buffer[i] = (uint8_t*)mmap(
            NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, wc->fd, buf.m.offset);
        if (!wc->buffer[i])
        {
            ESP_LOGE(TAG, "failed to map buffer");
            ret = ESP_FAIL;
            goto errout;
        }

        if (ioctl(wc->fd, VIDIOC_QBUF, &buf) != 0)
        {
            ESP_LOGE(TAG, "failed to queue frame buffer");
            ret = ESP_FAIL;
            goto errout;
        }
    }

    if (ioctl(wc->fd, VIDIOC_STREAMON, &type))
    {
        ESP_LOGE(TAG, "failed to start stream");
        ret = ESP_FAIL;
        goto errout;
    }

    *ret_wc = wc;
    return ESP_OK;

errout:
    free(wc);
    return ret;
}

// static HumanFaceDetect* human_face_detector;
static bool   cam_is_initial = false;
static cam_t* camera         = NULL;

void app_camera_display(void* arg)
{
    /* camera config */
    static esp_video_init_csi_config_t csi_config = {
        .sccb_config =
            {
                .init_sccb  = false,
                .i2c_handle = NULL,
                .freq       = 400000,  // TAB5_MIPI_CSI_SCCB_I2C_FREQ,
            },
        .reset_pin = -1,  // TAB5_MIPI_CSI_CAM_SENSOR_RESET_PIN,
        .pwdn_pin  = -1,  // TAB5_MIPI_CSI_CAM_SENSOR_PWDN_PIN,
    };
    csi_config.sccb_config.i2c_handle = bsp_i2c_get_handle();

    esp_video_init_config_t cam_config = {
        .csi  = &csi_config,  // Point to CSI config
        .dvp  = NULL,         // No DVP configuration
        .jpeg = NULL,         // No JPEG configuration
    };

    if (!cam_is_initial)
    {
        camera = (cam_t*)malloc(sizeof(cam_t));
        printf("\n============= video init ==============\n");
        cam_is_initial = true;
        ESP_ERROR_CHECK(esp_video_init(&cam_config));
        printf("\n============= video open ==============\n");
        int video_cam_fd = app_video_open(CAM_DEV_PATH, EXAMPLE_VIDEO_FMT_RGB565);
        if (video_cam_fd < 0)
        {
            ESP_LOGE(TAG, "video cam open failed");
            return;
        }
        ESP_ERROR_CHECK(new_cam(video_cam_fd, &camera));
        bool sensor_hflip_enabled =
            set_sensor_control(video_cam_fd, V4L2_CID_HFLIP, 1, "sensor horizontal flip");
        if (!sensor_hflip_enabled)
        {
            ESP_LOGW(TAG, "Sensor horizontal flip unsupported; the preview may remain mirrored");
        }
        set_sensor_control(video_cam_fd, V4L2_CID_VFLIP, 0, "sensor vertical flip");
    }

    struct v4l2_buffer buf;

    /* */
    uint16_t           screen_width         = 1280;  // 640;//lcd_height();
    uint16_t           screen_height        = 720;   // 480;//lcd_width();
    uint8_t*           img_show_data        = NULL;
    constexpr uint32_t kRgb565BytesPerPixel = 2;
    uint32_t           img_show_size        = screen_width * screen_height * kRgb565BytesPerPixel;
    // uint32_t img_offset = 280 * 720 * 2;
    heap_caps_free(img_show_data);
    img_show_data = NULL;
    }
    // close(camera->fd);

    camera_mutex.lock();
    is_camera_capturing = false;
    camera_mutex.unlock();

    vTaskDelete(NULL);
}

void HalEsp32::startCameraCapture(lv_obj_t* imgCanvas)
{
    mclog::tagInfo(TAG, "start camera capture");

    camera_canvas = imgCanvas;

    queue_camera_ctrl = xQueueCreate(10, sizeof(int));
    if (queue_camera_ctrl == NULL)
    {
        ESP_LOGD(TAG, "Failed to create semaphore\n");
    }

    is_camera_capturing = true;
    xTaskCreatePinnedToCore(app_camera_display, "cam", 8 * 1024, NULL, 5, NULL, 1);
}

void HalEsp32::stopCameraCapture()
{
    mclog::tagInfo(TAG, "stop camera capture");

    int control_state = 0;  // pause
    xQueueSend(queue_camera_ctrl, &control_state, portMAX_DELAY);

    control_state = 2;  // exit
    xQueueSend(queue_camera_ctrl, &control_state, portMAX_DELAY);
}

bool HalEsp32::isCameraCapturing()
{
    std::lock_guard<std::mutex> lock(camera_mutex);
    return is_camera_capturing;
}
