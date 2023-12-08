#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(OLED_RESET);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);
EthernetServer tcpServer(80);
EthernetClient tcpClient;
EthernetUDP udp;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  Ethernet.begin(mac, ip);
  tcpServer.begin();
  udp.begin(8888);
}

void loop() {
  checkTCPClient();
  checkUDPPacket();
}

void displayCommand(const char* cmd) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(cmd);
  display.display();
}

void displayResult(const char* result) {
  display.fillRect(0, 10, OLED_WIDTH, 10, BLACK);
  display.setCursor(0, 10);
  display.print(result);
  display.display();
}

void checkTCPClient() {
  if (!tcpClient.connected()) {
    tcpClient = tcpServer.available();
    return;
  }

  if (tcpClient.available()) {
    String command = tcpClient.readStringUntil('\n');
    command.trim();

    displayCommand(command.c_str());

    if (command == "GetTemp") {
      float temperature = readTemperature();
      String result = "Temperature: " + String(temperature) + "C";
      displayResult(result.c_str());
      tcpClient.print(result);
    }
    // 处理其他命令

    tcpClient.println();
    tcpClient.stop();
  }
}

void checkUDPPacket() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
    int len = udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    if (len > 0) {
      packetBuffer[len] = 0;
    }

    String command(packetBuffer);

    displayCommand(command.c_str());

    if (command == "GetTemp") {
      float temperature = readTemperature();
      String result = "Temperature: " + String(temperature) + "C";
      displayResult(result.c_str());
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.print(result);
      udp.endPacket();
    }
    // 处理其他命令
  }
}

float readTemperature() {
  // 通过 I2C 接口读取温度传感器的温度数据，并返回该数值
}

// 控制其他硬件函数的实现