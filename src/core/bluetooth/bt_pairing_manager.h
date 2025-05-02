#pragma once

namespace bt_pairing_manager {
    void init();
    void start_scan();  // <- scan déclenché depuis bluetooth_manager
    void stop_scan(); // arre le scan
}
