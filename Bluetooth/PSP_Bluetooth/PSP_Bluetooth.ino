void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}#include "BluetoothSerial.h"

// Verifica se il Bluetooth è abilitato correttamente nel core
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

void setup() {
  // Inizializza la comunicazione seriale fisica (USB)
  Serial.begin(115200);

  // Inizializza il Bluetooth con il nome del dispositivo
  // Questo è il nome che vedrai durante il pairing
  if (!SerialBT.begin("PSP_Bluetooth_ESP32")) {
    Serial.println("Errore durante l'inizializzazione Bluetooth!");
    return;
  }

  Serial.println("Il dispositivo è pronto per il pairing.");
  Serial.println("Nome Bluetooth: ESP32_Serial_Bridge");
}

void loop() {
  // 1. Legge dalla Seriale USB e invia al Bluetooth
  if (Serial.available()) {
    // Leggiamo un byte alla volta per mantenere la latenza bassa
    char incomingChar = Serial.read();
    SerialBT.write(incomingChar);
  }

  // 2. Legge dal Bluetooth e invia alla Seriale USB (Opzionale)
  if (SerialBT.available()) {
    char incomingBT = SerialBT.read();
    Serial.write(incomingBT);
  }
}
