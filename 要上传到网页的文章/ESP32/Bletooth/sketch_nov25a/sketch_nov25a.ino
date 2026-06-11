#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// 定义服务和特征的 UUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

// 连接状态回调类
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("设备已连接");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("设备已断开");
      // 断开后重新开启广播
      pServer->getAdvertising()->start();
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("启动ESP32蓝牙...");

  // 创建BLE设备
  BLEDevice::init("ESP32-BLE");

  // 创建BLE服务器
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 创建BLE服务
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 创建BLE特征
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // 添加描述符
  pCharacteristic->addDescriptor(new BLE2902());

  // 启动服务
  pService->start();

  // 开始广播
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("ESP32蓝牙已启动，等待连接...");
}

void loop() {
  if (deviceConnected) {
    // 当设备连接时，可以在这里添加你想要的操作
    // 例如发送数据给手机
    String message = "Hello from ESP32";
    pCharacteristic->setValue(message.c_str());
    pCharacteristic->notify();
    delay(2000);
  }
  delay(1000);
}