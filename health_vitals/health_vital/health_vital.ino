#include <Wire.h>

// Libraries for IoT

// Libraries for OLED Display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Health Vitals Library
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

#include <Adafruit_MLX90614.h>
#include "SparkFunLSM6DS3.h"

// Display size in pixels
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// User function

// Authentication

// Firebase components

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Display what's on the screen
MAX30105 mex; // Heart Rate & SPO2
Adafruit_MLX90614 mlx = Adafruit_MLX90614(); // Temperature
LSM6DS3 myIMU(I2C_MODE, 0x6A); // Steps

// Pulse Rate and SPO2
uint32_t irBuffer[100];
uint32_t redBuffer[100];

int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

// Steps
int steps = 0;
unsigned long lastStepTime = 0;
unsigned long stopDelay = 4000; // 4 seconds without steps
unsigned long stepInterval = 300;

// Timing
unsigned long lastOLEDUpdate = 0;

// Front-End Interface
static void healthVitals(int bpm, int spo2, float temp, int steps) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Title
    display.setCursor(0, 0);
    display.println("Health Monitoring");

    // Display Pulse Rate and SPO2
    display.setCursor(0, 12);
    display.print("Pulse Rate: ");
    display.print(bpm);
    display.println(" BPM");

    display.setCursor(0, 24);
    display.print("SPO2: ");
    display.print(spo2);
    display.println(" %");

    // Display Temperature
    display.setCursor(0, 36);
    display.print("Temp(C): ");
    display.print(temp, 1);
    display.println(" C");

    // Display Steps
    display.setCursor(0, 48);
    display.print("Steps: ");
    display.print(steps);

    display.display();
}

void detectSteps() {}

void alert() {}

void setup() {
    Serial.begin(115200);
    Wire.begin(5, 4);
    Wire.setClock(400000);

    // ------------- IOT SETUP ------------

    // Check if the OLED Display is working
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
        for(;;) {
            delay(10);
        }
    }

    // Display setup
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Initializing...");
    display.display();
    delay(1000);

    if(!mex.begin()) { // Heart rate & SPO2 sensor
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Error, MAX30102 not connected");
        for (;;) {
            delay(10);
        }
    }

    mex.setup();
    mex.setPulseAmplitudeRed(0x3F);
    mex.setPulseAmplitudeIR(0x3F);

    if(!mlx.begin()) { // Temperature sensor
        display.clearDisplay();
        display.setCursor(0, 12);
        display.println("Error, MLX90614 not connected");
        display.display();
        for (;;) {
            delay(10);
        }
    }

    if(!myIMU.begin()) { // Accelerometer sensor
        display.clearDisplay();
        display.setCursor(0, 24);
        display.println("Error, LSM6DS3 not connected");
        for (;;) {
            delay(10);
        }
    }

    // When the sensors are working
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("All sensors are OK!!!");
    display.display();
    delay(500);
}

void loop() {
    // ------------- MAX30102 READINGS ------------
    for (byte i = 0; i < 100; i++) {
        while (!mex.available())
            mex.check();

        redBuffer[i] = mex.getRed();
        irBuffer[i] = mex.getIR();
        mex.nextSample();
    }

    uint32_t irValue = irBuffer[99];
    bool skinDetected = irValue > 50000;

    maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    int bpm = (skinDetected && validHeartRate) ? heartRate : 0;
    int ESPO2 = (skinDetected && validSPO2 && spo2 > 0) ? spo2 : 0;
    spo2 = ESPO2;

    // ------------- TEMPERATURE READINGS -------------
    float temp = mlx.readObjectTempC();
    temp = temp + 0.5;

    // ------------- STEP COUNTING -------------
    detectSteps();

    if (millis() - lastOLEDUpdate > 1000) {
        lastOLEDUpdate = millis();
        healthVitals(bpm, spo2, temp, steps); // Displayed on OLED
    }
}
