#pragma once
#include <string>

// 1970-01-01 se kitne din guzre (calendar ka hisaab, timezone ka jhanjhat nahi).
// Leap year ka hisaab bhi isi mein hai.
inline long long daysFromCivil(int y, int m, int d) {
    y -= (m <= 2) ? 1 : 0;
    long long era = (y >= 0 ? y : y - 399) / 400;
    long long yoe = y - era * 400;
    long long mp = (m > 2) ? (m - 3) : (m + 9);
    long long doy = (153 * mp + 2) / 5 + d - 1;
    long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

// "2026-10-05" aur "14:32:10" ko 1970 se seconds mein badalta hai.
// Maan ke chalta hai ke date aur time ka format pehle hi check ho chuka hai.
inline long long toEpochSeconds(const std::string& date, const std::string& time) {
    int year  = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day   = std::stoi(date.substr(8, 2));
    int hour   = std::stoi(time.substr(0, 2));
    int minute = std::stoi(time.substr(3, 2));
    int second = std::stoi(time.substr(6, 2));
    return daysFromCivil(year, month, day) * 86400LL
         + hour * 3600LL + minute * 60LL + second;
}