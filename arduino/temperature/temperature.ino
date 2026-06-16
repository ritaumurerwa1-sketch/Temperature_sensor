/*
  =============================================
  HARDWARE CONNECTION VERIFICATION — CONFIRMED
  =============================================

  DHT11 MODULE (3-pin: S, middle, —):
    S    → Arduino Digital Pin 7   (signal / data)
    VCC  → Arduino IOREF           (= 5V on Uno, electrically identical to 5V)
    —    → Arduino GND

  I2C LCD MODULE (4-pin: GND, VCC, SDA, SCL):
    GND  → Arduino GND
    VCC  → Arduino 5V
    SDA  → Arduino A4              (I2C data, handled by Wire library)
    SCL  → Arduino A5              (I2C clock, handled by Wire library)
    I2C Address: 0x27  (change to 0x3F in lcd() constructor if display stays blank)

  POWER NOTE:
    Arduino has 1x 5V pin  → goes to LCD VCC
    DHT11 powered via IOREF pin (same as 5V on Uno — no code difference)
    Arduino has 3x GND pins → used for LCD GND and DHT11 — separately
    DHT11 module has built-in pull-up resistor — no external resistor or pinMode needed

  LIBRARY REQUIREMENTS:
    - LiquidCrystal_I2C by Frank de Brabander  (install via Arduino Library Manager)
    - DHT sensor library by Adafruit           (install via Arduino Library Manager)
    - Adafruit Unified Sensor                  (dependency, install alongside DHT library)
  =============================================
*/

// ============================================================
// Temperature Display and MQTT Monitoring System
// Trade Code: SPE — Embedded Systems Software Integration
// Author: user273
//
// Hardware:
//   - DHT11 sensor on Digital Pin 7
//   - I2C 16x2 LCD: SDA=A4, SCL=A5, I2C address 0x27
//     (change to 0x3F if display stays blank)
// ============================================================

#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ---- DHT11 Configuration ----
#define DHTPIN  7
#define DHTTYPE DHT11

// ---- I2C LCD Configuration ----
// Address 0x27 is the most common; use 0x3F if display stays blank
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---- DHT Object ----
DHT dht(DHTPIN, DHTTYPE);

// ---- Candidate Name ----
const String candidateName = "Bonnette Umurerwa";  // 7 chars — will display statically

// ---- Scrolling Variables ----
String paddedName       = "";
int    scrollIndex      = 0;
unsigned long lastScrollTime = 0;
const unsigned long SCROLL_INTERVAL = 300; // ms per scroll step

// ---- Temperature Timing ----
unsigned long lastTempTime = 0;
const unsigned long TEMP_INTERVAL = 2000; // ms between readings

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(9600);
  Serial.println("SPE Temperature Monitor — user273");
  Serial.println("Initialising DHT11 and LCD...");

  lcd.init();       // I2C initialisation (replaces lcd.begin() for parallel LCD)
  lcd.backlight();  // Turn on the backlight
  dht.begin();

  // Sensor warm-up (only delay() allowed — startup only)
  delay(2000);

  // Prepare padded name for scrolling logic (reusable if name > 16 chars)
  if (candidateName.length() > 16) {
    // Pad with 16 spaces on each side
    String spaces = "                "; // 16 spaces
    paddedName = spaces + candidateName + spaces;
  } else {
    paddedName = candidateName;
  }

  // Display name on LCD row 0 immediately
  lcd.setCursor(0, 0);
  if (candidateName.length() <= 16) {
    lcd.print(candidateName);
  } else {
    lcd.print(paddedName.substring(0, 16));
  }

  // Placeholder on row 1 while first reading loads
  lcd.setCursor(0, 1);
  lcd.print("Temp: --.- ");
  lcd.print((char)223);
  lcd.print("C");

  Serial.println("Ready. Sending readings every 2s.");
}

// ============================================================
// LOOP — only calls handler functions, no delay()
// ============================================================
void loop() {
  handleScroll();
  handleTemperature();
}

// ============================================================
// handleScroll()
// Scrolls row 0 only when candidate name > 16 characters.
// Uses millis() for non-blocking 300 ms intervals.
// ============================================================
void handleScroll() {
  if (candidateName.length() <= 16) {
    // Name fits — nothing to scroll
    return;
  }

  unsigned long now = millis();
  if (now - lastScrollTime >= SCROLL_INTERVAL) {
    lastScrollTime = now;

    // Display 16-character window starting at scrollIndex
    lcd.setCursor(0, 0);
    lcd.print(paddedName.substring(scrollIndex, scrollIndex + 16));

    // Advance index, wrap around
    scrollIndex++;
    if (scrollIndex > (int)(paddedName.length() - 16)) {
      scrollIndex = 0;
    }
  }
}

// ============================================================
// handleTemperature()
// Reads DHT11 every 2000 ms using millis().
// Updates LCD row 1 and sends serial data.
// ============================================================
void handleTemperature() {
  unsigned long now = millis();
  if (now - lastTempTime < TEMP_INTERVAL) {
    return;
  }
  lastTempTime = now;

  float temperature = dht.readTemperature(); // Celsius

  // Check for failed read
  if (isnan(temperature)) {
    // ---- Error path ----
    Serial.println("TEMP:ERROR");
    Serial.println("DHT11 Read FAILED — check wiring and pull-up resistor");

    lcd.setCursor(0, 1);
    lcd.print("Temp: Error     ");
    return;
  }

  // ---- Successful read ----
  // Serial: machine-readable line
  Serial.print("TEMP:");
  Serial.println(temperature, 1);   // 1 decimal place, e.g. "TEMP:24.5"

  // Serial: human-readable debug line
  Serial.print("DHT11 Read OK - Temp: ");
  Serial.print(temperature, 1);
  Serial.println("C");

  // LCD row 1: "Temp: XX.X °C"
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temperature, 1);
  lcd.print(" ");
  lcd.print((char)223);  // degree symbol
  lcd.print("C");
  // Pad remaining characters to clear any leftover digits
  lcd.print("   ");
}
