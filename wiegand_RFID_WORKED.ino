/*
 * Example on how to use the Wiegand reader library with interruptions.
 */
// https://github.com/paulo-raca/YetAnotherArduinoWiegandLibrary
#include <Wiegand.h>

// These are the pins connected to the Wiegand D0 and D1 signals.
// Ensure your board supports external Interruptions on these pins


// The object that handles the wiegand protocol
Wiegand wiegand;
// When any of the pins have changed, update the state of the wiegand library
void ICACHE_RAM_ATTR pinStateChanged() {
  wiegand.setPin0State(digitalRead(D5));
  wiegand.setPin1State(digitalRead(D6));
}

// Initialize Wiegand reader
void setup() {
  Serial.begin(9600);

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
  for (int i = 0; i < bytes; i++) {
    Serial.print(data[i] >> 4, 16);
    Serial.print(data[i] & 0xF, 16);
  }
  Serial.println();
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