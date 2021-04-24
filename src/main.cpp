#include <string>
#include <iostream>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "IREosStrobe.h"

static const char *TAG = "IR Flash";

Take4::IREosStrobe strobe(GPIO_NUM_12, RMT_CHANNEL_0);

extern "C" void app_main()
{
    ESP_LOGI(TAG, "Configuring transmitter");
    strobe.begin();

    std::string buffer;
    for (;;) {
        auto ch = getchar();
        if (ch != EOF) {
            if (iscntrl(ch)) {
                if (!buffer.empty()) {
                    char *pos;
                    auto val = strtol(buffer.c_str(), &pos, 10);
                    if (val >= 0) {
                        printf("\nFlash %ld\n", val);
                        // プリ発光データは入力した値
                        strobe.set(Take4::IREosStrobe::ETTLPreFlash);
                        strobe.set(val);
                        strobe.sendParameter();

                        vTaskDelay(12 / portTICK_PERIOD_MS);
                        strobe.flash();
                        vTaskDelay(40 / portTICK_PERIOD_MS);

                        strobe.set(Take4::IREosStrobe::ETTLFlash);
                        // 本発光データはとりあえず固定
                        strobe.set(13);
                        strobe.sendParameter();
                        vTaskDelay(12 / portTICK_PERIOD_MS);
                        strobe.flash();
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