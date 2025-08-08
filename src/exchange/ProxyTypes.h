#ifndef EXCHANGE_PROXYTYPES_H
#define EXCHANGE_PROXYTYPES_H


// Stl  headers
#include <string>

// Qt headers
#include <QObject>

// exchange headers
#include "exchange/CommonTypes.h"
#include "exchange/ProxyStatistics.h"
#include "exchange/TimerThread.h"


namespace exchange {

    // enum for proxy types
    enum class ProxyType {
        SystemStatus,
        TickersInformation,
        OrderBook,
        RecentTrades,
        Unknown
    };

    std::string getProxyTypeName(ProxyType type);


    // The Proxy base class
    class Worker : public QObject {

        Q_OBJECT

    public:
        Worker(ProxyType type, Service service, unsigned int period)
            : QObject()
            , type_(type)
            , service_(service)
            , period_(period)
            , end_()
            , status_(ProxyStatus::Unknown)
            , stats_() {
        }

        ~Worker() {}

    public:
        inline ProxyType getProxyType() const { return type_; }
        inline Service getServiceType() const { return service_; }
        inline QString getServiceName() const { return exchange::getServiceName(service_).c_str(); }
        
        inline int getPeriod() const { return period_; }
        inline ProxyStatus getStatus() const { return status_; }
        inline void setStatus(ProxyStatus status) {
            status_ = status;
            emit statusUpdated(service_, type_, status_);
        }

        inline void init() {
            end_ = std::chrono::system_clock::now();
            setStatus(ProxyStatus::Running);
            exec();
        }

        virtual QString getLabel() const {
            return getServiceName() + "." + QString::fromStdString(getProxyTypeName(type_));
        }

    signals:
        void statisticsUpdated(Service service, ProxyType type, WorkStats workStats);
        void statusUpdated(Service service, ProxyType type, ProxyStatus status);

        void slowdown(int msec);
        void pause();
        void resume();

        void info(const QString& text);
        void warning(const QString& text);
        void error(const QString& text);
        void trace(const QString& text);

    public slots:
        void exec() {
            // Get the start time of the exec method
            std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
            // Execute the controller main method /////////////////////////////////////////////////////////////
            Response response = exec_();
            if(response == Response::Stop) {
                emit slowdown(60000);
            } else if(response == Response::Banned) {
                emit pause();
            }
            ///////////////////////////////////////////////////////////////////////////////////////////////////
            // Get the end time of the exec method
            std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
            // Calculate the time spent in the exec method in milliseconds
            std::chrono::duration<double, std::milli> elapsed = end - start;
            // Calculate the duration between two exec calls
            std::chrono::duration<double, std::milli> period;
            if (end_ != std::chrono::system_clock::time_point()) {
                period = end - end_;
                std::chrono::duration<double, std::milli> waitingTime = period - elapsed;
                // Calculate the frequency of the controller
                double frequency = 1000.0 / period.count();
                unsigned int key = static_cast<unsigned int>(frequency);
                stats_.addExecution(key, elapsed.count(), waitingTime.count());
                WorkStats workStats = stats_.getWorkStats();
                workStats.response = response;
                emit statisticsUpdated(service_, type_, workStats);
            }
            end_ = end;
        }

        void pong() {
            emit info(getLabel() + ": OK");
        }

    private:
        virtual Response exec_() = 0;

    private:
        ProxyType type_;
        Service service_;
        int period_;
        std::chrono::system_clock::time_point end_;
        ProxyStatus status_;
        ProxyStatistics stats_;
    };



    // The MarketWorker class
    class MarketWorker : public Worker {

        Q_OBJECT

    public:
        MarketWorker(ProxyType type, Service service, Market market, unsigned int period = 1000) // 1s
            : Worker(type, service, period)
            , market_(market) {
            connect(this, &Worker::statusUpdated, [&](Service service, ProxyType type, ProxyStatus status) {
                emit workerStatusUpdated(service, type, market_, status);
            });
            connect(this, &Worker::statisticsUpdated, [&](Service service, ProxyType type, WorkStats workStats) {
                emit workerStatisticsUpdated(service, type, market_, workStats);
            });
        }

        ~MarketWorker() {}

    public:
        QString getLabel() const {
            return Worker::getLabel() + "[" + QString::fromStdString(getMarketName(market_)) + "]";
        }

    signals:
        void workerStatusUpdated(Service service, ProxyType type, Market market, ProxyStatus status);
        void workerStatisticsUpdated(Service service, ProxyType type, Market market, WorkStats workStats);

    private:
        virtual Response exec_() = 0;

    protected:
        Market market_;
    };

} // namespace exchange

#endif // EXCHANGE_PROXYTYPES_H
