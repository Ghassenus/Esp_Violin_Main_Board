#include "bt_pairing_manager.h"
#include "core/uart/uart_manager.h"
#include "core/bluetooth/bluetooth_history.h"

#include <esp_bt_main.h>
#include <esp_bt_device.h>
#include <esp_bt_defs.h>
#include <esp_gap_bt_api.h>
#include <esp_bt.h>

#include <Arduino.h>

static void on_gap_event(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch (event) {
        case ESP_BT_GAP_PIN_REQ_EVT: {
            esp_bt_pin_code_t pin_code;
            memcpy(pin_code, "1234", 4);
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
            Serial.println("[BT_PAIR] PIN 1234 envoyée");
            break;
        }

        case ESP_BT_GAP_AUTH_CMPL_EVT:
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                Serial.printf("[BT_PAIR] Appairage OK avec %s\n", param->auth_cmpl.device_name);
            } else {
                Serial.printf("[BT_PAIR] Appairage échoué. Statut: %d\n", param->auth_cmpl.stat);
            }
            break;

        case ESP_BT_GAP_CFM_REQ_EVT:
            esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
            Serial.println("[BT_PAIR] Confirmation SSP automatique envoyée");
            break;

        case ESP_BT_GAP_DISC_RES_EVT: {
            char bda_str[18];
            snprintf(bda_str, sizeof(bda_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                param->disc_res.bda[0], param->disc_res.bda[1], param->disc_res.bda[2],
                param->disc_res.bda[3], param->disc_res.bda[4], param->disc_res.bda[5]);

            String name = "";
            if (param->disc_res.num_prop > 0) {
                for (int i = 0; i < param->disc_res.num_prop; ++i) {
                    if (param->disc_res.prop[i].type == ESP_BT_GAP_DEV_PROP_EIR) {
                        uint8_t* eir = (uint8_t*)param->disc_res.prop[i].val;
                        uint8_t len = 0;
                        uint8_t* name_data = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &len);
                        if (name_data && len > 0) {
                            name = String((char*)name_data).substring(0, len);
                        }
                    }
                }
            }

            if (name == "") name = "(Inconnu)";

            Serial.printf("[BT_PAIR] Trouvé : %s [%s]\n", name.c_str(), bda_str);
            bluetooth_history::add(bda_str, name);
            uart_manager::send_formatted("SCAN_RESULT", String(bda_str) + "|" + name);
            break;
        }

        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
            if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
                Serial.println("[BT_PAIR] Scan terminé");
                uart_manager::send_formatted("SCAN_END", "");
            }
            break;

        default:
            Serial.printf("[BT_PAIR] Événement GAP ignoré: %d\n", event);
            break;
    }
}

void bt_pairing_manager::init() {
    Serial.println("[BT_PAIR] Initialisation BT + appairage PIN...");

    esp_bt_dev_set_device_name("Violin_Setup");
    esp_bt_gap_register_callback(on_gap_event);

    esp_bt_pin_code_t pin_code = "1234";
    esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, 4, pin_code);
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    Serial.println("[BT_PAIR] Bluetooth prêt, visible avec PIN 1234");
}

void bt_pairing_manager::start_scan() {
    Serial.println("[BT_PAIR] Démarrage du scan...");
    esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);  // 10 secondes
}

void bt_pairing_manager::stop_scan() {
    esp_bt_gap_cancel_discovery();
    Serial.println("[BT_PAIR] Scan Bluetooth arrêté manuellement");
}
