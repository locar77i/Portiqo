#ifndef HMI_PROXYSTATISTICS_H
#define HMI_PROXYSTATISTICS_H


// Stl  headers
#include <string>
#include <chrono>
#include <map>


// exchange headers
#include "exchange/CommonTypes.h"


namespace exchange {

    struct WorkStats {
        double frequency;
        unsigned int maxElapsedTime;
        unsigned int count;
        unsigned long long cycles;
        Response response;

        WorkStats() : frequency(0), maxElapsedTime(0), count(0), cycles(0), response(Response::Unknown) {}
        WorkStats(double frequency, unsigned int maxElapsedTime, unsigned int count, unsigned long long cycles)
            : frequency(frequency), maxElapsedTime(maxElapsedTime), count(count), cycles(cycles), response(Response::Unknown) {}

        std::string description() const {
            std::ostringstream oss;
            oss << "Frequency: " << frequency << " Hz - Max elapsed time: " << maxElapsedTime << " ms - Count: " << count << " - Cycles: " << cycles;
            return oss.str();
        }
    };

    struct ExecutionEntry {
        unsigned long long cycles;
        unsigned int elapsed;
        unsigned int sleepTime;

        ExecutionEntry() : cycles(0), elapsed(0), sleepTime(0) {}
    };

    class ProxyStatistics {
    public:
        ProxyStatistics() : executions_(), maxElapsedTime_(0) {}

        void addExecution(unsigned int key, unsigned int elapsed, unsigned int sleepTime) {
            if(maxElapsedTime_ < elapsed) maxElapsedTime_ = elapsed;
            auto& entry = executions_[key];
            entry.cycles++;
            entry.elapsed += elapsed;
            entry.sleepTime += sleepTime;
        }

        unsigned int getCount() const {
            return static_cast<unsigned int>(executions_.size());
        }

        WorkStats getWorkStats() const {
            return WorkStats(getAverageFrequency(), getMaxElapsedTime(), getCount(), getCycles());
        }

        unsigned int getMaxElapsedTime() const {
            return maxElapsedTime_;
        }

        double getAverageFrequency() const {
            double frequency = 0.0;
            unsigned long long cycles = 0;
            unsigned long long elapsed = 0;
            unsigned long long sleepTime = 0;
            for (const auto& entry : executions_) {
                cycles += entry.second.cycles;
                elapsed += entry.second.elapsed;
                sleepTime += entry.second.sleepTime;
                double totalSeconds = (double)(elapsed + sleepTime) / 1000.0;
                double averagePeriod = (cycles!=0? (totalSeconds / (double)cycles) : 0.0);
                frequency = (averagePeriod!=0? (1.0 / averagePeriod) : 0.0);
            }
            return frequency;
        }

        unsigned long long getCycles() const {
            unsigned long long cycles = 0;
            for (const auto& entry : executions_) {
                cycles += entry.second.cycles;
            }
            return cycles;
        }

        long long getElapsed() const {
            long long elapsed = 0;
            for (const auto& entry : executions_) {
                elapsed += entry.second.elapsed;
            }
            return elapsed;
        }

        long long getSleepTime() const {
            long long sleepTime = 0;
            for (const auto& entry : executions_) {
                sleepTime += entry.second.sleepTime;
            }
            return sleepTime;
        }

        std::string description() const {
            std::ostringstream oss;
            oss << "Cycles: " << getCycles()
                << " - Elapsed: " << getElapsed()
                << " - Sleep time: " << getSleepTime()
                << " - Average frequency: " << getAverageFrequency();
            return oss.str();
        }

    private:
        std::map<unsigned int, ExecutionEntry> executions_;
        unsigned int maxElapsedTime_;
    };

} // namespace exchange


#endif // HMI_PROXYSTATISTICS_H
