#ifndef MANAGER_H
#define MANAGER_H

// Stl headers
#include <map>
#include <memory>
#include <string>
#include <thread>

// Qt headers
#include <QObject>

// headers
#include "Types.h"
#include "Controller.h"

// exchange headers
#include "exchange/Portfolio.h"
#include "exchange/Exchange.h"

namespace exchange {


class Manager : public QObject {
    Q_OBJECT

public:
    Manager();

    void init();

signals:
    void portfolioCreated(const PortfolioData& data, bool openFromFile);

    void portfolioOperationAdded(const QString& name, const PortfolioUpdate& data);
    void portfolioOperationRemoved(const QString& name, const PortfolioRemove& data);

    void portfolioSaved(const QString& name, unsigned int numOperations);
    void portfolioSavedAs(const QString& name, const QString& filename, unsigned int numOperations);

    void info(const QString& text);
    void warning(const QString& text);
    void error(const QString& text);
    void trace(const QString& text);

public slots:
    void exec();
    
    void createPortfolio(const QString& name, exchange::Ticket referenceCurrency, double capital);
    void openPortfolio(const QString& filename);
    void savePortfolio(const QString& name, bool feedback);
    void savePortfolioAs(const QString& name, const QString& filename, bool feedback);
    void closePortfolio(const QString& name, bool force);

    void lockPortfolio(const QString& name, bool locked);
    void order(exchange::OperationType operationType, const QString& name, const OrderData& data);
    void simulateOrder(exchange::OperationType operationType, const QString& name, const DummyOrder& data);
    void transfer(exchange::OperationType operationType, const QString& name, const TransferData& data);
    void simulateTransfer(exchange::OperationType operationType, const QString& name, const DummyTransfer& data);
    void removeLastPortfolioOperation(const QString& name);

    void updateTickersInformation(exchange::Service service, TickersInformation tickersInformation) {
        if(service == Service::Kraken) {
            controller_.setTickersInformation(tickersInformation);
        }
        for(const auto& pair : tickersInformation.get()) {
            controller_.addTicker(service, pair.first, pair.second);
            controller_.addMarketDataSummary(service, pair.first, pair.second);
        }
        
    }
    void updateOrderBookSummary(exchange::Service service, Market market, OrderBookSummary orderBookSummary) {
        controller_.addOrderBookSummary(service, market, orderBookSummary);
    }
    void updateRecentTradesSummary(exchange::Service service, Market market, RecentTradesSummary recentTradesSummary) {
        controller_.addRecentTradesSummary(service, market, recentTradesSummary);
    }

    void saveData(Service service, ProxyType type, Market market) {
        controller_.saveData(service, type, market, true);
    }

private:
    exchange::Controller controller_;
};

} // namespace exchange

#endif // MANAGER_H
