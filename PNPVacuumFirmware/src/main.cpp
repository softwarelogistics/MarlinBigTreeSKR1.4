#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// XGZP6857D V2.5, factory I2C address 0x6D.
// Bench firmware: both sensors on independent ESP32 I2C controllers.
// IMPORTANT: The K values below are EXAMPLES for a 100 kPa-range sensor.
// Replace them with the K for EACH installed sensor's actual rated range.
constexpr uint8_t SENSOR_ADDR = 0x6D;
constexpr float PRS1_K = 64.0f; // provisional: 100 kPa range
constexpr float PRS2_K = 64.0f; // provisional: 100 kPa range
constexpr int ZERO_SAMPLES = 16;

TwoWire prs1Bus(0);
TwoWire prs2Bus(1);

int32_t prs1Zero = 0;
int32_t prs2Zero = 0;
bool hasZero = false;
bool diagnosticOnce = false;

bool readByte(TwoWire& bus, uint8_t reg, uint8_t& byteValue) {
    bus.beginTransmission(SENSOR_ADDR);
    bus.write(reg);
    if (bus.endTransmission(true) != 0) return false;
    if (bus.requestFrom(SENSOR_ADDR, (uint8_t)1) != 1) return false;
    byteValue = bus.read();
    return true;
}

bool readPressureCounts(TwoWire& bus, int32_t& counts) {
    // Trigger a calibrated temperature + pressure conversion.
    bus.beginTransmission(SENSOR_ADDR);
    bus.write((uint8_t)0x30);
    bus.write((uint8_t)0x0A);
    if (bus.endTransmission(true) != 0) return false;
    delay(20); // V2.5 datasheet's alternative to polling SCO.

    // Keep the individually addressed reads already proven on this board.
    uint8_t msb, csb, lsb;
    if (!readByte(bus, 0x06, msb)) return false;
    if (!readByte(bus, 0x07, csb)) return false;
    if (!readByte(bus, 0x08, lsb)) return false;

    uint32_t bits = ((uint32_t)msb << 16)
                  | ((uint32_t)csb << 8)
                  | (uint32_t)lsb;

    // Correctly sign-extend 24-bit two's-complement to signed 32-bit.
    if (bits & 0x00800000UL) bits |= 0xFF000000UL;
    counts = (int32_t)bits;
    return true;
}

bool readBoth(int32_t& p1, int32_t& p2) {
    bool ok1 = readPressureCounts(prs1Bus, p1);
    bool ok2 = readPressureCounts(prs2Bus, p2);
    return ok1 && ok2;
}

void zeroAtAtmosphere() {
    Serial.println("Zeroing: both pumps must be OFF, sensor ports vented...");
    int64_t sum1 = 0, sum2 = 0;
    for (int i = 0; i < ZERO_SAMPLES; ++i) {
        int32_t r1, r2;
        if (!readBoth(r1, r2)) {
            Serial.println("ERROR: zero calibration failed (I2C read)");
            return;
        }
        sum1 += r1;
        sum2 += r2;
    }
    prs1Zero = (int32_t)(sum1 / ZERO_SAMPLES);
    prs2Zero = (int32_t)(sum2 / ZERO_SAMPLES);
    hasZero = true;
    Serial.printf("Zero set: prs1=%ld prs2=%ld\n",
                  (long)prs1Zero, (long)prs2Zero);
}

// Positive pressure drop (vacuum magnitude), in pascals.
// Assumes raw counts decrease when vacuum increases, as measured on this board.
long vacuumPa(int32_t raw, int32_t zero, float k) {
    float pa = ((float)zero - (float)raw) / k;
    return lroundf(pa > 0.0f ? pa : 0.0f);
}

void setup() {
    Serial.begin(115200);
    delay(900);
    prs1Bus.begin(21, 22, 100000);
    prs2Bus.begin(26, 23, 100000);
    prs1Bus.setTimeOut(60);
    prs2Bus.setTimeOut(60);

    Serial.println("ESP32 dual XGZP6857D normalized vacuum bench");
    Serial.println("PRS1 SDA21/SCL22; PRS2 SDA26/SCL23; both 0x6D");
    Serial.println("K values are PROVISIONAL: verify sensor pressure ranges!");
    Serial.println("With pumps OFF and ports vented, send 'z' to zero.");
    Serial.println("Send 'd' to display diagnostic raw readings.");
}

void loop() {
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == 'z' || c == 'Z') zeroAtAtmosphere();
        if (c == 'd' || c == 'D') diagnosticOnce = true;
    }

    int32_t p1, p2;
    if (!readBoth(p1, p2)) {
        Serial.println("error:vacuum-read-failed");
    } else if (!hasZero) {
        Serial.printf("raw1:%ld raw2:%ld (send z to zero)\n", (long)p1, (long)p2);
    } else {
        // Legacy-compatible PRS labels. Physical left/right assignment can be
        // confirmed separately; downstream thresholds must be recalibrated.
        Serial.printf("prs1:%ld prs2:%ld\n",
                      vacuumPa(p1, prs1Zero, PRS1_K),
                      vacuumPa(p2, prs2Zero, PRS2_K));
        if (diagnosticOnce) {
            Serial.printf("debug raw1:%ld raw2:%ld zero1:%ld zero2:%ld K1:%.0f K2:%.0f\n",
                          (long)p1, (long)p2,
                          (long)prs1Zero, (long)prs2Zero,
                          PRS1_K, PRS2_K);
            diagnosticOnce = false;
        }
    }
    delay(500);
}
