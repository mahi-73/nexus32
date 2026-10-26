#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <time.h>

#define WIFI_SSID     "mohan"
#define WIFI_PASS     "mahi@8985"

#define APP_KEY       "6f679963-8260-4a30-a6af-89a3cb373945"
#define APP_SECRET    "4a3b1765-e286-4e6b-a0d7-6ad88b535222-84d77242-add4-43f1-ac94-39c69907f042"

#define DEVICE_ID_1   "6a909ef309efd1746c4db88d"
#define DEVICE_ID_2   "6a909f3129c6be3342a06de4"
#define DEVICE_ID_3   "6a909f5d969af7ec249457c6"
#define DEVICE_ID_4   "YOUR_DEVICE_ID_4"

#define RELAY_1   23
#define RELAY_2   22
#define RELAY_3   21
#define RELAY_4   19

#define PIR_PIN   34

#define RELAY_ACTIVE   LOW
#define RELAY_INACTIVE HIGH

bool relayState1 = false;
bool relayState2 = false;
bool relayState3 = false;
bool relayState4 = false;

unsigned long lastMotionTime = 0;
bool motionTimerRunning = false;

unsigned long relay1TurnedOnTime = 0;
bool pendingRelay2Activation = false;
const unsigned long RELAY2_DELAY = 15000;
const unsigned long MOTION_TIMEOUT = 300000;

unsigned long lastDebugPrint = 0;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;

void setRelay(int pin, bool state) {
  digitalWrite(pin, state ? RELAY_ACTIVE : RELAY_INACTIVE);
}

bool isNightTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return true;
  int currentHour = timeinfo.tm_hour;
  return (currentHour >= 18 || currentHour < 5);
}

bool onPowerState1(const String &deviceId, bool &state) {
  relayState1 = state;
  setRelay(RELAY_1, relayState1);
  if (state) {
    lastMotionTime = millis();
    motionTimerRunning = true;
    relay1TurnedOnTime = millis();
    pendingRelay2Activation = true;
  } else {
    pendingRelay2Activation = false;
  }
  return true;
}

bool onPowerState2(const String &deviceId, bool &state) {
  relayState2 = state;
  setRelay(RELAY_2, relayState2);
  if (state) {
    lastMotionTime = millis();
    motionTimerRunning = true;
  }
  return true;
}

bool onPowerState3(const String &deviceId, bool &state) {
  relayState3 = state;
  setRelay(RELAY_3, relayState3);
  return true;
}

bool onPowerState4(const String &deviceId, bool &state) {
  relayState4 = state;
  setRelay(RELAY_4, relayState4);
  return true;
}

void handlePIRSensor() {
  int pirState = digitalRead(PIR_PIN);
  bool nightWindowActive = isNightTime();
  unsigned long currentMillis = millis();

  if (currentMillis - lastDebugPrint >= 2000) {
    lastDebugPrint = currentMillis;
    long remainingSec = 0;
    if (motionTimerRunning && (lastMotionTime + MOTION_TIMEOUT > currentMillis)) {
      remainingSec = (lastMotionTime + MOTION_TIMEOUT - currentMillis) / 1000;
    }
    Serial.printf("PIR: %d | Night: %s | R1: %s | R2: %s | Auto-Off in: %ld s
",
                  pirState,
                  nightWindowActive ? "YES" : "NO",
                  relayState1 ? "ON" : "OFF",
                  relayState2 ? "ON" : "OFF",
                  remainingSec);
  }

  if (nightWindowActive && pirState == HIGH) {
    lastMotionTime = currentMillis;
    motionTimerRunning = true;

    if (!relayState1) {
      relayState1 = true;
      setRelay(RELAY_1, relayState1);
      SinricProSwitch &sw1 = SinricPro[DEVICE_ID_1];
      sw1.sendPowerStateEvent(relayState1);

      relay1TurnedOnTime = currentMillis;
      pendingRelay2Activation = true;
      Serial.println(">>> Motion detected: Relay 1 ON. Relay 2 scheduled in 15 seconds <<<");
    }
  }

  if (pendingRelay2Activation && relayState1 && (currentMillis - relay1TurnedOnTime >= RELAY2_DELAY)) {
    pendingRelay2Activation = false;

    if (!relayState2) {
      relayState2 = true;
      setRelay(RELAY_2, relayState2);
      SinricProSwitch &sw2 = SinricPro[DEVICE_ID_2];
      sw2.sendPowerStateEvent(relayState2);
      Serial.println(">>> 15 seconds passed: Relay 2 turned ON <<<");
    }
  }

  if (motionTimerRunning && pirState == LOW && (currentMillis - lastMotionTime >= MOTION_TIMEOUT)) {
    motionTimerRunning = false;
    pendingRelay2Activation = false;

    if (relayState1) {
      relayState1 = false;
      setRelay(RELAY_1, relayState1);
      SinricProSwitch &sw1 = SinricPro[DEVICE_ID_1];
      sw1.sendPowerStateEvent(relayState1);
    }

    if (relayState2) {
      relayState2 = false;
      setRelay(RELAY_2, relayState2);
      SinricProSwitch &sw2 = SinricPro[DEVICE_ID_2];
      sw2.sendPowerStateEvent(relayState2);
    }

    Serial.println(">>> TIMEOUT: 5 minutes reached -> BOTH RELAYS OFF <<<");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);

  setRelay(RELAY_1, false);
  setRelay(RELAY_2, false);
  setRelay(RELAY_3, false);
  setRelay(RELAY_4, false);

  pinMode(PIR_PIN, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("
Connected to Wi-Fi! IP: " + WiFi.localIP().toString());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  SinricProSwitch &sw1 = SinricPro[DEVICE_ID_1];
  sw1.onPowerState(onPowerState1);

  SinricProSwitch &sw2 = SinricPro[DEVICE_ID_2];
  sw2.onPowerState(onPowerState2);

  SinricProSwitch &sw3 = SinricPro[DEVICE_ID_3];
  sw3.onPowerState(onPowerState3);

  SinricProSwitch &sw4 = SinricPro[DEVICE_ID_4];
  sw4.onPowerState(onPowerState4);

  SinricPro.begin(APP_KEY, APP_SECRET);
}

void loop() {
  SinricPro.handle();
  handlePIRSensor();
}
