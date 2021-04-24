#pragma once
#include <driver/gpio.h>
#include <driver/rmt.h>

namespace Take4
{
    // 赤外線でEOSストロボを制御するクラス
    // setでパラメータを設定後、sendParameter, flashを組み合わせてストロボを発光させる
    class IREosStrobe {
    public:
        // 発光種類
        enum FlashType {
            Manual,
            ETTLPreFlash,
            ETTLFlash,
        };
    
    private:
        gpio_num_t pin_;
        rmt_channel_t channel_;

        uint8_t value_;
        FlashType type_;

    public:
        IREosStrobe(gpio_num_t pin, rmt_channel_t channel);

        ~IREosStrobe();

        // 初期化関数
        void begin();

        // 光量の設定(0～31までが有効)
        void set(uint8_t lightIntensity);

        // パラメータの種別
        void set(FlashType type);

        // パラメータの送信
        void sendParameter() const;

        // 発光信号の送信
        void flash() const;
    };
} // namespace Take4
