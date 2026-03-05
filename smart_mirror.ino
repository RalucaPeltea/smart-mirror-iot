#include <FastLED.h> //biblioteci folosite
#include "BluetoothSerial.h"

// --- Initializare Banda LED ---
#define LED_PIN     5
#define NUM_LEDS    20
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

// --- Initializare senzori ---
#define PIR_PIN     16
#define TOUCH_PIN   17
#define LDR_PIN     34

CRGB leds[NUM_LEDS]; //retine ce culoare are fiecare led

// --- Variabile de stare ---
String mode = "AUTO";        // OFF / ON / AUTO
bool ledState = false;       // LED-urile aprinse sau stinse
unsigned long lastPIRTime = 0;
const unsigned long PIR_TIMEOUT = 5000; // dupa 5 sec de la detectarea miscarii se sting ledurile

// --- Variabile Touch ---
bool touchPrev = false;
unsigned long touchLastTime = 0;
const unsigned long TOUCH_DEBOUNCE = 200; // definește un timp minim de 200 ms intre doua atingeri ale butonului touch

BluetoothSerial SerialBT;

void setup() { //functie care se executa o singura data
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  pinMode(PIR_PIN, INPUT);
  pinMode(TOUCH_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  Serial.begin(115200); //inițializează comunicația serială între microcontroler și calculator

  // Bluetooth
  SerialBT.begin("SmartMirror");    //cum apare dispozitivul denumit
  Serial.println("Bluetooth activ, nume: SmartMirror");

  turnOffLEDs(); //initial, ledurile sunt stinse 
}

void loop() { //functie care se executa in loop
  // --- Citire comenzi Bluetooth ---
  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();
    if(cmd == "ON" || cmd == "OFF" || cmd == "AUTO") {
      mode = cmd;
      Serial.println("Mode set by BT: " + mode);
    }
  }

  // --- Citire buton touch ---
  bool touchNow = digitalRead(TOUCH_PIN);
  if(touchNow && !touchPrev && millis() - touchLastTime > TOUCH_DEBOUNCE) {
    // Toggle între OFF → ON → AUTO → OFF
    if(mode == "OFF") mode = "ON";
    else if(mode == "ON") mode = "AUTO";
    else if(mode == "AUTO") mode = "OFF";

    Serial.println("Mode set by TOUCH: " + mode);
    touchLastTime = millis();
  }
  touchPrev = touchNow;

  // --- Mod AUTO / ON / OFF ---
  if(mode == "AUTO") {
    if(digitalRead(PIR_PIN) == HIGH) {
      lastPIRTime = millis();
      ledState = true;
    }
    if(millis() - lastPIRTime > PIR_TIMEOUT) ledState = false;
  }
  else if(mode == "ON") ledState = true;
  else if(mode == "OFF") ledState = false;

  // --- Ajustare luminozitate LDR ---
  int lux = analogRead(LDR_PIN); // valoarea citita de senzorul ldr
  int brightness = map(lux, 4095, 0, 255, 50); // invers: întuneric = mai luminos
  FastLED.setBrightness(brightness);

  // --- Actualizare LED-uri ---
  if(ledState) fill_solid(leds, NUM_LEDS, CRGB::White);
  else turnOffLEDs();

  FastLED.show();
  delay(50);
}

// --- Funcție stingere LED-uri ---
void turnOffLEDs() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}
