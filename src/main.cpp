#include <string>
#include <iostream>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"

static const char *TAG = "IR Flash";

static constexpr rmt_channel_t RMT_TX_CHANNEL = RMT_CHANNEL_0;
static constexpr gpio_num_t CONFIG_EXAMPLE_RMT_TX_GPIO = GPIO_NUM_12;

static constexpr size_t PatternBufferSize = 20;
static constexpr size_t ValueHeaderPosition = 15;
static rmt_item32_t patternBuffer[PatternBufferSize] = {
    // 先頭データ11ビット分
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 0, 40, 0 }}},
    {{{ 60, 0, 40, 0 }}},
    {{{ 60, 0, 40, 0 }}},
    {{{ 60, 0, 40, 0 }}},
    // 光量データ9ビット分
    {{{ 60, 0, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 0, 40, 0 }}},
    {{{ 60, 0, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}},
    {{{ 60, 1, 40, 0 }}}
};

/*
 * Initialize the RMT Tx channel
 */
static void rmt_tx_init(void)
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(CONFIG_EXAMPLE_RMT_TX_GPIO, RMT_TX_CHANNEL);
    config.tx_config.carrier_en = false;
    config.clk_div = 80;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}

// 発光量を1～31で表す。
// 計測した結果では、31がフル発光。3が1/128発光だった。

extern "C" void app_main()
{
    ESP_LOGI(TAG, "Configuring transmitter");
    rmt_tx_init();

    std::string buffer;
    for (;;) {
        auto ch = getchar();
        if (ch != EOF) {
            if (iscntrl(ch)) {
                if (!buffer.empty()) {
                    char *pos;
                    auto val = strtol(buffer.c_str(), &pos, 10);
                    if (val >= 0 && val < 100) {
                        printf("\nFlash %ld\n", val);
                        uint8_t work = 2;
                        for (size_t i = ValueHeaderPosition; i < PatternBufferSize; i++) {
                            patternBuffer[i].level0 = val & 0x1;
                            work = work >> 1;
                        }
                        ESP_ERROR_CHECK(rmt_write_items(RMT_TX_CHANNEL, patternBuffer, PatternBufferSize, true));
                        vTaskDelay(val / portTICK_PERIOD_MS);
                        rmt_write_items(RMT_TX_CHANNEL, patternBuffer, 1, true);
                    }
                    else {
                        printf("\nIllegal Val %ld\n", val);
                    }
                    buffer.clear();
                }
            }
            else {
                putchar(ch);
                buffer += (char)ch;
            }
        }
    }
}