#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define soilMoistPin 35

const char* ssid = "metallica";
const char* password = "jlmerak6";

#define FIREBASE_URL "https://hygroplant-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyBrwjUB4uf0KTcQlNbkEept51eGUr50vI8"

int soilMoisture;
int soilMoisturePercent;
String uid;
String databasePath;

bool signupOK = false;

WiFiClient wifiClient;
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;
FirebaseJson json;
FirebaseData fbdo;

TaskHandle_t readMoistureTaskHandle;

void readMoistureTask(void *args);
void initFirebase();
void initWifi();

#define EEPROM_ADDR 0

void setup()
{
  Serial.begin(9600);
  initWifi();
  initFirebase();
  uid = ESP.getEfuseMac();
  Serial.println(uid);
  // xTaskCreatePinnedToCore(readMoistureTask, "MoistureTask", 1024, NULL, 0, &readMoistureTaskHandle, 0);
}

void loop()
{
  if (Firebase.ready())
  {
    soilMoisture = analogRead(soilMoistPin);
    soilMoisturePercent = map(soilMoisture, 4095, 0, 0, 100);
    Serial.println(soilMoisturePercent);
    Serial.println(soilMoisture);
    json.set("/moisture", soilMoisturePercent);
    databasePath = "/plants/" + uid;
    Serial.printf("%s", Firebase.RTDB.setJSON(&fbdo, databasePath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    delay(5000);
  }
}

void readMoistureTask(void *args)
{
  for (;;)
  {
    soilMoisture = analogRead(soilMoistPin);
    soilMoisturePercent = map(soilMoisture, 4095, 0, 0, 100);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

void initWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}

void initFirebase() {
  firebaseConfig.api_key = API_KEY;
  firebaseConfig.database_url = FIREBASE_URL;
  if (Firebase.signUp(&firebaseConfig, &firebaseAuth, "", "")){
  signupOK = true;
  }
  else  {
  Serial.printf("%s\n", firebaseConfig.signer.signupError.message.c_str());
  }
  firebaseConfig.token_status_callback = tokenStatusCallback;
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);
}