#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include "readFile.h"
#include "secrets.h"
#include <ESP_Mail_Client.h>
#include <LittleFS.h>
#include <time.h>

// Pin definitions
#define BUTTON_PIN 0  // GPIO0 for the onboard button
#define LDR_PIN 34    // GPIO34 for LDR
#define LED_PIN 2     // GPIO2 for onboard LED
#define THRESHOLD 0.8 // Voltage threshold for LDR

// Time configurations
const char *ntpServer = "pool.ntp.org";
const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3"; // POSIX TZR für DE/CH/AT

// SMTP session
SMTPSession smtp;

// Variables to track timestamps and duration
String activationTimestamp = "N/A";
String emergencyStopTimestamp = "N/A";
String duration = "N/A";
unsigned long activationStart = 0;
bool emergencyStopTriggered =
    false; // Tracks if emergency stop has been triggered

// Function prototypes
void smtpCallback(SMTP_Status status);
void sendEmail(const String &subject, const char *plainTextPath,
               const char *htmlPath, const String &extraInfo);
String readFile(fs::FS &fs, const char *path);
String getFormattedTime();
String calculateDuration(unsigned long startMillis);

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  if (!LittleFS.begin(true)) {
    Serial.println("Failed to mount LittleFS");
    return;
  }
  Serial.println("LittleFS mounted successfully.");

  // List all files in LittleFS
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.print("File: ");
    Serial.println(file.name());
    file = root.openNextFile();
  }

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nWi-Fi connected.");

  // Initialize NTP with automatic timezone and DST
  configTzTime(time_zone, ntpServer);
  Serial.print("NTP initialized. Waiting for time sync");
  
  // Wait for NTP synchronization before sending any emails
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    Serial.print(".");
    delay(500);
    now = time(nullptr);
  }
  Serial.println("\nTime synced.");

  // Store activation timestamp
  activationTimestamp = getFormattedTime();
  activationStart = millis();

  // Send activation email
  sendEmail("ESP32 Activation Notification", "/activation_plain.txt",
            "/activation_html.html", "Activation Time: " + activationTimestamp);
}

void loop() {
  static bool buttonState = HIGH;
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;

  static unsigned long lastLdrCheck = 0;
  const unsigned long ldrCheckInterval = 1000;

  // Debug LDR readings
  static unsigned long lastPrintTime = 0;
  const unsigned long printInterval = 1000;
  if (millis() - lastPrintTime >= printInterval) {
    lastPrintTime = millis();
    int analogValue = analogRead(LDR_PIN);
    float voltage = (analogValue / 4095.0) * 3.3;
    Serial.printf("Raw Analog Value: %d | Voltage: %.2f V\n", analogValue,
                  voltage);
  }

  // Read button state
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW && !emergencyStopTriggered) {
        Serial.println("Button pressed, sending email...");
        sendEmail("ESP32 Notification: Button Press", "/button_plain.txt",
                  "/button_html.html", "");
      }
    }
  }
  lastButtonState = reading;

  // LDR Check Interval
  if (millis() - lastLdrCheck > ldrCheckInterval) {
    lastLdrCheck = millis();
    int analogValue = analogRead(LDR_PIN);
    float voltage = (analogValue / 4095.0) * 3.3;

    Serial.printf("LDR Voltage: %.2f V\n", voltage);

    // Emergency Stop Logic (only once)
    if (voltage > THRESHOLD && !emergencyStopTriggered) {
      emergencyStopTriggered = true; // Ensure only one email is sent
      emergencyStopTimestamp = getFormattedTime();
      duration = calculateDuration(activationStart);

      Serial.println(
          "Licht erkannt! Emergency Stop triggered. Sending email...");
      digitalWrite(LED_PIN, HIGH); // Turn on LED to indicate stop

      sendEmail("ESP32 Notification: Emergency Stop", "/ldr_plain.txt",
                "/ldr_html.html",
                "Emergency Stop Time: " + emergencyStopTimestamp +
                    "\nDuration: " + duration);
    }
  }
}

void sendEmail(const String &subject, const char *plainTextPath,
               const char *htmlPath, const String &extraInfo) {
  String plainTextMsg = readFile(LittleFS, plainTextPath);
  String htmlMsg = readFile(LittleFS, htmlPath);

  // Get the current timestamp
  String currentTime = getFormattedTime();

  // Replace placeholders
  htmlMsg.replace("{{timestamp}}", currentTime);
  htmlMsg.replace("{{device_ip}}", WiFi.localIP().toString());
  htmlMsg.replace("{{extra_info}}", extraInfo);
  plainTextMsg.replace("{{timestamp}}", currentTime);
  plainTextMsg.replace("{{device_ip}}", WiFi.localIP().toString());
  plainTextMsg.replace("{{extra_info}}", extraInfo);

  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;

  SMTP_Message message;
  message.sender.name = "ESP32 Notification System";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = subject;
  message.addRecipient("Recipient Name", RECIPIENT_EMAIL);

  message.text.content = plainTextMsg.c_str();
  message.html.content = htmlMsg.c_str();

  if (!smtp.connect(&config)) {
    Serial.printf("Connection error: %s\n", smtp.errorReason().c_str());
    return;
  }

  if (!MailClient.sendMail(&smtp, &message, true)) {
    Serial.printf("Error sending Email: %s\n", smtp.errorReason().c_str());
  } else {
    Serial.println("Email sent successfully!");
  }

  smtp.sendingResult.clear();
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "N/A";
  }
  char timeString[64];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}

String calculateDuration(unsigned long startMillis) {
  unsigned long durationMillis = millis() - startMillis;
  unsigned long seconds = durationMillis / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  minutes %= 60;
  seconds %= 60;
  char durationStr[32];
  sprintf(durationStr, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(durationStr);
}
