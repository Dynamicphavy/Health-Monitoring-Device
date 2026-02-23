#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
MAX30105 mex;

void irCheck(uint32_t irValue) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("IR Checker");

    display.setCursor(0, 20);
    display.print("IR Value:");
    display.print(irValue);

    display.display();
}

void setup() {
    Serial.begin(115200);
    Wire.begin(5, 4);
    Wire.setClock(400000);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        while (1);
    }

    if (!mex.begin()) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("MAX30102 Error");
        display.display();
        while (1);
    }

    byte ledBrightness = 50;   // Increase for testing
    byte sampleAverage = 4;
    byte ledMode = 2;
    byte sampleRate = 100;
    int pulseWidth = 411;
    int adcRange = 4096;

    mex.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Place skin...");
    display.display();
}

void loop() {
    mex.check();

    if (mex.available()) {
        uint32_t irValue = mex.getIR();
        irCheck(irValue);
        mex.nextSample();
    }

    delay(100);
}