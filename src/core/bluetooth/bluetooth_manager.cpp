#include "bluetooth_manager.h"
#include "core/uart/uart_manager.h"
#include "core/bluetooth/bluetooth_history.h"
#include "core/bluetooth/bt_pairing_manager.h"
#include "logger.h"

#include <BluetoothA2DPSource.h>

static BluetoothA2DPSource a2dp_source;

namespace bluetooth_manager {

void init() {
    log_info("[BT] Initialisation A2DP Source...");

    // Définir nom visible côté A2DP
    a2dp_source.set_local_name("Violin_Setup");

    // Gestion des événements de connexion
    a2dp_source.set_on_connection_state_changed([](esp_a2d_connection_state_t state, void*) {
        if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
            log_info("[BT] Connexion A2DP établie !");
            uart_manager::send_formatted("CONNECT_OK", "Périphérique connecté");
        } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
            log_warn("[BT] Déconnecté A2DP");
            uart_manager::send_formatted("DISCONNECTED", "Connexion perdue");
        }
    });

    // Démarrer le profil A2DP (source audio)
    a2dp_source.start();
    log_info("[BT] A2DP Source démarré");

    // Charger historique
    bluetooth_history::load();
}

void loop() {
    // Rien pour l’instant
}

void scan_devices() {
    log_info("[BT] Scan Bluetooth demandé (10s)...");
    bt_pairing_manager::start_scan();
}

void stop_scan_devices() {
    log_info("[BT] Scan Bluetooth stoppé manuellement");
    bt_pairing_manager::stop_scan();
}

void connect_device(const String& mac_address) {
    log_info("[BT] Connexion à " + mac_address);

    uint8_t mac[6];
    int vals[6];

    if (sscanf(mac_address.c_str(), "%x:%x:%x:%x:%x:%x",
               &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5]) != 6) {
        uart_manager::send_formatted("ERROR", "Adresse MAC invalide !");
        return;
    }

    for (int i = 0; i < 6; ++i) mac[i] = (uint8_t)vals[i];

    if (!a2dp_source.connect_to(mac)) {
        uart_manager::send_formatted("ERROR", "Connexion A2DP échouée !");
    } else {
        log_info("[BT] Tentative de connexion A2DP envoyée");
    }
}

void handle_uart_command(const String& type, const String& data) {
    if (type == "BT_SCAN") {
        scan_devices();
    } else if (type == "BT_STOP_SCAN") {
        stop_scan_devices();
    } else if (type == "BT_CONNECT") {
        connect_device(data);
    } else if (type == "BT_REMOVE") {
        bluetooth_history::remove(data);
        uart_manager::send_formatted("REMOVE_OK", data);
    } else if (type == "BT_LIST") {
        bluetooth_history::send_list_over_uart();
    } else {
        uart_manager::send_formatted("ERROR", "Commande inconnue: " + type);
    }
}

}  // namespace
