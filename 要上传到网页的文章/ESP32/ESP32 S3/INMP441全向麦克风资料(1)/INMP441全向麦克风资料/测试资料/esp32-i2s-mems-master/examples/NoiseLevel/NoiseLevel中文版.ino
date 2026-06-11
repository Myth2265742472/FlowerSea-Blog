/**
 * ESP32 I2S 噪声级别示例
 *
 * 此示例计算平均噪声级别。
 * 此示例属于公共领域。
 *
 * @author maspetsberger
 */

#include <driver/i2s.h>

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int BLOCK_SIZE = 1024;

void setup() {
  Serial.begin(115200);
  Serial.println("正在配置 I2S...");
  esp_err_t err;

  // I2S 配置（根据示例）
  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // 接收模式，非传输模式
      .sample_rate = 16000,                         // 采样率 16KHz
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // 只能使用 32 位采样
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // 虽然 SEL 配置应该是左声道，但它似乎在右声道传输
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // 中断级别 1
      .dma_buf_count = 8,                           // 缓冲区数量
      .dma_buf_len = BLOCK_SIZE                     // 每个缓冲区的采样数
  };

  // 引脚配置（根据硬件连接）
  const i2s_pin_config_t pin_config = {
      .bck_io_num = 14,   // 位时钟（BCKL）
      .ws_io_num = 15,    // 左右声道时钟（LRCL）
      .data_out_num = -1, // 未使用（仅用于扬声器）
      .data_in_num = 32   // 数据输入（DOUT）
  };

  // 配置 I2S 驱动器和引脚。
  // 此函数必须在任何 I2S 驱动器读/写操作之前调用。
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("驱动器安装失败: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("引脚设置失败: %d\n", err);
    while (true);
  }
  Serial.println("I2S 驱动器已安装。");
}

void loop() {

  // 一次读取多个采样并计算声压级
  int32_t samples[BLOCK_SIZE];
  int num_bytes_read = i2s_read_bytes(I2S_PORT,
                                      (char *)samples,
                                      BLOCK_SIZE,     // 文档说是字节，但实际是元素数量
                                      portMAX_DELAY); // 无超时

  int samples_read = num_bytes_read / 8;
  if (samples_read > 0) {

    float mean = 0;
    for (int i = 0; i < samples_read; ++i) {
      mean += samples[i];
    }
    Serial.println(mean);  // 输出平均噪声级别
  }
}

// 实际上退出时应该调用 `i2s_driver_uninstall(I2S_PORT)`。
