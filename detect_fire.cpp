
#define BLYNK_TEMPLATE_ID   "TMPL6Zy-m-r3w"
#define BLYNK_TEMPLATE_NAME "IOT Basefire Detect"
#define BLYNK_AUTH_TOKEN    "O8AkhICIJHcNYhK1Ov3QwCtEk_f8ad3Y"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ================== WiFi ==================
char ssid[] = "TLU_ThuVien";
char pass[] = "123456789";

// ================== VÙNG 1 ==================
const int flamePin1 = 27;   // LOW = FIRE
const int gasPin1   = 34;
const int ledZone1  = 17;

// ================== VÙNG 2 ==================
const int flamePin2 = 21;   // LOW = FIRE
const int gasPin2   = 35;
const int ledZone2  = 16;

// ================== CẢM BIẾN KHÁC ==================
const int dhtPin   = 25;
const int pirPin   = 33;
const int waterPin = 32;
const int soilPin  = 22;

#define DHTTYPE DHT22
DHT dht(dhtPin, DHTTYPE);

// ================== RELAY ==================
const int valveRelay1Pin   = 23;
const int valveRelay2Pin   = 18;
const int mainPumpRelayPin = 19;

// ================== BLYNK ==================
#define VP_FLAME1  V0
#define VP_GAS1    V1
#define VP_TEMP    V2
#define VP_HUM     V3
#define VP_PIR     V4
#define VP_WATER   V5
#define VP_SOIL    V6
#define VP_FLAME2  V7
#define VP_GAS2    V100
#define VP_LED_ZONE1 V9
#define VP_LED_ZONE2 V10

BlynkTimer timer;

// ================== TRẠNG THÁI LỬA ==================
bool fireZone1 = false;
bool fireZone2 = false;

// ================== BLINK 50ms ==================
unsigned long lastBlink1 = 0;
unsigned long lastBlink2 = 0;
bool ledState1 = false;
bool ledState2 = false;

// ================== SENSOR ==================
void sendSensor() {
  // --- Đọc cảm biến ---
  int flame1 = digitalRead(flamePin1);
  int flame2 = digitalRead(flamePin2);

  fireZone1 = (flame1 == LOW);
  fireZone2 = (flame2 == LOW);

  int gas1 = analogRead(gasPin1);
  int gas2 = analogRead(gasPin2);

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  int pirVal   = digitalRead(pirPin);
  int waterVal = digitalRead(waterPin);
  int soilVal  = analogRead(soilPin);

  // --- Gửi dữ liệu lên Blynk ---
  Blynk.virtualWrite(VP_FLAME1, flame1);
  Blynk.virtualWrite(VP_GAS1, gas1);
  Blynk.virtualWrite(VP_FLAME2, flame2);
  Blynk.virtualWrite(VP_GAS2, gas2);
  if (!isnan(temp)) Blynk.virtualWrite(VP_TEMP, temp);
  if (!isnan(hum))  Blynk.virtualWrite(VP_HUM, hum);
  Blynk.virtualWrite(VP_PIR, pirVal);
  Blynk.virtualWrite(VP_WATER, waterVal);
  Blynk.virtualWrite(VP_SOIL, soilVal);

  // --- Điều khiển relay ---
  digitalWrite(valveRelay1Pin, fireZone1 ? LOW : HIGH);
  digitalWrite(valveRelay2Pin, fireZone2 ? LOW : HIGH);
  bool pumpOn = fireZone1 || fireZone2;
  digitalWrite(mainPumpRelayPin, pumpOn ? LOW : HIGH);

  // --- In ra Serial tất cả dữ liệu ---
  Serial.println("===== SENSOR DATA =====");
  Serial.print("Flame1: "); Serial.print(flame1); Serial.print("  FireZone1: "); Serial.println(fireZone1);
  Serial.print("Gas1: "); Serial.println(gas1);
  Serial.print("Flame2: "); Serial.print(flame2); Serial.print("  FireZone2: "); Serial.println(fireZone2);
  Serial.print("Gas2: "); Serial.println(gas2);
  Serial.print("Temp: "); Serial.print(temp); Serial.print(" C  Hum: "); Serial.println(hum);
  Serial.print("PIR: "); Serial.println(pirVal);
  Serial.print("Water: "); Serial.println(waterVal);
  Serial.print("Soil: "); Serial.println(soilVal);
  Serial.print("ValveRelay1: "); Serial.println(fireZone1 ? "ON" : "OFF");
  Serial.print("ValveRelay2: "); Serial.println(fireZone2 ? "ON" : "OFF");
  Serial.print("MainPump: "); Serial.println(pumpOn ? "ON" : "OFF");
  Serial.println("======================\n");
}

// ================== BLINK LOGIC ==================
void blinkZoneLEDs() {
  unsigned long now = millis();

  // -------- ZONE 1 --------
  if (fireZone1) {
    if (now - lastBlink1 >= 50) {
      lastBlink1 = now;
      ledState1 = !ledState1;
      digitalWrite(ledZone1, ledState1);
      Serial.print("LED Zone1: "); Serial.println(ledState1 ? "ON" : "OFF");

      // GỬI LED ZONE 1 LÊN BLYNK
      Blynk.virtualWrite(VP_LED_ZONE1, ledState1 ? 255 : 0);
    }
  } else {
    digitalWrite(ledZone1, LOW);
    ledState1 = false;
  }

  // -------- ZONE 2 --------
  if (fireZone2) {
    if (now - lastBlink2 >= 50) {
      lastBlink2 = now;
      ledState2 = !ledState2;
      digitalWrite(ledZone2, ledState2);
      Serial.print("LED Zone2: "); Serial.println(ledState2 ? "ON" : "OFF");
      // GỬI LED ZONE 2 LÊN BLYNK
      Blynk.virtualWrite(VP_LED_ZONE2, ledState2 ? 255 : 0);
    }
  } else {
    digitalWrite(ledZone2, LOW);
    ledState2 = false;
  }
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);

  pinMode(flamePin1, INPUT);
  pinMode(flamePin2, INPUT);
  pinMode(gasPin1, INPUT);
  pinMode(gasPin2, INPUT);

  pinMode(ledZone1, OUTPUT);
  pinMode(ledZone2, OUTPUT);

  pinMode(valveRelay1Pin, OUTPUT);
  pinMode(valveRelay2Pin, OUTPUT);
  pinMode(mainPumpRelayPin, OUTPUT);

  pinMode(pirPin, INPUT);
  pinMode(waterPin, INPUT);
  pinMode(soilPin, INPUT);

  digitalWrite(valveRelay1Pin, HIGH);
  digitalWrite(valveRelay2Pin, HIGH);
  digitalWrite(mainPumpRelayPin, HIGH);

  dht.begin();

  // --- Kết nối WiFi ---
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // --- Kết nối Blynk ---
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  Serial.println("Blynk connected!");

  // --- Timer gửi cảm biến mỗi 1s ---
  timer.setInterval(1000L, sendSensor);
}

// ================== LOOP ==================
void loop() {
  Blynk.run();
  timer.run();
  blinkZoneLEDs();   // LED nháy 50ms
}



