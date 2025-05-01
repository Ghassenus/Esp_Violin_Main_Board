#pragma once

#include <Arduino.h>

namespace uart_manager {

// Initialisation UART
void init();

// À appeler dans la loop()
void loop();

// Envoyer une réponse vers ESP1
void send_message(const String& msg);

// Envoyer un message rapide formaté
void send_formatted(const char* type, const String& data);

}
