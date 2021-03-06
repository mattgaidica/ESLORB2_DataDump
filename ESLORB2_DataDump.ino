/*
  ESLORB2 Data Dump Utility
  by Matt Gaidica
  [ ] add interrupt to D7 (CARD_DETECT)
*/

#include <SPI.h>
#include <SD.h>

const int ESLO_SD_CS = 4;
const int ESLO_RESET = 6;
const int ESLO_DEBUG = 5;
const int ESLO_LED_R = 13;
const int ESLO_LED_G = 8;
const int ESLO_BTN = 10;
const int CARD_DETECT = 7;
int fadeValue = 0;

bool doESLO = false;
bool fadeDir = true;
bool sdIn = false;

void setup() {
  pinMode(ESLO_RESET, OUTPUT); // active LOW
  pinMode(ESLO_DEBUG, OUTPUT); // active LOW
  pinMode(ESLO_LED_R, OUTPUT);
  pinMode(ESLO_LED_G, OUTPUT);
  pinMode(ESLO_BTN, INPUT_PULLUP); // active LOW
  pinMode(CARD_DETECT, INPUT_PULLUP); // active LOW

  digitalWrite(ESLO_RESET, HIGH);
  digitalWrite(ESLO_DEBUG, HIGH);
  analogWrite(ESLO_LED_R, 0);
  digitalWrite(ESLO_LED_G, LOW);
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial1.begin(115200);
  // do not wait, computer is optional
//  while (!Serial) {
//  }

  Serial.print("Initializing SD card...");
  initSD();
}

bool initSD() {
  delay(100); // debounce
  while(digitalRead(CARD_DETECT) == LOW) {
    analogWrite(ESLO_LED_R, 0);
    digitalWrite(ESLO_LED_G, HIGH);
    delay(100);
    analogWrite(ESLO_LED_R, 255);
    digitalWrite(ESLO_LED_G, LOW);
    delay(100);
  }
  delay(100);
  if (!SD.begin(ESLO_SD_CS)) {
    Serial.println("Card failed, or not present");
    return false;
  }
  Serial.println("card initialized.");
  return true;
}

void readESLO() {
  analogWrite(ESLO_LED_R, 255);
  digitalWrite(ESLO_LED_G, LOW); // keep LOW until incoming serial data
  Serial.println("Opening file...");
  String filename = "ESLORB2.txt";
  SD.remove(filename);
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    Serial.println("File open.");
    digitalWrite(ESLO_DEBUG, LOW);
    delay(100);
    toggleReset();
    // ESLO should be in serial mode now
    unsigned long lastSerialTime = millis();
    unsigned long curTime;
    bool doLoop = true;
    while(doLoop) {
      if(Serial1.available()) {
        uint8_t data = Serial1.read();
        dataFile.write(&data, sizeof(uint8_t));
        Serial.println(data, HEX); // !! rm for production
        digitalWrite(ESLO_LED_G, !digitalRead(ESLO_LED_G));
        lastSerialTime = millis();
      }
      
      curTime = millis();
      if(curTime - lastSerialTime > 1000) {
        doLoop = false; // timeout condition
        Serial.println("Serial1 timeout.");
      }
    }

    dataFile.close();
    Serial.println("File closed.");
  } else {
    Serial.println("Error opening data file.");
  }
  digitalWrite(ESLO_DEBUG, HIGH);
  analogWrite(ESLO_LED_R, 0);
  digitalWrite(ESLO_LED_G, LOW);
  delay(1000); // debounce quick transfers
//  attachInterrupt(digitalPinToInterrupt(ESLO_BTN), btnPressed, FALLING);
  Serial.println("Done, returning to loop.");
}

void toggleReset() {
  digitalWrite(ESLO_RESET, LOW);
  delay(100);
  digitalWrite(ESLO_RESET, HIGH);
}


void loop() {
  while(sdIn == false || digitalRead(CARD_DETECT) == LOW) {
    sdIn = initSD();
  }
  
  // interrupt is being finnicky, just poll
  if(digitalRead(ESLO_BTN) == LOW) {
    doESLO = true;
  }
  
  if(doESLO) {
    doESLO = false;
    readESLO(); // resets doESLO to false
  }

  analogWrite(ESLO_LED_R, fadeValue);
  if (fadeValue == 0x2F) {
    fadeDir = false;
  }
  if (fadeValue == 0x00) {
    fadeDir = true;
  }
  if (fadeDir) {
    ++fadeValue;
  } else {
    --fadeValue;
  }
  
  delay(20);
}
