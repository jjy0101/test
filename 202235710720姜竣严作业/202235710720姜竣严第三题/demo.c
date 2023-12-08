#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringSerial.h>
#include "ssd1306.h" // OLED显示屏驱动库

#define OLED_I2C_ADDR 0x3C // OLED显示屏的I2C地址
#define SENSOR_ADDR 0x01   // 温湿度传感器的I2C地址
#define SERIAL_PORT "/dev/ttyAMA0" // UART串口设备地址

int main() {
    // 初始化I2C接口和OLED显示屏
    int i2c_fd = wiringPiI2CSetup(OLED_I2C_ADDR);
    ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, i2c_fd);

    // 打开UART串口
    int serial_fd = serialOpen(SERIAL_PORT, 9600);

    // 主循环
    while (1) {
        // 从温湿度传感器读取数据（假设数据格式为32位的温度和湿度值）
        uint32_t data = wiringPiI2CReadReg32(SENSOR_ADDR, 0x00);

        // 解析温度和湿度值
        float temperature = (float)((data >> 16) & 0xFFFF) / 100.0;
        float humidity = (float)(data & 0xFFFF) / 100.0;

        // 将温度和湿度值打印到终端
        printf("Temperature: %.2f °C, Humidity: %.2f %%\n", temperature, humidity);

        // 将温度和湿度值显示到OLED显示屏上
        char message[64];
        snprintf(message, sizeof(message), "Temp: %.2fC", temperature);
        ssd1306_clearDisplay(i2c_fd);
        ssd1306_drawString(i2c_fd, 0, 0, message);
        snprintf(message, sizeof(message), "Humidity: %.2f%%", humidity);
        ssd1306_drawString(i2c_fd, 0, 16, message);
        ssd1306_display(i2c_fd);

        // 延时一段时间
        delay(1000);
    }

    // 关闭UART串口和I2C接口
    serialClose(serial_fd);
    close(i2c_fd);

    return 0;
}