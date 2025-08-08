#ifndef HMI_UTILS_H
#define HMI_UTILS_H


// Stl  headers
#include <string>
#include <chrono>


namespace exchange {

    // Custom fuzzy compare function
    bool fuzzyCompare(double a, double b, double tolerance);
    double roundDouble(double value, unsigned short decimals);

    std::string toString(std::chrono::system_clock::time_point date, std::string format = "%d/%m/%Y %H:%M:%S");

} // namespace exchange


#endif // HMI_UTILS_H
