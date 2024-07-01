/*
 * Example on how to use the Wiegand reader library with interruptions.
 */
// https://github.com/paulo-raca/YetAnotherArduinoWiegandLibrary
#include <Wiegand.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

// These are the pins connected to the Wiegand D0 and D1 signals.
// Ensure your board supports external Interruptions on these pins


// The object that handles the wiegand protocol
Wiegand wiegand;


ESP8266WiFiMulti WiFiMulti;
const char* ssid = "Bumi";
const char* password = "indonizingtheworld";

// When any of the pins have changed, update the state of the wiegand library
void ICACHE_RAM_ATTR pinStateChanged() {
  wiegand.setPin0State(digitalRead(D5));
  wiegand.setPin1State(digitalRead(D6));
}

// Initialize Wiegand reader
void setup() {
  Serial.begin(9600);

  setupWIFI();
  //Install listeners and initialize Wiegand reader
  wiegand.onReceive(receivedData, "Card readed: ");
  wiegand.onReceiveError(receivedDataError, "Card read error: ");
  wiegand.onStateChange(stateChanged, "State changed: ");
  wiegand.begin(Wiegand::LENGTH_ANY, true);

  //initialize pins as INPUT and attaches interruptions
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  attachInterrupt(digitalPinToInterrupt(D5), pinStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(D6), pinStateChanged, CHANGE);

  //Sends the initial pin state to the Wiegand library
  pinStateChanged();
}

void setupWIFI() {
  // Connecting to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  while (WiFiMulti.run() != WL_CONNECTED)
  {
    delay(1500);
    Serial.println("connecting to wifi….");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Every few milliseconds, check for pending messages on the wiegand reader
// This executes with interruptions disabled, since the Wiegand library is not thread-safe
void loop() {
  noInterrupts();
  wiegand.flush();
  interrupts();
  //Sleep a little -- this doesn't have to run very often.
  delay(100);
}



// Notifies when a reader has been connected or disconnected.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onStateChange()`
void stateChanged(bool plugged, const char* message) {
  Serial.print(message);
  Serial.println(plugged ? "CONNECTED" : "DISCONNECTED");
}

// Notifies when a card was read.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onReceive()`
void receivedData(uint8_t* data, uint8_t bits, const char* message) {
  Serial.print(message);
  Serial.print(bits);
  Serial.print("bits / ");
  //Print value in HEX
  uint8_t bytes = (bits + 7) / 8;
   String id = "";
  for (int i = 0; i < bytes; i++) {
    // Serial.print(data[i] >> 4, 16);
    // Serial.print(data[i] & 0xF, 16);
       char hexChar[3];
        sprintf(hexChar, "%02X", data[i]); // Convert each byte to a hex string
        id += hexChar; // Append the hex string to id
  }
  Serial.println();
  Serial.println(id); // Print the entire id
 
  handleAPIPOST();
  delay(1500);
}

//send to api
void handleAPIPOST()
{
  String serverName = "https://arduinotesting.free.beeceptor.com/rfidpost";
  if ((WiFiMulti.run() == WL_CONNECTED))
  {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    // client->setFingerprint(fingerprint_sni_cloudflaressl_com);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, serverName))
    { // HTTPS

      Serial.print("[HTTPS] POST...\n");
      // start connection and send HTTP header
      // https.addHeader("Content-Type", "application/json");
      // String httpRequestData = "{\"api_key\":\"" + String(random(40)) + "\",\"field1\":\"" + String(random(40)) + "\"}";

      https.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String httpRequestData = "api_key=" + String(random(40)) + "&field1=" + String(random(40));
      int httpCode = https.POST(httpRequestData);

      // httpCode will be negative on error
      if (httpCode > 0)
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
          String payload = https.getString();
          // String payload = https.getString(1024);  // optionally pre-reserve string to avoid reallocations in chunk mode
          Serial.println(payload);
        }
      }
      else
      {
        Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    }
    else
    {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
}
void handleAPIGET()
{
  String serverName = "https://valdy-arduino.solusi-k8s.com/rfidget";
  if ((WiFiMulti.run() == WL_CONNECTED))
  {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    // client->setFingerprint(fingerprint_sni_cloudflaressl_com);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, serverName))
    { // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0)
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
          String payload = https.getString();
          // String payload = https.getString(1024);  // optionally pre-reserve string to avoid reallocations in chunk mode
          Serial.println(payload);
        }
      }
      else
      {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    }
    else
    {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
}

// Notifies when an invalid transmission is detected
void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
  Serial.print(message);
  Serial.print(Wiegand::DataErrorStr(error));
  Serial.print(" - Raw data: ");
  Serial.print(rawBits);
  Serial.print("bits / ");

  //Print value in HEX
  uint8_t bytes = (rawBits + 7) / 8;
  for (int i = 0; i < bytes; i++) {
    Serial.print(rawData[i] >> 4, 16);
    Serial.print(rawData[i] & 0xF, 16);
  }
  Serial.println();
}