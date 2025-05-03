#include "bluetooth_manager.h"
#include "core/uart/uart_manager.h"
#include "core/bluetooth/bluetooth_history.h"
#include "core/bluetooth/bt_pairing_manager.h"
#include "logger.h"

#include <BluetoothA2DPSource.h>
#include <math.h>

static BluetoothA2DPSource a2dp_source;

// Paramètres audio
static bool is_playing = false;
static float time_pos = 0.0;
static float amplitude = 10000.0;
static float freq = 261.63; // Do
static float sample_rate = 44100.0;
static float phase = 0.0;

int32_t audio_callback(Frame *frame, int32_t frame_count) {
    if (!is_playing) {
        memset(frame, 0, sizeof(Frame) * frame_count);
        return frame_count;
    }

    float delta_time = 1.0 / sample_rate;
    float pi2 = 2 * PI;

    for (int i = 0; i < frame_count; ++i) {
        float angle = pi2 * freq * time_pos + phase;
        int16_t sample = amplitude * sin(angle);
        frame[i].channel1 = sample;
        frame[i].channel2 = sample;
        time_pos += delta_time;
    }

    delay(1); // Watchdog friendly
    return frame_count;
}

namespace bluetooth_manager {

void init() {
    log_info("[BT] Initialisation A2DP Source...");

    a2dp_source.set_local_name("Violin_Setup");

    a2dp_source.set_on_connection_state_changed([](esp_a2d_connection_state_t state, void*) {
        if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
            log_info("[BT] Connexion A2DP établie !");
            uart_manager::send_formatted("CONNECT_OK", "Périphérique connecté");
        } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
            log_warn("[BT] Déconnecté A2DP");
            uart_manager::send_formatted("DISCONNECTED", "Connexion perdue");
        }
    });

    a2dp_source.set_data_callback_in_frames(audio_callback);
    a2dp_source.set_volume(60);  // Volume initial
    a2dp_source.start();

    log_info("[BT] A2DP Source démarré");
    bluetooth_history::load();
}

void loop() {
    // Rien ici pour l’instant
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

    a2dp_source.disconnect(); // Important en cas de reconnexion
    if (!a2dp_source.connect_to(mac)) {
        uart_manager::send_formatted("ERROR", "Connexion A2DP échouée !");
    } else {
        log_info("[BT] Tentative de connexion A2DP envoyée");
    }
}

void play() {
    if (!is_playing) {
        is_playing = true;
        log_info("[BT] Lecture audio démarrée");
    }
}

void stop() {
    if (is_playing) {
        is_playing = false;
        log_info("[BT] Lecture audio arrêtée");
    }
}

void forward() {
    time_pos += 10.0;
    log_info("[BT] Audio +10s");
}

void backward() {
    time_pos = max(0.0f, time_pos - 10.0f);
    log_info("[BT] Audio -10s");
}

void volume_up() {
    int v = a2dp_source.get_volume();
    v = min(127, v + 10);
    a2dp_source.set_volume(v);
    LOGI("[BT] Volume augmenté à %d", v);
}

void volume_down() {
    int v = a2dp_source.get_volume();
    v = max(0, v - 10);
    a2dp_source.set_volume(v);
    LOGI("[BT] Volume diminué à %d", v);
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
    } else if (type == "BT_PLAY") {
        play();
    } else if (type == "BT_STOP") {
        stop();
    } else if (type == "BT_FORWARD") {
        forward();
    } else if (type == "BT_BACKWARD") {
        backward();
    } else if (type == "BT_VOL_UP") {
        volume_up();
    } else if (type == "BT_VOL_DOWN") {
        volume_down();
    } else {
        uart_manager::send_formatted("ERROR", "Commande inconnue: " + type);
    }
}

} // namespace
