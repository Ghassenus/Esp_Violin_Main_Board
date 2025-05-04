#include "bluetooth_history.h"
#include "core/uart/uart_manager.h"  // pour envoyer sur UART
#include <Preferences.h>
#include <ArduinoJson.h>

static Preferences prefs;

namespace bluetooth_history {

static const char* KEY_BT_LIST = "bt_list";
static const size_t MAX_DEVICES = 10;

// Charger et afficher pour debug
void load() {
    prefs.begin("bluetooth", true);
    String raw = prefs.getString(KEY_BT_LIST, "[]");
    prefs.end();

    Serial.println("[BT_HIST] Historique chargé : " + raw);
}

// Sauvegarder l'historique
void save(const JsonArray& array) {
    String out;
    serializeJson(array, out);

    prefs.begin("bluetooth", false);
    prefs.putString(KEY_BT_LIST, out);
    prefs.end();
}

// Ajouter un appareil
void add(const String& mac, const String& name) {
    prefs.begin("bluetooth", true);
    String raw = prefs.getString(KEY_BT_LIST, "[]");
    prefs.end();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, raw);
    JsonArray arr = doc.as<JsonArray>();

    // Éviter les doublons
    for (JsonObject dev : arr) {
        if (dev["mac"] == mac) return;
    }

    if (arr.size() >= MAX_DEVICES) {
        arr.remove(0); // Supprimer le plus ancien
    }

    JsonObject new_dev = arr.createNestedObject();
    new_dev["mac"] = mac;
    new_dev["name"] = name;

    save(arr);
}

// Effacer toute l'historique
void clear() {
    prefs.begin("bluetooth", false);
    prefs.remove(KEY_BT_LIST);
    prefs.end();
    Serial.println("[BT_HIST] Historique effacé !");
    uart_manager::send_formatted("BT_ALL_REMOVED","" );
}

// Envoyer toute la liste via UART
void send_list_over_uart() {
    prefs.begin("bluetooth", true);
    String raw = prefs.getString(KEY_BT_LIST, "[]");
    prefs.end();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, raw);
    JsonArray arr = doc.as<JsonArray>();

    for (JsonObject dev : arr) {
        String name = dev["name"] | "Unknown";
        String mac = dev["mac"] | "00:00:00:00:00:00";
        
        // On envoie chaque device comme {BT_SAVED:<mac>|<name>]}
        uart_manager::send_formatted("BT_SAVED", mac + "|" + name );
   }
   
}

void remove(const String& mac) {
    prefs.begin("bluetooth", true);
    String raw = prefs.getString(KEY_BT_LIST, "[]");
    prefs.end();

    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, raw);
    if (err) {
        Serial.println("[BT_HIST] Erreur JSON lors de la suppression");
        return;
    }

    JsonArray arr = doc.as<JsonArray>();
    bool found = false;

    for (size_t i = 0; i < arr.size(); i++) {
        if (arr[i]["mac"] == mac) {
            arr.remove(i);
            found = true;
            break;
        }
    }

    if (found) {
        save(arr);
        Serial.println("[BT_HIST] Appareil supprimé : " + mac);
        uart_manager::send_formatted("BT_REMOVED", mac );
    } else {
        Serial.println("[BT_HIST] Appareil non trouvé : " + mac);
        uart_manager::send_formatted("BT_REMOVED_KO", mac );
    }
}


} // namespace
