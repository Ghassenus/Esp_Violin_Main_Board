#pragma once
#include <Arduino.h>

// === Fonctions principales ===
inline void log_info(const String& msg) {
    Serial.println("[INFO] " + msg);
    
}

inline void log_warn(const String& msg) {
    Serial.println("[WARN] " + msg);
    
}

inline void log_error(const String& msg) {
    Serial.println("[ERROR] " + msg);
    
}

// === Version printf() style ===
inline void logf(const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    Serial.println(buf);
}

// === Macros ultra rapides ===
#define LOGI(...) logf("[INFO] " __VA_ARGS__)
#define LOGW(...) logf("[WARN] " __VA_ARGS__)
#define LOGE(...) logf("[ERROR] " __VA_ARGS__)
