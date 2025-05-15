#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NewPing.h>

// WiFi and Blynk credentials
char auth[] = "G5ofN7gi8rOUsVJ-DthfD1MSvv1Ba2hH";
char ssid[] = "123456789";
char pass[] = "123456789";

// Ultrasonic sensor pins and setup
#define TRIG_PIN D7
#define ECHO_PIN D6
#define MAX_DISTANCE 200
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// TDS Sensor analog pin
const int TDS_PIN = A0;

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Blynk timer
BlynkTimer timer;

void setup() {
  // Start serial and LCD
  Serial.begin(9600);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Water Monitor in ");
  lcd.setCursor(0,1);
  lcd.print("seawage water");
  delay(4000);
  lcd.clear();

  // Connect to WiFi and Blynk
  connectToWiFi();
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  // Set timer to send distance every second
  timer.setInterval(1000L, sendDistance);
}

void loop() {
  // Read analog value from TDS sensor
  int analogValue = analogRead(TDS_PIN);
  float voltage = analogValue * (3.3 / 1024.0);

  // Calculate TDS value
  float tdsValue = (133.42 * voltage * voltage * voltage
                  - 255.86 * voltage * voltage
                  + 857.39 * voltage) * 0.5;

  // Display TDS value
  lcd.setCursor(0, 0);
  lcd.print("TDS:");
  lcd.print(tdsValue, 0);
  lcd.print("ppm");  // Padding to clear leftovers
Blynk.virtualWrite(V0,tdsValue);
  lcd.setCursor(8,0);
  if (tdsValue > 1000) {
    lcd.print(" Sewage   ");
  } else {
    lcd.print(" Clear    ");
  }

  // Run Blynk and timer
  Blynk.run();
  timer.run();

  // Keep WiFi alive
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  delay(1000);  // Optional; reduce frequency if needed
}

void sendDistance() {
  int distance = sonar.ping_cm();
  if (distance == 0) distance = MAX_DISTANCE;
  Blynk.virtualWrite(V1, distance);
  lcd.setCursor(0, 1);
  lcd.print("Distance:");
  lcd.setCursor(10, 1);
  lcd.print(distance);
  lcd.print("cm");
}

void connectToWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");

    WiFi.begin(ssid, pass);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
      delay(500);
      lcd.print(".");
      retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi Connected");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
      delay(1500);
      lcd.clear();
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi Failed");
      lcd.setCursor(0, 1);
      lcd.print("Retrying...");
      delay(2000);
    }
  }
}
