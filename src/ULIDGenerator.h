#ifndef ULIDGENERATOR_H
#define ULIDGENERATOR_H

// Stl headers
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>

// headers

// exchange headers


namespace exchange {

/*
ULID (Universally Unique Lexicographically Sortable Identifier)
ULIDs are 128-bit identifiers that are lexicographically sortable. They consist of a 48-bit timestamp and an 80-bit random component. This makes them suitable for use in databases where sorting by creation time is important.
ULIDs are similar to UUIDs, but they are more efficient for databases because they are shorter and can be sorted by creation time.
These method ensure that IDs are unique and can be generated efficiently in distributed systems without collisions.
*/

    class ULIDGenerator {
    public:
        static std::string next() {
            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

            std::stringstream ss;
            ss << std::hex << std::setw(12) << std::setfill('0') << timestamp;

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<uint64_t> dis(0, std::numeric_limits<uint64_t>::max());

            for (int i = 0; i < 2; ++i) {
                ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
            }

            return ss.str();
        }
    };


} // namespace exchange

#endif // ULIDGENERATOR_H
