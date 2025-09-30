#include "wifi_board.h"
#include "codecs/es8311_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "led/single_led.h"
#include "esp32_camera.h"

#include <wifi_station.h>
#include <esp_log.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include <driver/ledc.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_st7796.h>

typedef struct {
    int cmd;
    const void* data;
    size_t data_bytes;
    unsigned int delay_ms;
} st7796_lcd_init_cmd_t;

typedef struct {
    const st7796_lcd_init_cmd_t* init_cmds;
    uint16_t init_cmds_size;
} st7796_vendor_config_t;

static const st7796_lcd_init_cmd_t kSt7796InitCmds[] = {
    {0x11, nullptr, 0, 120},
    {0x36, (uint8_t[]){0x48}, 1, 0},
    {0x3A, (uint8_t[]){0x55}, 1, 0},
    {0xF0, (uint8_t[]){0xC3}, 1, 0},
    {0xF0, (uint8_t[]){0x96}, 1, 0},
    {0xB4, (uint8_t[]){0x01}, 1, 0},
    {0xB7, (uint8_t[]){0xC6}, 1, 0},
    {0xB9, (uint8_t[]){0x02, 0xE0}, 2, 0},
    {0xC0, (uint8_t[]){0xF0, 0x54}, 2, 0},
    {0xC1, (uint8_t[]){0x15}, 1, 0},
    {0xC2, (uint8_t[]){0xAF}, 1, 0},
    {0xC5, (uint8_t[]){0x1C}, 1, 0},
    {0xE7, (uint8_t[]){0x27, 0x02, 0x42, 0xB5, 0x05}, 5, 0},
    {0xE8, (uint8_t[]){0x40, 0x8A, 0x00, 0x00, 0x29, 0x19, 0xA5, 0x33}, 8, 0},
    {0xE0, (uint8_t[]){0xD0, 0x0A, 0x00, 0x1B, 0x15, 0x27, 0x33, 0x44, 0x48, 0x17, 0x14, 0x15, 0x2C, 0x31}, 14, 0},
    {0xE1, (uint8_t[]){0xD0, 0x14, 0x00, 0x1F, 0x13, 0x0B, 0x32, 0x43, 0x47, 0x38, 0x12, 0x12, 0x2A, 0x32}, 14, 0},
    {0xF0, (uint8_t[]){0x3C}, 1, 0},
    {0xF0, (uint8_t[]){0x69}, 1, 120},
};

#define TAG "XiaomouBoard"

class XiaomouBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t codec_i2c_bus_{};
    Button boot_button_;
    Display* display_{};
    Esp32Camera* camera_{};

    void InitializeCodecI2c() {
        i2c_master_bus_config_t cfg = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &codec_i2c_bus_));
    }

    void InitializeSpi() {
        spi_bus_config_t bus_cfg = {};
        bus_cfg.mosi_io_num = DISPLAY_SPI_MOSI_PIN;
        bus_cfg.miso_io_num = GPIO_NUM_NC;
        bus_cfg.sclk_io_num = DISPLAY_SPI_SCLK_PIN;
        bus_cfg.quadwp_io_num = GPIO_NUM_NC;
        bus_cfg.quadhd_io_num = GPIO_NUM_NC;
        bus_cfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO));
    }

    void InitializeDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_io_spi_config_t io_cfg = {};
        io_cfg.cs_gpio_num = DISPLAY_SPI_CS_PIN;
        io_cfg.dc_gpio_num = DISPLAY_SPI_DC_PIN;
        io_cfg.spi_mode = 0;
        io_cfg.pclk_hz = DISPLAY_SPI_SCLK_HZ;
        io_cfg.trans_queue_depth = 10;
        io_cfg.lcd_cmd_bits = 8;
        io_cfg.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_cfg, &panel_io));

        esp_lcd_panel_handle_t panel = nullptr;
        const st7796_vendor_config_t vendor_cfg = {
            .init_cmds = kSt7796InitCmds,
            .init_cmds_size = sizeof(kSt7796InitCmds) / sizeof(kSt7796InitCmds[0]),
        };

        esp_lcd_panel_dev_config_t panel_cfg = {};
        panel_cfg.reset_gpio_num = DISPLAY_SPI_RESET_PIN;
        panel_cfg.rgb_endian = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_cfg.bits_per_pixel = 16;
        panel_cfg.vendor_config = &vendor_cfg;
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(panel_io, &panel_cfg, &panel));

        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR));
        ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY));
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

        display_ = new SpiLcdDisplay(panel_io, panel,
            DISPLAY_WIDTH, DISPLAY_HEIGHT,
            DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
            DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    void InitializeCamera() {
        camera_config_t config = {};
        config.pin_pwdn = CAMERA_PIN_PWDN;
        config.pin_reset = CAMERA_PIN_RESET;
        config.pin_xclk = CAMERA_PIN_XCLK;
        config.pin_sccb_sda = CAMERA_PIN_SIOD;
        config.pin_sccb_scl = CAMERA_PIN_SIOC;
        config.pin_d7 = CAMERA_PIN_D7;
        config.pin_d6 = CAMERA_PIN_D6;
        config.pin_d5 = CAMERA_PIN_D5;
        config.pin_d4 = CAMERA_PIN_D4;
        config.pin_d3 = CAMERA_PIN_D3;
        config.pin_d2 = CAMERA_PIN_D2;
        config.pin_d1 = CAMERA_PIN_D1;
        config.pin_d0 = CAMERA_PIN_D0;
        config.pin_vsync = CAMERA_PIN_VSYNC;
        config.pin_href = CAMERA_PIN_HREF;
        config.pin_pclk = CAMERA_PIN_PCLK;
        config.xclk_freq_hz = XCLK_FREQ_HZ;
        config.ledc_timer = LEDC_TIMER_0;
        config.ledc_channel = LEDC_CHANNEL_0;
        config.pixel_format = PIXFORMAT_RGB565;
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        config.sccb_i2c_port = 0;
        camera_ = new Esp32Camera(config);
        camera_->SetHMirror(false);
        camera_->SetVFlip(false);
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
    }

public:
    XiaomouBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeCodecI2c();
        InitializeSpi();
        InitializeDisplay();
        InitializeCamera();
        InitializeButtons();
        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            GetBacklight()->RestoreBrightness();
        }
    }

    virtual Led* GetLed() override {
        static SingleLed led(BUILTIN_LED_GPIO);
        return &led;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }

    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }

    virtual Camera* GetCamera() override {
        return camera_;
    }

    virtual AudioCodec* GetAudioCodec() override {
        static Es8311AudioCodec audio_codec(
            codec_i2c_bus_, I2C_NUM_0,
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS,
            AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, AUDIO_CODEC_ES8311_ADDR);
        return &audio_codec;
    }
};

DECLARE_BOARD(XiaomouBoard);
