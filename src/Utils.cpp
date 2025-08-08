// Header
#include "Utils.h"

// Stl headers
#include <iomanip>
#include <cmath>


namespace exchange {

    // Custom fuzzy compare function
    bool fuzzyCompare(double a, double b, double tolerance) {
        return std::fabs(a - b) <= tolerance;
    }

    double roundDouble(double value, unsigned short decimals) {
        double val = value;
        // if val is less than 1e-'decimals' then remove the minus sign
        if (value < 0 && value > -1.0 / std::pow(10, decimals)) { 
            val = std::abs(value);
        }
        // Round the value
        double factor = std::pow(10, decimals);
        return std::round(val * factor) / factor;
    }

    std::string toString(std::chrono::system_clock::time_point date, std::string format) {
        std::time_t date_time = std::chrono::system_clock::to_time_t(date);
        std::tm tm;
        localtime_s(&tm, &date_time);
        char dateStr[256];
        strftime(dateStr, sizeof(dateStr), format.c_str(), &tm);
        return std::string(dateStr);
    }

} // namespace exchange
