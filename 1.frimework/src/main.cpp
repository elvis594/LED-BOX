

#include <Arduino.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
BLECharacteristic *pCharacteristic = NULL;
BLEServer *pServer = NULL;

Preferences preferences;

// led part
#ifdef USING_C3_BOARD
// C3 专用逻辑
#define LED_PIN 4
#else
// 普通 ESP32
#define LED_PIN 13
#endif
#define LED_COUNT 7
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int breathDirection = 5;

struct State
{
    bool isLightOn;
    bool isBreathing;
    uint8_t r, g, b;
    uint8_t brightness;
    uint8_t breathBrightness;
    uint16_t periodMs; // 频率
};

static State currentState = {true, false, 255, 255, 255, 128, 128, 5000};
static State lastState = currentState;
void sendCurrentState();

void loadState()
{
    preferences.begin("app", true);

    currentState.isLightOn = preferences.getBool("isLightOn", currentState.isLightOn);

    currentState.isBreathing = preferences.getBool("isBreathing", currentState.isBreathing);

    currentState.r = preferences.getUInt("Red", currentState.r);

    currentState.g = preferences.getUInt("Green", currentState.g);

    currentState.b = preferences.getUInt("Blue", currentState.b);

    currentState.brightness = preferences.getUInt("Brightness", currentState.brightness);

    currentState.breathBrightness = preferences.getInt("Breath_lvl", currentState.breathBrightness);
    currentState.periodMs = preferences.getUInt("Period", currentState.periodMs);
    preferences.end();
}

void dumpNVS()
{
    preferences.begin("app", true);
    Serial.println("[NVS] Current State:");
    Serial.printf("isLightOn: %d\n", preferences.getBool("isLightOn", currentState.isLightOn));
    Serial.printf("isBreathing: %d\n", preferences.getBool("isBreathing", currentState.isBreathing));
    Serial.printf("RGB: (%d, %d, %d)\n", preferences.getUInt("Red", currentState.r), preferences.getUInt("Green", currentState.g), preferences.getUInt("Blue", currentState.b));
    Serial.printf("Brightness: %d\n", preferences.getUInt("Brightness", currentState.brightness));
    Serial.printf("BreathBrightness: %d\n", preferences.getInt("Breath_lvl", currentState.breathBrightness));
    Serial.printf("Breath_period: %d\n", preferences.getUInt("Period", currentState.periodMs));
    preferences.end();
}

void saveState()
{
    if(memcmp(&lastState, &currentState, sizeof(State)) == 0){Serial.println("[NVS] IS Same."); return ;}
    
    preferences.begin("app", false);
    preferences.putBool("isLightOn", currentState.isLightOn);
    preferences.putBool("isBreathing", currentState.isBreathing);
    preferences.putUInt("Brightness", currentState.brightness);
    preferences.putInt("Breath_lvl", currentState.breathBrightness);
    preferences.putUInt("Red", currentState.r);
    preferences.putUInt("Green", currentState.g);
    preferences.putUInt("Blue", currentState.b);
    preferences.putUInt("Period", currentState.periodMs);
    Serial.println("[NVS] saved.");
    // dumpNVS();
    preferences.end();

    lastState = currentState;
}

void setBrightness(uint8_t value)
{
    currentState.brightness = value;
}

void enableBreathing()
{
    currentState.isBreathing = true;
}

void clearLight()
{
    pixels.clear();
    pixels.show();
}
void applyColor()
{
    for (int i = 0; i < LED_COUNT; i++)
    {
        pixels.setPixelColor(i, pixels.Color(currentState.r, currentState.g, currentState.b));
    }
    pixels.setBrightness(currentState.brightness);
    pixels.show();
}

// ---- 可调周期参数 ----
static const uint32_t kUpdateMs = 20; // 你的 vTaskDelayUntil 的节拍
static uint8_t phaseStep = 2;

void setBreathPeriodMs(uint16_t ms)
{
    if (ms < 300)
        ms = 300;
    if (ms > 20000)
        ms = 20000;
    uint32_t step = (256u * kUpdateMs + ms / 2) / ms; // 四舍五入
    if (step < 1)
        step = 1;
    if (step > 16)
        step = 16;
    phaseStep = (uint8_t)step;
}
void applyBreathColor()
{
    if (!currentState.isLightOn || !currentState.isBreathing)
        return;

    static uint8_t phase = 0; // 0..255 循环相位
    phase += phaseStep;

    // 三角波：0..255..0
    uint8_t tri = (phase < 128) ? (phase << 1) : ((uint8_t)((255 - phase) << 1));

    // 可选：感知均匀化（暗部不跑得快）
    // tri = pixels.gamma8(tri);

    // 按用户设置的最大亮度限幅
    uint16_t cap = currentState.brightness; // 0..255
    uint16_t level = (cap * tri) / 255;     // 0..cap

    pixels.setBrightness((uint8_t)level);

    uint32_t c = pixels.Color(currentState.r, currentState.g, currentState.b);
    for (int i = 0; i < LED_COUNT; ++i)
    {
        pixels.setPixelColor(i, c);
    }
    pixels.show();
}

void handleCommand(String command)
{
    if (command.isEmpty() || command.length() > 64) // 限制命令长度
        return;
    command.trim();
    String cmd = command;

    Serial.print("Received command: ");
    Serial.println(cmd);

    if (cmd == "ON")
    {
        currentState.isLightOn = true;
        currentState.isBreathing = false;
        applyColor();
    }
    else if (cmd == "OFF")
    {
        currentState.isBreathing = false;
        currentState.isLightOn = false;
        clearLight();
    }
    else if (cmd == "BREATHE ON")
    {
        currentState.isBreathing = true;
    }
    else if (cmd == "BREATHE OFF")
    {
        currentState.isBreathing = false;
        applyColor();
    }
    else if (cmd.startsWith("BREATHE PERIOD "))
    {
        // 提取毫秒数（命令后面的数字）
        int ms = cmd.substring(15).toInt(); // "BREATHE PERIOD " 长度是15
        currentState.periodMs = (uint16_t)ms;
        setBreathPeriodMs((uint16_t)ms);
        Serial.printf("[BREATH] Period set to %d ms\n", ms);
    }

    else if (cmd.startsWith("BRIGHT ")) // BRIGHT 后有空格
    {
        int val = 0;
        if (cmd.endsWith("%"))
        {                                                         // 百分比模式
            int pct = cmd.substring(7, cmd.length() - 1).toInt(); // 去掉 BRIGHT 和 %
            if (pct < 0)
                pct = 0;
            if (pct > 100)
                pct = 100;
            val = (pct * 255) / 100;
        }
        else
        { // 直接0~255
            val = cmd.substring(7).toInt();
            if (val < 0)
                val = 0;
            if (val > 255)
                val = 255;
        }

        currentState.brightness = (uint8_t)val;
        if (!currentState.isBreathing && currentState.isLightOn)
        {
            applyColor();
        }
    }
    else if (cmd == "GET STATE"){
        sendCurrentState();
    }
    else
    {
        // 解析 RGB 格式（如 "255,100,50"）
        int rr, gg, bb;
        if (sscanf(command.c_str(), "%d,%d,%d", &rr, &gg, &bb) == 3)
        {
            // 校验范围并更新颜色
            if (rr >= 0 && rr <= 255 && gg >= 0 && gg <= 255 && bb >= 0 && bb <= 255)
            {
                currentState.r = rr;
                currentState.g = gg;
                currentState.b = bb;
                if (!currentState.isBreathing && currentState.isLightOn)
                {
                    applyColor();
                }
            }
        }
    }
}

bool deviceConnected = false;
bool oldDeviceConnected = false;
class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        Serial.println("BLE Client connected");
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        Serial.println("BLE Client disconnected");
    }
};

void sendBLEMessage(String message) {
    if (deviceConnected && pCharacteristic) {
        pCharacteristic->setValue(message.c_str());
        pCharacteristic->notify();
    }
}

// 发送当前状态到指定输出(串口/BLE)
void sendCurrentState()
{
    // 构建状态字符串
    String stateMsg = String("STATE,") + 
                     String(currentState.isLightOn) + "," +
                     String(currentState.isBreathing) + "," +
                     String(currentState.r) + "," +
                     String(currentState.g) + "," + 
                     String(currentState.b) + "," +
                     String(currentState.brightness) + "," +
                     String(currentState.periodMs);
    
    // 根据参数选择输出方式
    if(deviceConnected) {
        sendBLEMessage(stateMsg);
    }

    Serial.println(stateMsg);
}
class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        String rxValue = String(pCharacteristic->getValue().c_str());
        if (rxValue.length() > 0)
        {
            Serial.println("BLE Command received: " + rxValue);
            handleCommand(rxValue);
            // 发送确认消息
            sendBLEMessage("OK: " + rxValue);
        }
    }
};
void initBLE()
{
    BLEDevice::init("Led-Box-BLE");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);
    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    // 开始广播
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0); // 不广播连接参数
    BLEDevice::startAdvertising();

    Serial.println("BLE service started, waiting for connections...");
}

static TimerHandle_t saveTimer;

static void saveTimerCb(TimerHandle_t)
{
    saveState(); // 注意：回调在 Timer 任务上下文里执行，需可重入/快速
}
static inline void scheduleSave()
{
    // 任意地方状态变化时调用它；会把 5s 计时重置
    xTimerReset(saveTimer, 0);
}

void initSaveDebounceTimer()
{
    saveTimer = xTimerCreate("saveDebounce",
                             pdMS_TO_TICKS(5000),
                             pdTRUE, nullptr, saveTimerCb);
    configASSERT(saveTimer);
    xTimerStart(saveTimer, 0);
}

void serialCommandTask(void *pvParameters)
{
    const TickType_t period = pdMS_TO_TICKS(20);

    TickType_t lastWake = xTaskGetTickCount();
    char line[128];
    while (true)
    {
        if (Serial.available())
        {
            String command = Serial.readStringUntil('\n');
            handleCommand(command);
        }
        applyBreathColor();
        vTaskDelayUntil(&lastWake, period);
    }
}

void bleCommandTask(void *pvParameters)
{
    while (true)
    {
    // Handle BLE disconnection/reconnection
    if (!deviceConnected && oldDeviceConnected)
    {
        vTaskDelay(pdMS_TO_TICKS(500)); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising();    // restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }

    // Handle new BLE connection
    if (deviceConnected && !oldDeviceConnected)
    {
        oldDeviceConnected = deviceConnected;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void setup()
{
    Serial.begin(115200);

    loadState();
    setBreathPeriodMs(currentState.periodMs);
    delay(200);
    pixels.begin();
    applyColor();
    // 初始化定时器
    initSaveDebounceTimer();
    initBLE();

    xTaskCreate(serialCommandTask, "SerialCommandTask", 4096, NULL, 1, NULL);
    xTaskCreate(bleCommandTask, "BleCommandTask", 4096, NULL, 1, NULL);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}