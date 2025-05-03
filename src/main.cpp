#include <Arduino.h>
#include "core/uart/uart_manager.h"
#include <BluetoothA2DPSource.h> // <-- juste inclure la lib
#include "core/bluetooth/bluetooth_manager.h"
#include "core/bluetooth/bt_pairing_manager.h"
#include "core/audio/audio_manager.h"

void setup() {
  Serial.begin(115200);      
  delay(500); // Laisse le temps à Serial USB de se stabiliser
  Serial.println("[ESP2][MAIN] Démarrage setup...");  
  bt_pairing_manager::init();
  uart_manager::init(); // Initialise UART avec Serial2
  bluetooth_manager::init();    // Initialise le Bluetooth
  //audio_manager::init();
}

void loop() {
  
  uart_manager::loop(); // Gestion UART
  bluetooth_manager::loop();  // Gestion Bluetooth s'il y a un besoin (on verra plus tard)
}
