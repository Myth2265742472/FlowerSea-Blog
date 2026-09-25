# ESP32 I2S MEMS 麦克风 Arduino IDE 示例

本仓库包含一些将 I2S MEMS 麦克风连接到 ESP32 开发板的示例代码。

起初我以为连接 I2S 麦克风会很直接，但看起来 I2S 是一个相对较新或被忽视的接口。Adafruit 提供的示例仅适用于 Feather M0；ESP32 自带的通用 I2S 示例并不能直接应用。因此，我创建了此仓库。

## 本示例中使用的组件

我使用了以下组件：
* [adafruit-huzzah32-esp32-feather](https://www.adafruit.com/product/3405)
* [adafruit-i2s-mems-microphone-breakout](https://www.adafruit.com/product/3421)

## 如何连接组件？

ESP32 上没有专用的 I2S 引脚。相反，您需要在代码中配置/启用引脚。最终，我将 I2S-MEMS 引脚连接到以下 ESP32 引脚：
* SEL 悬空，即单声道，实际上使用左声道
* LRCL 连接到 #15
* DOUT 连接到 #32
* BCKL 连接到 #14
* GND 连接到 GND
* 3V 连接到 3V

**不要尝试将它们连接到名称相似的 SCL/SCA/SCK，这些是用于不兼容的 I2C 接口的。**

## 如何使用这些示例？

您可以在 Arduino IDE 中将每个示例作为草图打开。我通常使用的波特率是 `115200`，您需要在串口监视器中配置该波特率，否则会显示乱码。

## 注意事项

* 要么是 `SEL` 配置错误，要么是 ESP32 的 I2S 通道处理问题。
  * 当 `SEL` 引脚悬空或接地时，我必须使用 `I2S_CHANNEL_FMT_ONLY_RIGHT`，而当 `SEL` 为高电平时，也使用 `I2S_CHANNEL_FMT_ONLY_RIGHT`。
* 我只能使用 32 位采样才能让它工作。
  * 我不知道这是硬件限制还是我配置错误。
* 虽然设置对噪声有很好的反应。
  * 我不知道记录的数据是否真的是有效的声音。
  * 可能还需要进行一些位操作。

## 致谢

感谢 Adafruit 提供易于使用的硬件和不错的指南（即使这里不完全适用，它仍然是初学者的好起点），以及 [espressif 示例](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2s)。
