#ifndef CONTROLLER_H
#define CONTROLLER_H

// Stl headers
#include <map>
#include <memory>
#include <string>
#include <thread>

// exchange headers
#include "exchange/Exchange.h"

// headers
#include "Types.h"

// exchange headers
#include "exchange/Portfolio.h"



namespace exchange {


class Controller {
public:
    Controller();

    void init();

public:
    void createPortfolio(const std::string& name, Ticket referenceCurrency, double capital, PortfolioData& data);
    void openPortfolio(const std::string& filename, PortfolioData& data);
    bool savePortfolio(const std::string& name, unsigned int& numOperations);
    void savePortfolioAs(const std::string& name, const std::string& filename, unsigned int& numOperations);
    bool closePortfolio(const std::string& name, bool force=false);

    void order(OperationType operationType, const std::string& name, const OrderData& order, PortfolioUpdate& feedback);
    void simulateOrder(OperationType operationType, const std::string& name, const DummyOrder& order, PortfolioUpdate& feedback);
    void transfer(OperationType operationType, const std::string& name, const TransferData& transfer, PortfolioUpdate& feedback);
    void simulateTransfer(OperationType operationType, const std::string& name, const DummyTransfer& transfer, PortfolioUpdate& feedback);

    void lockPortfolio(const std::string& name, bool locked);
    void removeLastPortfolioOperation(const std::string& name, PortfolioRemove& feedback);

    void setTickersInformation(const TickersInformation& tickersInformation) {
        tickersInformation_ = tickersInformation;
    }

    void addTicker(exchange::Service service, Market market, const Ticker& ticker);
    void addOrderBookSummary(exchange::Service service, Market market, const OrderBookSummary& orderBookSummary);
    void addRecentTradesSummary(exchange::Service service, Market market, const RecentTradesSummary& recentTradesSummary);

    void addMarketDataSummary(Service service, Market market, const Ticker& ticker);
    void addMarketDataSummary(Service service, Market market, const MarketDataSummary& marketDataSummary);
    void saveData(Service service, ProxyType type, Market market, bool flush=false);

public: // Signals getters
    void setTraceSignal(std::function<void(std::string msg)> signal) { traceSignal_ = signal; }
    void setInfoSignal(std::function<void(std::string msg)> signal) { infoSignal_ = signal; }
    void setWarningSignal(std::function<void(std::string msg)> signal) { warningSignal_ = signal; }
    void setErrorSignal(std::function<void(std::string msg)> signal) { errorSignal_ = signal; }

private: // Signals emitters
    void emitTraceSignal_(const std::string& msg) { if(traceSignal_) traceSignal_(msg); }
    void emitInfoSignal_(const std::string& msg) { if(infoSignal_) infoSignal_(msg); }
    void emitWarningSignal_(const std::string& msg) { if(warningSignal_) warningSignal_(msg); }
    void emitErrorSignal_(const std::string& msg) { if(errorSignal_) errorSignal_(msg); }

private:
    void add_(std::shared_ptr<Portfolio> portfolio, PortfolioData& data);
    Portfolio& getPortfolio_(const std::string& name);
    void processUpdateData_(const Portfolio& portfolio, std::shared_ptr<Operation> entry, PortfolioUpdate& feedback);

    void saveTickersInfoData_(Service service, Market market, bool flush);
    void saveOrderBookData_(Service service, Market market, bool flush);
    void saveRecentTradesData_(Service service, Market market, bool flush);

private:
    std::map<std::string, std::shared_ptr<Portfolio>> portfolios_;
    TickersInformation tickersInformation_;
    std::map<exchange::Service, RealTimeDataManager<Ticker>> tickerManagers_;
    std::map<exchange::Service, RealTimeDataManager<OrderBookSummary>> orderBookManagers_;
    std::map<exchange::Service, RealTimeDataManager<RecentTradesSummary>> recentTradesManagers_;
    std::map<exchange::Service, RealTimeDataManager<MarketDataSummary>> marketDataManagers_;

    std::function<void(std::string msg)> traceSignal_;
    std::function<void(std::string msg)> infoSignal_;
    std::function<void(std::string msg)> warningSignal_;
    std::function<void(std::string msg)> errorSignal_;
};

} // namespace exchange

#endif // CONTROLLER_H
