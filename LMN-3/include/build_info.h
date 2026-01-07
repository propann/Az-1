#pragma once

#include <Arduino.h>

#ifndef BUILD_TAG
#define BUILD_TAG "AZ-1-LMN3"
#endif

#ifndef VARIANT
#define VARIANT "SERIAL_ONLY"
#endif

#ifndef GIT_SHA
#define GIT_SHA "UNKNOWN"
#endif

#ifndef BUILD_DATE
#define BUILD_DATE __DATE__
#endif

#ifndef BUILD_TIME
#define BUILD_TIME __TIME__
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

inline String normalize_define(const char *val) {
    String s(val);
    if (s.length() >= 2 && s[0] == '"' && s[s.length() - 1] == '"') {
        s.remove(0, 1);
        s.remove(s.length() - 1, 1);
    }
    return s;
}

inline String build_tag() {
    String banner = normalize_define(STR(BUILD_TAG));
    banner += " / ";
    banner += normalize_define(STR(VARIANT));
    banner += " / ";
    banner += normalize_define(STR(GIT_SHA));
    banner += " / ";
    banner += normalize_define(STR(BUILD_DATE));
    banner += " ";
    banner += normalize_define(STR(BUILD_TIME));
    return banner;
}
