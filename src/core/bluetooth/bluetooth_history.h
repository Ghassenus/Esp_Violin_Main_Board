#pragma once

#include <Arduino.h>

namespace bluetooth_history {

    // Charger l'historique depuis la mémoire flash
    void load();

    // Ajouter un périphérique Bluetooth à l'historique
    void add(const String& mac, const String& name);

    // Supprimer tout l'historique Bluetooth
    void clear();

    // Envoyer la liste complète sur l'UART (format lisible)
    void send_list_over_uart();

    // Supprimer un device de la liste par par son adresse mac
    void remove(const String& mac);


}