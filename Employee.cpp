#include "Employee.h"

bool Employee::canWork(int day, int shift) const {
    if (vacations.first <= day && day <= vacations.second) {return false;}
    bool has_day_preference = false;
    for (const auto& cons : constraints) {
        if (cons.first == day) {
            if (cons.second == -1) { return false;}
            has_day_preference = true;
            if (cons.second == shift) {return true; }
        }
    }
    if (has_day_preference) {return false;}
    return true;
}
