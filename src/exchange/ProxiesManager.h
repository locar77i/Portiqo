#ifndef EXCHANGE_PROXIESMANAGER_H
#define EXCHANGE_PROXIESMANAGER_H


// Stl  headers
#include <string>
#include <chrono>
#include <vector>
#include <map>
#include <iostream>

// Qt headers
#include <QObject>
#include <QEventLoop>
#include <QtCore/QTimer>
#include <QtCore/QThread>

// exchange headers
#include "exchange/CommonTypes.h"
#include "exchange/ProxyTypes.h"
#include "exchange/ProxyStatistics.h"
#include "exchange/TimerThread.h"

// hmi headers
#include "hmi/MainWindow.h"

// headers
#include "Manager.h"


namespace exchange {



    // The SystemStatusWorker class
    class SystemStatusWorker : public Worker {

        Q_OBJECT

    public:
        SystemStatusWorker(Service service, unsigned int period = 1000) // 1s
            : Worker(ProxyType::SystemStatus, service, period) {
        } 

        ~SystemStatusWorker() {}

    signals:
        void systemStatusUpdated(SystemStatus systemStatus);

    private:
        Response exec_() {
            Response response;
            SystemStatus systemStatus;
            try {
                switch(getServiceType()) {
                    case Service::Kraken:
                        response = Kraken::getSystemStatus(systemStatus);
                        break;
                    case Service::Binance:
                        response = Binance::getSystemStatus(systemStatus);
                        break;
                    case Service::Coinbase:
                        response = Coinbase::getSystemStatus(systemStatus);
                        break;
                    default:
                        emit warning(getServiceName() + ": Service not implemented");
                        return Response::Warning;
                }
            }
            catch (const std::exception& e) {
                emit error(getLabel() + ": ERROR - " + QString(e.what()));   
            }
            emit systemStatusUpdated(systemStatus);
            return response;
        }

    private:
    };



    // The TickersInformationWorker class
    class TickersInformationWorker : public Worker {

        Q_OBJECT

    public:
        TickersInformationWorker(Service service, unsigned int period = 1000) // 1s
            : Worker(ProxyType::TickersInformation, service, period) {
        } 

        ~TickersInformationWorker() {}

    signals:
        void tickersInformationUpdated(Service service, TickersInformation tickersInformation);

    private:
        Response exec_() {
            Response response;
            TickersInformation tickersInformation;
            try {
                switch(getServiceType()) {
                    case Service::Kraken:
                        response = Kraken::getTickersInformation(tickersInformation, KrakenPairs);
                        break;
                    case Service::Binance:
                        response = Binance::getTickersInformation(tickersInformation, BinancePairs);
                        break;
                    case Service::Coinbase:
                        for(const auto& pair : CoinbasePairs) {
                            Ticker ticker;
                            response = Coinbase::getTicker(ticker, pair);
                            tickersInformation.add(pair.first, TickerInformation(ticker));
                        }
                        break;
                    default:
                        emit warning(getServiceName() + ": Service not implemented");
                        return Response::Warning;
                }
            }
            catch (const std::exception& e) {
                emit error(getLabel() + ": ERROR - " + QString(e.what()));   
            }
            emit tickersInformationUpdated(getServiceType(), tickersInformation);
            return response;
        }

    private:
    };



    // The OrderBookWorker class
    class OrderBookWorker : public MarketWorker {

        Q_OBJECT

    public:
        OrderBookWorker(Service service, Market market, unsigned int period = 1000) // 1s
            : MarketWorker(ProxyType::OrderBook, service, market, period) {}

        ~OrderBookWorker() {}

    signals:
        void orderBookSummaryUpdated(Service service, Market market, OrderBookSummary orderBookSummary);

    private:
        Response exec_() {
            Response response;
            try {
                OrderBook orderBook(market_);
                switch(getServiceType()) {
                    case Service::Kraken:
                        response = Kraken::getOrderBook(orderBook, 500);
                        break;
                    case Service::Binance:
                        response = Binance::getOrderBook(orderBook, 500);
                        break;
                    case Service::Coinbase:
                        response = Coinbase::getOrderBook(orderBook, 500);
                        break;
                    default:
                        emit warning(getServiceName() + " " + QString::fromStdString(getMarketName(market_)) + ": Service not implemented");
                        return Response::Warning;
                }
                emit orderBookSummaryUpdated(getServiceType(), market_, orderBook.getSummary());
            }
            catch (const std::exception& e) {
                emit error(getLabel() + ": ERROR - " + QString(e.what()));   
            }
            return response;
        }
    };



    // The RecentTradesWorker class
    class RecentTradesWorker : public MarketWorker {

        Q_OBJECT

    public:
        RecentTradesWorker(Service service, Market market, unsigned int period = 1000) // 1s
            : MarketWorker(ProxyType::RecentTrades, service, market, period)
            , recentTrades_(market) {}

        ~RecentTradesWorker() {}

    signals:
        void recentTradesSummaryUpdated(Service service, Market market, RecentTradesSummary recentTradesSummary);

    private:
        Response exec_() {
            Response response;
            try {
                recentTrades_.start();
                switch(getServiceType()) {
                    case Service::Kraken:
                        response = Kraken::getRecentTrades(recentTrades_, 500);
                        break;
                    case Service::Binance:
                        response = Binance::getRecentTrades(recentTrades_, 500);
                        break;
                    case Service::Coinbase:
                        response = Coinbase::getRecentTrades(recentTrades_, 500);
                        break;
                    default:
                        emit warning(getServiceName() + " " + QString::fromStdString(getMarketName(market_)) + ": Service not implemented");
                        return Response::Warning;
                }
                auto summary = recentTrades_.getSummary();
                if(!summary.isEmpty()) {
                    emit recentTradesSummaryUpdated(getServiceType(), market_, summary);
                }
            }
            catch (const std::exception& e) {
                emit error(getLabel() + ": ERROR - " + QString(e.what()));   
            }
            return response;
        }

        private:
            RecentTrades recentTrades_;
    };


    struct Proxy {
        std::shared_ptr<Worker> worker;
        TimerThread thread;

        Proxy(std::shared_ptr<Worker> worker): worker(worker), thread(worker->getPeriod()) {}

        void start() {
            worker->moveToThread(&thread);
            std::cout << "Launching proxy: " << worker->getLabel().toStdString() << " ..." << std::endl;
            worker->init();
            thread.start();
        }
    };

    class ServiceProxies {

    public:
        ServiceProxies()
            : proxies_() {
        }

        ~ServiceProxies() {}

    public:
        bool addProxy(Market market, std::shared_ptr<Proxy> proxy) {
            auto it = proxies_.find(market);
            if(it == proxies_.end()) {
                proxies_[market] = proxy;
                return true;
            }
            return false;
        }

        std::shared_ptr<Proxy> getProxy(Market market) {
            auto it = proxies_.find(market);
            if(it != proxies_.end()) {
                return it->second;
            }
            return nullptr;
        }

        std::shared_ptr<Proxy> getAndRemoveProxy(Market market) {
            std::shared_ptr<Proxy> proxy = nullptr;
            auto it = proxies_.find(market);
            if(it != proxies_.end()) {
                proxy = it->second;
                proxies_.erase(it);
            }
            return proxy;
        }

        void requestExit() {
            for(auto& pair : proxies_) {
                std::cout << "Waiting for proxy " << pair.second->worker->getLabel().toStdString() << " to finish..." << std::endl;
                pair.second->thread.requestExit(true);
                pair.second->thread.wait();
            }
        }

    private:
        std::map<Market, std::shared_ptr<Proxy>> proxies_;
    };



    class ProxiesManager : public QObject {

        Q_OBJECT

    public:
        ProxiesManager(hmi::MainWindow& window, Manager& manager)
            : QObject()
            , window_(window)
            , manager_(manager)
            , orderBookProxies_()
            , recentTradesProxies_() {
        }

        ~ProxiesManager() {}

    public:
        void requestExit() {
            // iterate over all services
            for(auto& servicePair : orderBookProxies_) {
                servicePair.second.requestExit();
            }
            for(auto& servicePair : recentTradesProxies_) {
                servicePair.second.requestExit();
            }
        }

    signals:
        void proxyCreated(Service service, ProxyType type, Market market, unsigned int period);
        void proxyDestroyed(Service service, ProxyType type, Market market);

        void info(const QString& text);
        void warning(const QString& text);
        void error(const QString& text);
        void trace(const QString& text);

    public slots:
        void createProxy(Service service, ProxyType type, Market market, unsigned int period = 1000) {
            std::cout << "Creating proxy: " << getServiceName(service) << " " << getProxyTypeName(type) << " " << getMarketName(market) << " ..." << std::endl;
            std::shared_ptr<Proxy> proxy;
            switch(type) {
                case ProxyType::OrderBook:
                    proxy = createOrderBookProxy_(service, market, period);
                    break;
                case ProxyType::RecentTrades:
                    proxy = createRecentTradesProxy_(service, market, period);
                    break;
                default:
                    emit warning(tr("unknown-proxy-type")  // "Unknown proxy type: %1"
                        .arg(getProxyTypeName(type).c_str())
                    );
                    return;
            }
            if(proxy) {
                proxy->start();
            }
        }

        void slowdownProxy(Service service, ProxyType type, Market market, unsigned int msec) {
            std::shared_ptr<Proxy> proxy = getProxy_(service, type, market);
            if(proxy) {
                proxy->worker->emit slowdown(msec);
            }
        }

        void pauseProxy(Service service, ProxyType type, Market market) {
            std::shared_ptr<Proxy> proxy = getProxy_(service, type, market);
            if(proxy) {
                proxy->worker->emit pause();
            }
        }

        void resumeProxy(Service service, ProxyType type, Market market) {
            std::shared_ptr<Proxy> proxy = getProxy_(service, type, market);
            if(proxy) {
                proxy->worker->emit resume();
            }
        }

        void destroyProxy(Service service, ProxyType type, Market market) {
            std::shared_ptr<Proxy> proxy = getAndRemoveProxy_(service, type, market);
            if(proxy) {
                proxy->worker->blockSignals(true);
                proxy->thread.requestExit(true);
                proxy->thread.wait();
                emit proxyDestroyed(service, type, market);
            }
        }

    private:
        std::shared_ptr<Proxy> createOrderBookProxy_(Service service, Market market, unsigned int period) {
            auto worker = std::make_shared<OrderBookWorker>(service, market, period);
            auto proxy = std::make_shared<Proxy>(worker);
            connectProxy_(worker.get(), &(proxy->thread));
            // worker -> manager
            QObject::connect(worker.get(), &OrderBookWorker::orderBookSummaryUpdated, &manager_, &Manager::updateOrderBookSummary, Qt::QueuedConnection);
            // worker -> window
            QObject::connect(worker.get(), &OrderBookWorker::workerStatusUpdated, &window_, &hmi::MainWindow::updateProxyStatus, Qt::QueuedConnection);
            QObject::connect(worker.get(), &OrderBookWorker::workerStatisticsUpdated, &window_, &hmi::MainWindow::updateProxyStatistics, Qt::QueuedConnection);
            // Add the proxy to the list of proxies
            return addProxy_(proxy, orderBookProxies_, service, market)? proxy : nullptr;
        }

        std::shared_ptr<Proxy> createRecentTradesProxy_(Service service, Market market, unsigned int period) {
            auto worker = std::make_shared<RecentTradesWorker>(service, market, period);
            auto proxy = std::make_shared<Proxy>(worker);
            connectProxy_(worker.get(), &(proxy->thread));
            // worker -> manager
            QObject::connect(worker.get(), &RecentTradesWorker::recentTradesSummaryUpdated, &manager_, &Manager::updateRecentTradesSummary, Qt::QueuedConnection);
            // worker -> window
            QObject::connect(worker.get(), &RecentTradesWorker::workerStatusUpdated, &window_, &hmi::MainWindow::updateProxyStatus, Qt::QueuedConnection);
            QObject::connect(worker.get(), &RecentTradesWorker::workerStatisticsUpdated, &window_, &hmi::MainWindow::updateProxyStatistics, Qt::QueuedConnection);
            // Add the proxy to the list of proxies
            return addProxy_(proxy, recentTradesProxies_, service, market)? proxy : nullptr;
        }

        void connectProxy_(Worker* worker, TimerThread* thread) {
            // window -> worker
            QObject::connect(&window_, &hmi::MainWindow::ping, worker, &Worker::pong, Qt::QueuedConnection);
            // worker -> window
            QObject::connect(worker, &Worker::warning, &window_, &hmi::MainWindow::showWarning, Qt::QueuedConnection);
            QObject::connect(worker, &Worker::info, &window_, &hmi::MainWindow::showInfo, Qt::QueuedConnection);
            QObject::connect(worker, &Worker::error, &window_, &hmi::MainWindow::showError, Qt::QueuedConnection);
            QObject::connect(worker, &Worker::trace, &window_, &hmi::MainWindow::showTrace, Qt::QueuedConnection);
            // thread -> worker
            QObject::connect(thread->timer(), &QTimer::timeout, worker, &OrderBookWorker::exec);
            // worker -> thread
            QObject::connect(worker, &OrderBookWorker::slowdown, thread->timer(), [=](int msec) {
                thread->timer()->setInterval(msec);
                if(msec == worker->getPeriod()) 
                    worker->setStatus(ProxyStatus::Running);
                else if(msec > worker->getPeriod()) {
                    worker->setStatus(ProxyStatus::SlowedDown);
                } else {
                    worker->setStatus(ProxyStatus::SpeededUp);
                }
            });
            QObject::connect(worker, &OrderBookWorker::pause, thread->timer(), [=]() {
                thread->timer()->stop();
                worker->setStatus(ProxyStatus::Paused);
            });
            QObject::connect(worker, &OrderBookWorker::resume, thread->timer(), [=]() {
                thread->timer()->start(worker->getPeriod());
                worker->setStatus(ProxyStatus::Running);
            });
        }

        bool addProxy_(std::shared_ptr<Proxy> proxy, std::map<Service, ServiceProxies>& proxies, Service service, Market market) {
            auto service_it = proxies.find(service);
            if(service_it == proxies.end()) {
                ServiceProxies serviceProxies;
                serviceProxies.addProxy(market, proxy);
                proxies[service] = serviceProxies;
                emit proxyCreated(service, proxy->worker->getProxyType(), market, proxy->worker->getPeriod());
                return true;
            }
            if(service_it->second.addProxy(market, proxy)) {
                emit proxyCreated(service, proxy->worker->getProxyType(), market, proxy->worker->getPeriod());
                return true;
            }
            emit warning("Proxy " + proxy->worker->getLabel() + " already exists");
            return false;
        }

        std::shared_ptr<Proxy> getProxy_(Service service, ProxyType type, Market market) {
            std::shared_ptr<Proxy> proxy = nullptr;
            switch(type) {
                case ProxyType::OrderBook:
                    proxy = getProxy_(orderBookProxies_, service, market);
                    break;
                case ProxyType::RecentTrades:
                    proxy = getProxy_(recentTradesProxies_, service, market);
                    break;
                default:
                    emit warning(tr("unknown-proxy-type")  // "Unknown proxy type: %1"
                        .arg(getProxyTypeName(type).c_str())
                    );
            }
            if(!proxy) {
                emitProxyNotFound_(service, type, market);
            }
            return proxy;
        }
        std::shared_ptr<Proxy> getProxy_(std::map<Service, ServiceProxies>& proxies, Service service, Market market) {
            auto service_it = proxies.find(service);
            if(service_it != proxies.end()) {
                return service_it->second.getProxy(market);
            }
            return nullptr;
        }

        std::shared_ptr<Proxy> getAndRemoveProxy_(Service service, ProxyType type, Market market) {
            std::shared_ptr<Proxy> proxy = nullptr;
            switch(type) {
                case ProxyType::OrderBook:
                    proxy = getAndRemoveProxy_(orderBookProxies_, service, market);
                    break;
                case ProxyType::RecentTrades:
                    proxy = getAndRemoveProxy_(recentTradesProxies_, service, market);
                    break;
                default:
                    emit warning(tr("unknown-proxy-type")  // "Unknown proxy type: %1"
                        .arg(getProxyTypeName(type).c_str())
                    );
            }
            if(!proxy) {
                emitProxyNotFound_(service, type, market);
            }
            return proxy;
        }
        std::shared_ptr<Proxy> getAndRemoveProxy_(std::map<Service, ServiceProxies>& proxies, Service service, Market market) {
            std::shared_ptr<Proxy> proxy = nullptr;
            auto service_it = proxies.find(service);
            if(service_it != proxies.end()) {
                proxy = service_it->second.getAndRemoveProxy(market);
            }
            return proxy;
        }

        void emitProxyNotFound_(Service service, ProxyType type, Market market) {
            emit warning("Proxy " + QString::fromStdString(getServiceName(service)) + "." + QString::fromStdString(getProxyTypeName(type)) + "[" + QString::fromStdString(getMarketName(market)) + "] does not exist");
        }

    private:
        hmi::MainWindow& window_;
        Manager& manager_;
        std::map<Service, ServiceProxies> orderBookProxies_;
        std::map<Service, ServiceProxies> recentTradesProxies_;
    };

} // namespace exchange

#endif // EXCHANGE_PROXIESMANAGER_H
