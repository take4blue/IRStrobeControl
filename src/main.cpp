#include <string>
#include <iostream>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"

static const char *TAG = "example";

static constexpr rmt_channel_t RMT_TX_CHANNEL = RMT_CHANNEL_0;
static constexpr gpio_num_t CONFIG_EXAMPLE_RMT_TX_GPIO = GPIO_NUM_12;

/*
 * Prepare a raw table with a message in the Morse code
 *
 * The message is "ESP" : . ... .--.
 *
 * The table structure represents the RMT item structure:
 * {duration, level, duration, level}
 *
 */
static const rmt_item32_t morse_esp[] = {
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
    {{{ 60, 0, 40, 0 }}},
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
                    auto val = strtol(buffer.c_str(), &pos, 40);
                    if (val >= 0 && val < 60000) {
                        printf("\nFlash %ld\n", val);
                        ESP_ERROR_CHECK(rmt_write_items(RMT_TX_CHANNEL, morse_esp, sizeof(morse_esp) / sizeof(morse_esp[0]), true));
                        vTaskDelay(50 / portTICK_PERIOD_MS);
                        rmt_write_items(RMT_TX_CHANNEL, morse_esp, 1, true);
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