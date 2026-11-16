#include <Arduino.h>
#include <Wire.h>
#include <stdio.h>

// ESP32 reads both XGZP6857D sensors at address 0x6D on independent buses.
// Stream signed 24-bit raw counts; C# owns calibration and pickup decisions.
constexpr uint8_t SENSOR_ADDR = 0x6D;
constexpr uint32_t BAUD = 115200;
constexpr uint32_t SAMPLE_INTERVAL_MS = 50; // ~20 Hz
constexpr int UART2_RX = 16;
constexpr int UART2_TX = 17;

TwoWire prs1Bus(0); // GPIO21 SDA, GPIO22 SCL
TwoWire prs2Bus(1); // GPIO26 SDA, GPIO23 SCL

bool readByte(TwoWire &bus, const uint8_t reg, uint8_t &value) {
  bus.beginTransmission(SENSOR_ADDR);
  bus.write(reg);
  if (bus.endTransmission(true) != 0) return false;
  if (bus.requestFrom(SENSOR_ADDR, (uint8_t)1) != 1) return false;
  value = (uint8_t)bus.read();
  return true;
}

bool readPressureCounts(TwoWire &bus, int32_t &counts) {
  bus.beginTransmission(SENSOR_ADDR);
  bus.write((uint8_t)0x30);
  bus.write((uint8_t)0x0A);
  if (bus.endTransmission(true) != 0) return false;
  delay(20);

  uint8_t msb, csb, lsb;
  if (!readByte(bus, 0x06, msb)) return false;
  if (!readByte(bus, 0x07, csb)) return false;
  if (!readByte(bus, 0x08, lsb)) return false;

  uint32_t bits = (uint32_t(msb) << 16)
                | (uint32_t(csb) << 8)
                | uint32_t(lsb);
  if (bits & 0x00800000UL) bits |= 0xFF000000UL;
  counts = int32_t(bits);
  return true;
}

void setup() {
  Serial.begin(BAUD); // USB diagnostics
  Serial2.begin(BAUD, SERIAL_8N1, UART2_RX, UART2_TX);
  prs1Bus.begin(21, 22, 100000);
  prs2Bus.begin(26, 23, 100000);
  prs1Bus.setTimeOut(60);
  prs2Bus.setTimeOut(60);
  Serial.println("ESP32 PNP raw vacuum bridge to Marlin UART3 at 115200 baud");
}

void loop() {
  static uint32_t last_sample_ms = 0, last_error_ms = 0;
  const uint32_t now = millis();
  if (uint32_t(now - last_sample_ms) < SAMPLE_INTERVAL_MS) {
    delay(1);
    return;
  }
  last_sample_ms = now;

  int32_t p1 = 0, p2 = 0;
  const bool ok1 = readPressureCounts(prs1Bus, p1);
  const bool ok2 = readPressureCounts(prs2Bus, p2);
  if (!ok1 || !ok2) {
    // Withhold bad frames; Marlin reports stale data after 750 ms.
    if (uint32_t(now - last_error_ms) >= 1000UL) {
      Serial.printf("error:vacuum-read-failed bus1:%d bus2:%d\n", ok1, ok2);
      last_error_ms = now;
    }
    return;
  }

  char frame[48];
  const int n = snprintf(frame, sizeof(frame), "prs1:%ld prs2:%ld\n",
                         (long)p1, (long)p2);
  if (n > 0 && n < (int)sizeof(frame)) {
    Serial2.write((const uint8_t *)frame, (size_t)n);
    Serial.write((const uint8_t *)frame, (size_t)n);
  }
}
