#pragma once
#include <Arduino.h>

namespace bluetooth_manager {
void init();
void loop();

void scan_devices();
void connect_device(const String& mac_address);

void handle_uart_command(const String& type, const String& data);
}
