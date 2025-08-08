#include "Controller.h"

// Stl headers
#include <iostream>



namespace exchange {

// Implementation of the Controller class
Controller::Controller()
    : portfolios_()
    , tickersInformation_()
    , tickerManagers_()
    , orderBookManagers_()
    , recentTradesManagers_()
    , marketDataManagers_() {
}

void Controller::init() {
}

void Controller::createPortfolio(const std::string& name, Ticket referenceCurrency, double capital, PortfolioData& data) {
    // check if the name already exists
    if (portfolios_.find(name) != portfolios_.end()) {
        throw std::invalid_argument("Portfolio " + name + " already exists"); // Error code: Already exists (100)
    }
    else { // Create a new portfolio and add it to the map
        add_(std::make_shared<Portfolio>(name, referenceCurrency, capital), data);
    }
}

void Controller::openPortfolio(const std::string& filename, PortfolioData& data) {
    auto portfolio = std::make_shared<Portfolio>();
    portfolio->open(filename);
    if (portfolios_.find(portfolio->getName()) != portfolios_.end()) {
        throw std::invalid_argument("Portfolio " + portfolio->getName() + " already exists"); // Error code: Already exists
    }
    add_(portfolio, data); 
}

bool Controller::savePortfolio(const std::string& name, unsigned int& numOperations) {
    auto& portfolio = getPortfolio_(name);
    if(portfolio.isModified()) {
        numOperations = portfolio.save();
        return true;
    }
    return false;
}

void Controller::savePortfolioAs(const std::string& name, const std::string& filename, unsigned int& numOperations) {
    auto& portfolio = getPortfolio_(name);
    numOperations = getPortfolio_(name).saveAs(filename);
}

bool Controller::closePortfolio(const std::string& name, bool force) {
    
    auto& portfolio = getPortfolio_(name);
    if (portfolio.isModified() && !force) {
        return false;
    }    
    portfolios_.erase(name);
    return true;
}

void Controller::lockPortfolio(const std::string& name, bool locked) {
    auto& portfolio = getPortfolio_(name);
    portfolio.setLocked(locked);
}

void Controller::order(OperationType operationType, const std::string& name, const OrderData& order, PortfolioUpdate& feedback) {
    auto& portfolio = getPortfolio_(name);
    std::shared_ptr<Operation> operation;
    if (operationType == OperationType::Buy) {
        Fee fee = calculateBuyFee(order.market, order.total, portfolio.getReferenceCurrency(), tickersInformation_);
        emitTraceSignal_(std::string("Calculated fee: ") + fee.description());
        emitTraceSignal_(std::string("OrderData: ") + order.description());
        double quantity = ((order.total - fee.quantity) / order.price);
        emitTraceSignal_(std::string("Calculated quantity: ") + std::to_string(quantity));
        operation = portfolio.buy(order.market, order.price, quantity, order.total);
    } else {
        Fee fee = calculateSellFee(order.market, order.quantity, portfolio.getReferenceCurrency(), tickersInformation_);
        emitTraceSignal_(std::string("Calculated fee: ") + fee.description());
        emitTraceSignal_(std::string("OrderData: ") + order.description());
        double total = (order.price * (order.quantity - fee.quantity));
        emitTraceSignal_(std::string("Calculated total: ") + std::to_string(total));
        operation = portfolio.sell(order.market, order.price, order.quantity, total);
    }
    processUpdateData_(portfolio, operation, feedback);
}

void Controller::simulateOrder(OperationType operationType, const std::string& name, const DummyOrder& order, PortfolioUpdate& feedback) {    
    auto& portfolio = getPortfolio_(name);
    if(portfolio.isLocked()) {
        throw std::runtime_error("The portfolio is locked");
    }
    std::shared_ptr<Operation> operation;
    if (operationType == OperationType::Buy) {
        Fee fee = calculateBuyFee(order.data.market, order.data.total, portfolio.getReferenceCurrency(), tickersInformation_, order.multiplier, order.increment);
        double quantity = ((order.data.total - fee.quantity) / order.data.price);
        operation = portfolio.buy(order.data.market, order.data.price, quantity, order.data.total);
    } else {
        Fee fee = calculateSellFee(order.data.market, order.data.quantity, portfolio.getReferenceCurrency(), tickersInformation_, order.multiplier, order.increment);
        double total = (order.data.price * (order.data.quantity - fee.quantity));
        operation = portfolio.sell(order.data.market, order.data.price, order.data.quantity, total);
    }
    processUpdateData_(portfolio, operation, feedback);
}

void Controller::transfer(OperationType operationType, const std::string& name, const TransferData& transfer, PortfolioUpdate& feedback) {
    auto& portfolio = getPortfolio_(name);
    std::shared_ptr<Operation> operation;
    if (operationType == OperationType::Deposit) {
        Fee fee = calculateDepositFee(transfer.asset, portfolio.getReferenceCurrency(), tickersInformation_);
        double quantity = transfer.quantity - fee.quantity;
        operation = portfolio.deposit(quantity, transfer.asset, fee.quantity);
    } else {
        Fee fee = calculateWithdrawFee(transfer.asset, portfolio.getReferenceCurrency(), tickersInformation_);
        double quantity = transfer.quantity - fee.quantity;
        operation = portfolio.withdraw(quantity, transfer.asset, fee.quantity);
    }
    processUpdateData_(portfolio, operation, feedback);
}

void Controller::simulateTransfer(OperationType operationType, const std::string& name, const DummyTransfer& transfer, PortfolioUpdate& feedback) {
    auto& portfolio = getPortfolio_(name);
    if(portfolio.isLocked()) {
        throw std::runtime_error("The portfolio is locked");
    }
    std::shared_ptr<Operation> operation;
    if (operationType == OperationType::Deposit) {
        Fee fee = calculateDepositFee(transfer.data.asset, portfolio.getReferenceCurrency(), tickersInformation_, transfer.multiplier, transfer.increment);
        double quantity = transfer.data.quantity - fee.quantity;
        operation = portfolio.deposit(quantity, transfer.data.asset, fee.quantity, std::chrono::system_clock::from_time_t(transfer.timePoint));
    } else {
        Fee fee = calculateWithdrawFee(transfer.data.asset, portfolio.getReferenceCurrency(), tickersInformation_, transfer.multiplier, transfer.increment);
        double quantity = transfer.data.quantity - fee.quantity;
        operation = portfolio.withdraw(quantity, transfer.data.asset, fee.quantity, std::chrono::system_clock::from_time_t(transfer.timePoint));
    }
    processUpdateData_(portfolio, operation, feedback);
}

void Controller::removeLastPortfolioOperation(const std::string& name, PortfolioRemove& feedback) {
    auto& portfolio = getPortfolio_(name);
    feedback.timePoint = portfolio.removeLastOperation();
    GeneralBalanceVisitor balanceVisitor(portfolio.getReferenceCurrency(), portfolio.getCapital());
    portfolio.applyVisitor(balanceVisitor);
    StatsVisitor statsVisitor(portfolio.getReferenceCurrency());
    portfolio.applyVisitor(statsVisitor);
    feedback.count = portfolio.getOperations().size();
    feedback.balance = balanceVisitor.getGeneralBalance();
    feedback.stats = statsVisitor.getStatsData();
}

void Controller::addTicker(exchange::Service service, Market market, const Ticker& ticker) {
    auto it = tickerManagers_.find(service);
    if(it == tickerManagers_.end()) {
        auto result = tickerManagers_.insert({service, RealTimeDataManager<Ticker>(service, "C:/DATA")});
        if(result.second) {
            it = result.first;
        } else {
            emitErrorSignal_("Memory error - cannot create a new RealTimeDataManager for service " + getServiceName(service));
        }
    }
    it->second.addData(market, ticker);
}

void Controller::addOrderBookSummary(Service service, Market market, const OrderBookSummary& orderBookSummary) {
    auto it = orderBookManagers_.find(service);
    if(it == orderBookManagers_.end()) {
        auto result = orderBookManagers_.insert({service, RealTimeDataManager<OrderBookSummary>(service, "C:/DATA")});
        if(result.second) {
            it = result.first;
        } else {
            emitErrorSignal_("Memory error - cannot create a new RealTimeDataManager for service " + getServiceName(service));
        }
    }
    it->second.addData(market, orderBookSummary);
}

void Controller::addRecentTradesSummary(exchange::Service service, Market market, const RecentTradesSummary& recentTradesSummary) {
    auto it = recentTradesManagers_.find(service);
    if(it == recentTradesManagers_.end()) {
        auto result = recentTradesManagers_.insert({service, RealTimeDataManager<RecentTradesSummary>(service, "C:/DATA")});
        if(result.second) {
            it = result.first;
        } else {
            emitErrorSignal_("Memory error - cannot create a new RealTimeDataManager for service " + getServiceName(service));
        }
    }
    it->second.addData(market, recentTradesSummary);
}

void Controller::addMarketDataSummary(Service service, Market market, const Ticker& ticker) {
    // get the oldest order book summary
    auto rt_it = recentTradesManagers_.find(service);
    if(rt_it != recentTradesManagers_.end()) {
        auto ob_it = orderBookManagers_.find(service);
        if(ob_it != orderBookManagers_.end()) {
            try {
                addMarketDataSummary(service, market, MarketDataSummary(ticker, rt_it->second.getLastData(market), ob_it->second.getLastData(market)));
            } catch(const std::exception&) {}
        }
    }
}

void Controller::addMarketDataSummary(Service service, Market market, const MarketDataSummary& marketDataSummary) {
    auto it = marketDataManagers_.find(service);
    if(it == marketDataManagers_.end()) {
        auto result = marketDataManagers_.insert({service, RealTimeDataManager<MarketDataSummary>(service, "C:/DATA")});
        if(result.second) {
            it = result.first;
        } else {
            emitErrorSignal_("Memory error - cannot create a new RealTimeDataManager for service " + getServiceName(service));
        }
    }
    it->second.addData(market, marketDataSummary);
}

void Controller::saveData(Service service, ProxyType type, Market market, bool flush) {
    switch(type) {
        case ProxyType::TickersInformation:
            saveTickersInfoData_(service, market, flush);
            break;
        case ProxyType::OrderBook:
            saveOrderBookData_(service, market, flush);
            break;
        case ProxyType::RecentTrades:
            saveRecentTradesData_(service, market, flush);
            break;
        default:
            emitErrorSignal_("Unknown ProxyType: " + std::to_string(static_cast<int>(type)));
            break;
    }   
}

void Controller::processUpdateData_(const Portfolio& portfolio, std::shared_ptr<Operation> entry, PortfolioUpdate& feedback) {
    GeneralBalanceVisitor balanceVisitor(portfolio.getReferenceCurrency(), portfolio.getCapital());
    portfolio.applyVisitor(balanceVisitor);
    StatsVisitor statsVisitor(portfolio.getReferenceCurrency());
    portfolio.applyVisitor(statsVisitor);
    feedback.count = portfolio.getOperations().size();
    feedback.operation = entry;
    feedback.balance = balanceVisitor.getGeneralBalance();
    feedback.stats = statsVisitor.getStatsData();
}


Portfolio & Controller::getPortfolio_(const std::string& name) {
    auto it = portfolios_.find(name);
    if (it == portfolios_.end()) {
        throw std::invalid_argument("Portfolio '" + name + "' not found"); // Error code: Not found (101)
    }
    if(it->second == nullptr) {
        throw std::runtime_error("Portfolio '" + name + "' is null"); // Error code: Null pointer (10)
    }
    return *it->second;
}

void Controller::add_(std::shared_ptr<Portfolio> portfolio, PortfolioData& data) {
    // Add the portfolio to the map
    portfolios_[portfolio->getName()] = portfolio;
    // Create a visitor to calculate the general balance of the portfolio
    GeneralBalanceVisitor balanceVisitor(portfolio->getReferenceCurrency(), portfolio->getCapital());
    portfolio->applyVisitor(balanceVisitor);
    // Create a visitor to calculate the statistics of the portfolio
    StatsVisitor statsVisitor(portfolio->getReferenceCurrency());
    portfolio->applyVisitor(statsVisitor);
    // Write the portfolio data
    data.version = portfolio->getVersion();
    data.name = portfolio->getName();
    data.filename = portfolio->getFilename();
    data.referenceCurrency = portfolio->getReferenceCurrency();
    data.operations = portfolio->getOperations();
    data.balance = balanceVisitor.getGeneralBalance();
    data.stats = statsVisitor.getStatsData();
}

void Controller::saveTickersInfoData_(Service service, Market market, bool flush) {
    auto it = tickerManagers_.find(service);
    if(it != tickerManagers_.end()) {
        it->second.save(market, flush);
    }
}

void Controller::saveOrderBookData_(Service service, Market market, bool flush) {
    auto it = orderBookManagers_.find(service);
    if(it != orderBookManagers_.end()) {
        it->second.save(market, flush);
    }
}

void Controller::saveRecentTradesData_(Service service, Market market, bool flush) {
    auto it = recentTradesManagers_.find(service);
    if(it != recentTradesManagers_.end()) {
        it->second.save(market, flush);
    }
}

} // namespace exchange
