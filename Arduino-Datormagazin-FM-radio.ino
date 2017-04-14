#include <Wire.h>
#include <TEA5767N.h> // https://github.com/mroger/TEA5767
#include "LedControl.h" // https://github.com/wayoda/LedControl.git
#include <Encoder.h> // From Arduino Library Manager

#define ROTARY_PIN1 2
#define ROTARY_PIN2 3
#define ROTARY_SWITCH_PIN 4
#define DISPLAY_DATA 5
#define DISPLAY_CLK 7
#define DISPLAY_LOAD 6
#define ROTARY_FACTOR 10.0
#define MIN_FREQ (88.0 * ROTARY_FACTOR) // Min 88.0 MHz
#define MAX_FREQ (108.0 * ROTARY_FACTOR) // Max 108.0 MHz
#define DEBOUNCE_TIME 50
Encoder enc(ROTARY_PIN1, ROTARY_PIN2);
int32_t currFreq = 99.4 * ROTARY_FACTOR; // SR P3 Göteborg 99.40 MHz
LedControl disp = LedControl(DISPLAY_DATA, DISPLAY_CLK, DISPLAY_LOAD);
TEA5767N radio = TEA5767N();

void setup() {
  Serial.begin(115200);
  pinMode(ROTARY_SWITCH_PIN, INPUT);
  digitalWrite(ROTARY_SWITCH_PIN, HIGH); // Enable pullup
  enc.write(currFreq); // Initialize to default frequency

  disp.shutdown(0, false);
  disp.setIntensity(0, 8); // Set brightness
  disp.clearDisplay(0);

  radio.setSearchMidStopLevel();
  radio.selectFrequency(currFreq / ROTARY_FACTOR);
  updateDisplay();
  Serial.println("Setup done");
}

void loop() {
  if (!digitalRead(ROTARY_SWITCH_PIN)) {
    Serial.println("Click");
    unsigned long startTime = millis();
    Serial.print("Radio Freq before search: ");
    Serial.println(radio.readFrequencyInMHz());
    if (currFreq == MAX_FREQ) {
      radio.startsSearchMutingFromBeginning();
    } else {
      radio.searchNextMuting();
    }
    delay(50);
    Serial.print("Radio Freq after search: ");
    Serial.println(radio.readFrequencyInMHz());
    currFreq = constrain((radio.readFrequencyInMHz() + 0.5 / ROTARY_FACTOR) * ROTARY_FACTOR, MIN_FREQ, MAX_FREQ); // 0.5 is used to get correct rounding
    enc.write(currFreq); // Re-set if value was out of range
    updateDisplay();
    unsigned long seekTime = startTime - millis();
    if (seekTime < DEBOUNCE_TIME) {
      delay(DEBOUNCE_TIME - seekTime);
    }
  }
  int32_t newFreq = enc.read();
  if (newFreq != currFreq) {
    Serial.print("Turn: ");
    Serial.println(newFreq);
    currFreq = constrain(newFreq, MIN_FREQ, MAX_FREQ);
    enc.write(currFreq); // Re-set if value was out of scope
    radio.mute();
    radio.selectFrequency(newFreq / ROTARY_FACTOR);
    radio.turnTheSoundBackOn();
    updateDisplay();
  }
}

void updateDisplay() {
  Serial.print("Update display with: ");
  Serial.println(currFreq);
  Serial.println(radio.isStereo() ? "Stereo" : "Mono");
  disp.clearDisplay(0);

  if (currFreq >= 1000) {
    disp.setDigit(0, 3, (currFreq / 1000) % 10, false);
  }
  disp.setDigit(0, 2, (currFreq / 100) % 10, false);
  disp.setDigit(0, 1, (currFreq / 10) % 10, true);
  disp.setDigit(0, 0, currFreq % 10, false);
}

