#include "IREosStrobe.h"

using namespace Take4;

static constexpr size_t PatternBufferSize = 20;
static constexpr size_t ValueBitPosition = 15;
static constexpr uint32_t HighDuration = 60;
static constexpr uint32_t LowDuration = 40;

IREosStrobe::IREosStrobe(gpio_num_t pin, rmt_channel_t channel)
: pin_(pin)
, channel_(channel)
, value_(0)
, type_(FlashType::Manual)
{    
}

IREosStrobe::~IREosStrobe()
{    
}

// 初期化関数
void IREosStrobe::begin()
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(pin_, channel_);
    config.tx_config.carrier_en = false;
    config.clk_div = 80;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}

// 光量の設定
void IREosStrobe::set(uint8_t lightIntensity)
{
    value_ = lightIntensity & 0b11111;
}

// パラメータの種別
void IREosStrobe::set(FlashType type)
{
    type_ = type;
}

// パラメータの送信
// バッファにはLSBファーストで詰めていく
void IREosStrobe::sendParameter() const
{
    rmt_item32_t buffer[PatternBufferSize];
    uint32_t sendValue = value_ << ValueBitPosition;
    switch (type_)
    {
    case FlashType::Manual:
        //             98765432109876543210
        sendValue |= 0b00000001100000111111;
        break;
    case FlashType::ETTLPreFlash:
        //             98765432109876543210
        sendValue |= 0b00000000100011000011;
        break;
    case FlashType::ETTLFlash:
        //             98765432109876543210
        sendValue |= 0b00000010100011110011;
        break;
    default:
        break;
    }

    for (int i = 0; i < PatternBufferSize; i++) {
        buffer[i].duration0 = HighDuration;
        buffer[i].duration1 = LowDuration;
        buffer[i].level1 = 0;
        buffer[i].level0 = sendValue & 0b1;
        sendValue = sendValue >> 1;
    }

    rmt_write_items(channel_, buffer, PatternBufferSize, true);
}

// 発光信号の送信
void IREosStrobe::flash() const
{
    rmt_item32_t buffer = {
        HighDuration, 1, LowDuration, 0
    };
    rmt_write_items(channel_, &buffer, 1, true);
}
