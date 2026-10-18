#include <iostream>
#include <string>
#include "timeutil.h"

static int failures = 0;

void expectEqual(const std::string& name, long long got, long long want) {
    if (got != want) {
        std::cout << "FAIL: " << name << "  got " << got << "  want " << want << "\n";
        failures++;
    }
}

bool isLeap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int daysInMonth(int y, int m) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && isLeap(y)) return 29;
    return days[m - 1];
}

int main() {
    // 1) Jaani pehchaani values (Python se nikaali hui)
    expectEqual("1970-01-01 00:00:00", toEpochSeconds("1970-01-01", "00:00:00"), 0);
    expectEqual("2000-01-01 00:00:00", toEpochSeconds("2000-01-01", "00:00:00"), 946684800);
    expectEqual("2026-10-05 00:00:00", toEpochSeconds("2026-10-05", "00:00:00"), 1791158400);
    expectEqual("2026-10-05 14:32:10", toEpochSeconds("2026-10-05", "14:32:10"), 1791210730);
    expectEqual("2028-02-29 12:00:00", toEpochSeconds("2028-02-29", "12:00:00"), 1835438400);
    expectEqual("2100-03-01 00:00:00", toEpochSeconds("2100-03-01", "00:00:00"), 4107542400LL);

    // 2) Din, mahina, saal badalne par sirf 1 second ka farq
    expectEqual("day change",
        toEpochSeconds("2026-10-06", "00:00:00") - toEpochSeconds("2026-10-05", "23:59:59"), 1);
    expectEqual("month change",
        toEpochSeconds("2026-11-01", "00:00:00") - toEpochSeconds("2026-10-31", "23:59:59"), 1);
    expectEqual("year change",
        toEpochSeconds("2027-01-01", "00:00:00") - toEpochSeconds("2026-12-31", "23:59:59"), 1);

    // 3) Leap year: 2028 mein 29 Feb hai (2 din), 2027 mein nahi (1 din)
    expectEqual("leap year 2028",
        toEpochSeconds("2028-03-01", "12:00:00") - toEpochSeconds("2028-02-28", "12:00:00"), 172800);
    expectEqual("non-leap year 2027",
        toEpochSeconds("2027-03-01", "12:00:00") - toEpochSeconds("2027-02-28", "12:00:00"), 86400);
    expectEqual("century 2100 is not leap",
        toEpochSeconds("2100-03-01", "00:00:00") - toEpochSeconds("2100-02-28", "00:00:00"), 86400);
    expectEqual("year 2000 is leap",
        toEpochSeconds("2000-03-01", "00:00:00") - toEpochSeconds("2000-02-28", "00:00:00"), 172800);

    // 4) 1970 se 2100 tak har din: agla din theek 1 din (86400 s) aage hona chahiye
    long long checkedDays = 0;
    long long prev = daysFromCivil(1970, 1, 1);
    expectEqual("first day is 0", prev, 0);
    for (int y = 1970; y <= 2100; y++) {
        for (int m = 1; m <= 12; m++) {
            for (int d = 1; d <= daysInMonth(y, m); d++) {
                if (y == 1970 && m == 1 && d == 1) continue;
                long long cur = daysFromCivil(y, m, d);
                if (cur != prev + 1) {
                    std::cout << "FAIL: gap at " << y << "-" << m << "-" << d << "\n";
                    failures++;
                }
                prev = cur;
                checkedDays++;
            }
        }
    }
    std::cout << "Days checked one by one: " << checkedDays << "\n";
    std::cout << "Failures: " << failures << "\n";
    std::cout << (failures == 0 ? "TEST PASSED" : "TEST FAILED") << "\n";
    return failures == 0 ? 0 : 1;
}