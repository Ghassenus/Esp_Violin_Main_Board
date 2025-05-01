#pragma once
#include <Arduino.h>

struct UartMessage {
    String type;
    String data;
};

// Fonction pour parser un message du type "{TYPE:DATA}"
inline bool uart_parse_message(const String& raw, UartMessage& msg) {
    if (!raw.startsWith("{") || !raw.endsWith("}")) {
        return false; // format invalide
    }

    int colonIndex = raw.indexOf(':');
    if (colonIndex == -1) {
        return false; // Pas de séparateur trouvé
    }

    msg.type = raw.substring(1, colonIndex);
    msg.data = raw.substring(colonIndex + 1, raw.length() - 1);
    return true;
}
