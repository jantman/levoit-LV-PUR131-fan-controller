\
/*
  24 V BLDC Fan Controller — Rotary 4-Pos (OFF = static LOW) + Telemetry
  Board: Arduino Uno R3 (ATmega328P)

  Hardware summary:
    - D9 (OC1A) → 1 kΩ → 2N2222 base; emitter → GND; collector → fan PWM
    - 10 kΩ pull-up: fan PWM → +5 V
    - Rotary 1P4T ladder to A0 with 4×10k (OFF=0V, LOW≈1.25V, MED≈2.5V, HIGH≈3.75V)
      and 0.1 µF from A0→GND near the MCU
    - LEDs: D4(LOW), D5(MED), D6(HIGH)

  Behavior:
    - OFF: PWM disabled; D9 driven HIGH → transistor ON → line LOW (~0V on DMM)
    - LOW/MED/HIGH: 4.000 kHz PWM; line-HIGH duties 36% / 56% / 93%

  Telemetry (115200 baud):
    - Periodic line with Mode, duty %, OCR1A, A0 ADC & volts
    - Mode change messages
*/

#define SOFTSTART_MS           400   // 0 to disable kick when leaving OFF
#define TELEMETRY_PERIOD_MS    500   // periodic print interval

// -------- Pins --------
const uint8_t PIN_PWM   = 9;   // OC1A
const uint8_t PIN_LED_L = 4;
const uint8_t PIN_LED_M = 5;
const uint8_t PIN_LED_H = 6;
const uint8_t PIN_AROT  = A0;

// -------- PWM config --------
const uint16_t TOP  = 499;             // 4.000 kHz @ prescale=8
const uint16_t FULL = TOP + 1;

// Desired line-HIGH duties
const uint8_t DUTY_LOW  = 36;
const uint8_t DUTY_MED  = 56;
const uint8_t DUTY_HIGH = 93;

enum Mode : uint8_t { MODE_OFF=0, MODE_LOW, MODE_MED, MODE_HIGH };
Mode currentMode = MODE_OFF, lastMode = MODE_OFF;

// Rotary thresholds (10-bit ADC, Vref=5 V)
const int ADC_LOW_C  = 256;  // ~1.25 V
const int ADC_MED_C  = 512;  // ~2.50 V
const int ADC_HIGH_C = 768;  // ~3.75 V
const int MARGIN     = 70;   // ≈0.34 V

// -------- Timer1 setup: Fast PWM, non-inverting on OC1A (D9) --------
void timerSetup() {
  pinMode(PIN_PWM, OUTPUT);
  TCCR1A = 0; TCCR1B = 0;
  ICR1   = TOP;
  OCR1A  = 0;
  TCCR1A = (1 << WGM11) | (1 << COM1A1);               // Fast PWM, non-inverting on OC1A
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);  // prescale=8
}

// Convert desired line-HIGH duty (%) to OCR1A counts
static inline uint16_t countsForLineHighPct(uint8_t pct) {
  if (pct > 100) pct = 100;
  uint16_t counts = (uint32_t)(100 - pct) * FULL / 100; // pin HIGH time = line-LOW %
  if (counts > TOP) counts = TOP;
  return counts;
}

// Convenience to set PWM duty (line-HIGH %) ensuring PWM is enabled
void setRunDuty(uint8_t pct) {
  if ((TCCR1A & (1<<COM1A1)) == 0) {
    // re-enable OC1A PWM output
    TCCR1A = (TCCR1A & ~((1<<COM1A1)|(1<<COM1A0))) | (1<<COM1A1);
  }
  OCR1A = countsForLineHighPct(pct);
}

// LEDs
void setLEDs(Mode m) {
  digitalWrite(PIN_LED_L, m == MODE_LOW);
  digitalWrite(PIN_LED_M, m == MODE_MED);
  digitalWrite(PIN_LED_H, m == MODE_HIGH);
}

// Rotary classification with simple hysteresis
Mode classifyAnalog(int adc, Mode prev) {
  if (adc < (ADC_LOW_C - MARGIN)) return MODE_OFF;
  if (adc > (ADC_HIGH_C + MARGIN)) return MODE_HIGH;
  int dL = abs(adc - ADC_LOW_C), dM = abs(adc - ADC_MED_C), dH = abs(adc - ADC_HIGH_C);
  Mode m = MODE_LOW; int d = dL;
  if (dM < d) { d = dM; m = MODE_MED; }
  if (dH < d) { m = MODE_HIGH; }
  return m;
}

// Apply selected mode
void applyMode(Mode m) {
  if (m == MODE_OFF) {
    // Robust OFF = static LOW on PWM line
    TCCR1A &= ~((1<<COM1A1)|(1<<COM1A0));   // disconnect OC1A PWM
    digitalWrite(PIN_PWM, HIGH);            // base ON → line LOW
  } else {
    // Ensure PWM enabled
    if ((TCCR1A & (1<<COM1A1)) == 0) {
      TCCR1A = (TCCR1A & ~((1<<COM1A1)|(1<<COM1A0))) | (1<<COM1A1);
      if (SOFTSTART_MS > 0 && lastMode == MODE_OFF) {
        setRunDuty(100);                    // 100% line-HIGH kick
        setLEDs(m);
        delay(SOFTSTART_MS);
      }
    }
    switch (m) {
      case MODE_LOW:  setRunDuty(DUTY_LOW);  break;
      case MODE_MED:  setRunDuty(DUTY_MED);  break;
      case MODE_HIGH: setRunDuty(DUTY_HIGH); break;
      default: break;
    }
  }
  setLEDs(m);
  currentMode = m;
}

// -------- Telemetry helpers --------
const char* modeName(Mode m) {
  switch (m) {
    case MODE_OFF:  return "OFF";
    case MODE_LOW:  return "LOW";
    case MODE_MED:  return "MED";
    case MODE_HIGH: return "HIGH";
  }
  return "?";
}

float a0VoltsFromADC(int adc) {
  return (adc * (5.0f / 1023.0f));
}

uint8_t currentLineHighPct() {
  if ((TCCR1A & (1<<COM1A1)) == 0) return 0;
  uint16_t counts = OCR1A;
  uint8_t pct = (uint8_t)(100 - ((uint32_t)counts * 100 / FULL));
  return pct;
}

// -------- Arduino lifecycle --------
void setup() {
  Serial.begin(115200);
  Serial.println(F("\\n=== 24V BLDC Fan Controller — Telemetry Build ==="));
  Serial.println(F("OFF = static LOW; PWM = 4.000 kHz; D9=OC1A; ladder on A0"));
  Serial.println(F("Duties: LOW=36%  MED=56%  HIGH=93% (line-HIGH %)"));

  pinMode(PIN_LED_L, OUTPUT);
  pinMode(PIN_LED_M, OUTPUT);
  pinMode(PIN_LED_H, OUTPUT);

  timerSetup();
  TCCR1A &= ~((1<<COM1A1)|(1<<COM1A0));
  digitalWrite(PIN_PWM, HIGH);  // hold line LOW

  applyMode(MODE_OFF);
}

void loop() {
  int acc = 0;
  for (int i = 0; i < 4; i++) { acc += analogRead(PIN_AROT); delay(2); }
  int adc = acc / 4;
  Mode m = classifyAnalog(adc, currentMode);

  if (m != currentMode) {
    lastMode = currentMode;
    applyMode(m);
    Serial.print(F("Mode change: "));
    Serial.print(modeName(lastMode));
    Serial.print(F(" -> "));
    Serial.print(modeName(currentMode));
    if (currentMode == MODE_OFF) {
      Serial.println(F("  (OFF static LOW)"));
    } else {
      Serial.print(F("  (line-HIGH="));
      Serial.print(currentLineHighPct());
      Serial.println(F("%)"));
    }
  }

  static uint32_t lastPrint = 0;
  uint32_t now = millis();
  if (now - lastPrint >= TELEMETRY_PERIOD_MS) {
    lastPrint = now;
    Serial.print(F("Mode="));        Serial.print(modeName(currentMode));
    Serial.print(F("  duty="));      Serial.print(currentLineHighPct()); Serial.print(F("%"));
    Serial.print(F("  OCR1A="));     Serial.print((uint16_t)OCR1A);
    Serial.print(F("  A0="));        Serial.print(adc);
    Serial.print(F(" ("));           Serial.print(a0VoltsFromADC(adc), 2); Serial.print(F(" V)"));
    Serial.println();
  }

  delay(5);
}
