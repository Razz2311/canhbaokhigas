#include <DHT.h>

#define DHTPIN D4         // DHT11 data pin
#define DHTTYPE DHT11     // Sensor type
#define BUZZER_PIN D3     // Buzzer pin
#define GAS_SENSOR_PIN A0 // MQ2/MQ5 analog pin
#define BUTTON_PIN D2     // Nút tắt còi

DHT dht(DHTPIN, DHTTYPE);

// Threshold values
const float TEMP_THRESHOLD = 40.0;  // Temperature threshold (°C)
const float HUM_THRESHOLD = 70.0;   // Humidity threshold (%)
const int GAS_THRESHOLD = 200;      // Gas concentration threshold (0-1023)

bool buzzerDisabled = false;        // Trạng thái tắt còi tạm thời
unsigned long lastPressTime = 0;    // Thời gian nhấn nút cuối
const unsigned long debounceTime = 200; // Thời gian chống nhiễu

void activateBuzzer() {
  if (!buzzerDisabled) {            // Chỉ kích hoạt nếu chưa bị tắt
    tone(BUZZER_PIN, 2000);         // Continuous buzzer at 2000Hz
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Kích hoạt điện trở kéo lên
  dht.begin();
}

void loop() {
  delay(5000); // Giữ nguyên thời gian delay
  
  // Xử lý nút nhấn D2
  if (digitalRead(BUTTON_PIN) == LOW && 
      millis() - lastPressTime > debounceTime) {
    buzzerDisabled = !buzzerDisabled; // Đảo trạng thái
    lastPressTime = millis();
    Serial.println(buzzerDisabled ? "🔕 Còi đã TẮT" : "🔔 Còi đã BẬT");
    noTone(BUZZER_PIN);              // Dừng còi ngay lập tức
  }

  // Phần đọc cảm biến giữ nguyên
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int gasValue = analogRead(GAS_SENSOR_PIN);

  // Sensor error handling (giữ nguyên)
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("⚠️ DHT11 Error! Check:");
    Serial.println("1. Wiring (VCC, GND, DATA)");
    Serial.println("2. Power supply (3.3V-5V)");
    Serial.println("3. Cable length < 20m");
    Serial.println("4. Try ESP8266 reset");
    return;
  }

  // Phần hiển thị giá trị (đã dịch sang tiếng Việt)
  Serial.print("✅ nhiệt: ");
  Serial.print(temperature);
  Serial.print("°C\tđộ ẩm: ");
  Serial.print(humidity);
  Serial.print("%\tGas: ");
  Serial.println(gasValue);

  // Kiểm tra ngưỡng
  bool alert = false;
  if (temperature >= TEMP_THRESHOLD) {
    Serial.println("🔥 Cảnh báo nhiệt độ cao!");
    alert = true;
  }
  if (humidity >= HUM_THRESHOLD) {
    Serial.println("💧 Cảnh báo độ ẩm cao!");
    alert = true;
  }
  if (gasValue >= GAS_THRESHOLD) {
    Serial.println("⚠️ PHÁT HIỆN RÒ RỈ GAS!");
    alert = true;
  }

  // Điều khiển còi với trạng thái nút nhấn
  if (alert && !buzzerDisabled) {
    activateBuzzer();
  } else {
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
  }
}
