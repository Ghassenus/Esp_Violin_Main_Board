#include "bluetooth_manager.h"
#include "core/uart/uart_manager.h"
#include "logger.h"
#include "core/bluetooth/bluetooth_history.h"

#include <BluetoothA2DPSource.h>
#include <BluetoothSerial.h>

static BluetoothA2DPSource a2dp_source;
static BluetoothSerial bt_serial;

namespace bluetooth_manager {

void init() {
    if (!btStart()) {
        log_error("[BT] Impossible de démarrer Bluetooth !");
        return;
    }

    bt_serial.begin("ESP2_Violin");
    log_info("[BT] Bluetooth initialisé en mode A2DP Source");

    bluetooth_history::load();
}

void loop() {
    // Pour le moment rien à faire en continu ici
}

void scan_devices() {
    log_info("[BT] Scan Bluetooth lancé...");

    bool ok = bt_serial.discoverAsync([](BTAdvertisedDevice* device) {
        if (device) {
            String name = device->getName().c_str();
            String mac = device->getAddress().toString().c_str();
            log_info("[BT] Trouvé : " + name + " [" + mac + "]");

            bluetooth_history::add(mac, name); // Mise à jour de l'historique
            uart_manager::send_formatted("SCAN_RESULT", name + " [" + mac + "]");
        }
    });

    if (!ok) {
        uart_manager::send_formatted("ERROR", "Scan Bluetooth échoué !");
    } else {
        uart_manager::send_formatted("SCAN_END", "");
    }
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

    bool ok = a2dp_source.connect_to(mac);
    if (ok) {
        uart_manager::send_formatted("CONNECT_OK", mac_address);
    } else {
        uart_manager::send_formatted("ERROR", "Connexion A2DP échouée !");
    }
}

void handle_uart_command(const String& type, const String& data) {
    if (type == "BT_SCAN") {
        scan_devices();
    } else if (type == "BT_CONNECT") {
        connect_device(data);
    } else if (type == "BT_REMOVE") {
        bluetooth_history::remove(data);
        uart_manager::send_formatted("REMOVE_OK", data); // Confirmation vers ESP1
    }else if (type == "BT_LIST") {
        bluetooth_history::send_list_over_uart();
    }
    
    else {
        uart_manager::send_formatted("ERROR", "Commande inconnue: " + type);
    }
}

} // namespace
