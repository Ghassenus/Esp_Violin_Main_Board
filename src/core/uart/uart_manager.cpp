#include "uart_manager.h"
#include "core/protocol/uart_parser.h"
#include <core/bluetooth/bluetooth_manager.h>

#define UART_ESP1 Serial2
#define UART_BAUD 115200
#define UART_RX_PIN 35
#define UART_TX_PIN 23

namespace uart_manager {

static String uart_buffer;

void init() {
    UART_ESP1.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    delay(100);
    // Vider tous les caractères parasites

    while (UART_ESP1.available()) {
    UART_ESP1.read();
    }
    
    Serial.println("[ESP2][UART] UART2 ready ");
}

void loop() {
    while (UART_ESP1.available()) {
        char c = UART_ESP1.read();
        if (c == '\n') {
            uart_buffer.trim();
            if (uart_buffer.length() > 0) {
                Serial.println("[ESP2][UART] Reçu: " + uart_buffer);

                UartMessage msg;
                if (uart_parse_message(uart_buffer, msg)) {
                    Serial.println("[ESP2][UART] Type=" + msg.type + " Data=" + msg.data);

                    // ⬇️ Dispatch vers Bluetooth Manager
                    bluetooth_manager::handle_uart_command(msg.type, msg.data);

                } else {
                    Serial.println("[ESP2][UART] Erreur de parsing !");
                }
            }
            uart_buffer = "";
        } else {
            uart_buffer += c;
        }
    }
}


void send_formatted(const char* type, const String& data) {
    String packet = "{" + String(type) + ":" + data + "}";
    UART_ESP1.println(packet);
}

} // namespace
