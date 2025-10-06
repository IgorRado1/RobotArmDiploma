// all3sensorsV5
// Vključuje: barvni senzor, tehtnico, 2 ultrazvočna senzorja + LED prikaz polnosti binov

#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <HX711_ADC.h>
#include <EEPROM.h>

// BARVNI SENZOR
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);

// ULTRAZVOČNI SENZORJI
const int trigPin1 = 9;
const int echoPin1 = 10;
const int trigPin2 = 11;
const int echoPin2 = 12;

// LED za bin full indikatorje
const int bin1LED = 2;
const int bin2LED = 3;
const int bin3LED = 6;
const int bin4LED = 7;

// TEHTNICA
const int HX711_dout = 4;
const int HX711_sck = 5;
HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_eepromAdress = 0;

String currentColor = "UNKNOWN";
int colorID = -1;

void setup() {
  Serial.begin(9600);
  delay(500);

  if (!tcs.begin()) {
    Serial.println("Napaka: TCS3472 senzor NI bil zaznan!");
    while (1);
  }

  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  // Nastavi LED pine
  pinMode(bin1LED, OUTPUT);
  pinMode(bin2LED, OUTPUT);
  pinMode(bin3LED, OUTPUT);
  pinMode(bin4LED, OUTPUT);

  // Izklopi vse LED na zacetku
  digitalWrite(bin1LED, LOW);
  digitalWrite(bin2LED, LOW);
  digitalWrite(bin3LED, LOW);
  digitalWrite(bin4LED, LOW);

  LoadCell.begin();
  float calibrationValue = 696.0;
  EEPROM.get(calVal_eepromAdress, calibrationValue);
  LoadCell.setCalFactor(calibrationValue);
  LoadCell.start(2000, true);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Napaka: HX711 ni odgovoril.");
    while (1);
  }
  while (!LoadCell.update());
  Serial.println("Sistem pripravljen.");
}

long readUltrasonicCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return (duration > 0) ? duration / 29 / 2 : -1;
}

void loop() {
  // 1. BARVNI SENZOR
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  if (c > 50) {
    float red = (float)r / c;
    float green = (float)g / c;
    float blue = (float)b / c;

    //če zazna rdeč valjček, vrne "Beli kruh"
    if (red > green && red > blue) currentColor = "WHITE", colorID = 0;
    //če zazna modri valjček, vrne "črni kruh"
    else if (red < blue && green < blue) currentColor = "BLACK", colorID = 1;
    else currentColor = "UNKNOWN", colorID = -1;
  } else {
    currentColor = "UNKNOWN"; colorID = -1;
  }

  // 2. ULTRAZVOČNI SENZORJI
  long cm1 = readUltrasonicCM(trigPin1, echoPin1);
  long cm2 = readUltrasonicCM(trigPin2, echoPin2);

  int detectedBin = 0;
  //2 škatli za prvi senzor
  if (cm1 >= 5 && cm1 < 13) detectedBin = 1;
  else if (cm1 >= 13 && cm1 < 19) detectedBin = 2;
  //2 škatli za drugi senzor
  else if (cm2 >= 5 && cm2 < 13) detectedBin = 4;
  else if (cm2 >= 13 && cm2 < 19) detectedBin = 3;

  static int lastDetectedBin = 0;
  if (detectedBin > 0) lastDetectedBin = detectedBin;
  if (cm1 > 25 && cm2 > 25) lastDetectedBin = 0;

  // LED kontrola
  digitalWrite(bin1LED, lastDetectedBin == 1 ? HIGH : LOW);
  digitalWrite(bin2LED, lastDetectedBin == 2 ? HIGH : LOW);
  digitalWrite(bin3LED, lastDetectedBin == 3 ? HIGH : LOW);
  digitalWrite(bin4LED, lastDetectedBin == 4 ? HIGH : LOW);

  // 3. TEŽA
  const byte STABLE_READS = 5;
  const float TOLERANCE = 0.5;
  static float weightBuffer[STABLE_READS];
  static byte bufferIndex = 0;
  static bool bufferFull = false;
  static bool alreadyReported = false;
  static String lastOutputColor = "NONE";
  static float lastOutputWeight = 0.0;

  if (LoadCell.update()) {
    float weight = LoadCell.getData();
    weightBuffer[bufferIndex++] = weight;
    if (bufferIndex >= STABLE_READS) {
      bufferIndex = 0;
      bufferFull = true;
    }

    if (bufferFull) {
      float avg = 0;
      for (byte i = 0; i < STABLE_READS; i++) avg += weightBuffer[i];
      avg /= STABLE_READS;

      bool stable = true;
      for (byte i = 0; i < STABLE_READS; i++) {
        if (abs(weightBuffer[i] - avg) > TOLERANCE) {
          stable = false;
          break;
        }
      }

      //pridobi weight class valjčka
      if (stable && avg >= 5.0 && !alreadyReported && currentColor != "UNKNOWN") {
        String currentClass = "UNKNOWN";
        if (avg >= 7.0 && avg <= 13.0) currentClass = "LIGHT";
        else if (avg >= 17.0 && avg <= 22.0) currentClass = "MEDIUM";
        else if (avg >= 23.0 && avg <= 27.0) currentClass = "HEAVY";
        
        //izpiši barvo in težo valjčka (vrednost prevere vmesna koda Python)
        if (currentColor != lastOutputColor || abs(avg - lastOutputWeight) > TOLERANCE) {
          Serial.print("COLOR: "); Serial.print(colorID);
          Serial.print(" | WEIGHT CLASS: "); Serial.println(currentClass);
          lastOutputColor = currentColor;
          lastOutputWeight = avg;
          alreadyReported = true;
        }
      }
      if (!stable) alreadyReported = false;
    }
  }

  if (LoadCell.getData() < 4.5 && alreadyReported) {
    alreadyReported = false;
    bufferIndex = 0;
    bufferFull = false;
    for (byte i = 0; i < STABLE_READS; i++) weightBuffer[i] = 0;
    lastOutputColor = "NONE";
    lastOutputWeight = 0.0;
  }

  delay(200);
}
