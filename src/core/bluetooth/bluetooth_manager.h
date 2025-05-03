#pragma once

#include <Arduino.h>

namespace bluetooth_manager {
    void init();
    void loop();

    void scan_devices();
    void stop_scan_devices();
    void connect_device(const String& mac_address);

    // Commandes audio
    void play();
    void stop();
    void forward();
    void backward();
    void volume_up();
    void volume_down();

    void handle_uart_command(const String& type, const String& data);
}
