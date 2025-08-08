#include "Portfolio.h"

#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>

// exchange header

// headers




namespace exchange {
    

        // Implementation of Order class methods
    Order::Order(Market market, double quantity, double price, double total)
        : Operation(), market_(market), quantity_(quantity), price_(price), total_(total) {
        asset_ = getAssetType(market_);
        currency_ = getCurrencyType(market_);
    }

    Order::Order(Market market, double quantity, double price, double total, std::chrono::system_clock::time_point timePoint)
        : Operation(timePoint), market_(market), quantity_(quantity), price_(price), total_(total) {
        asset_ = getAssetType(market_);
        currency_ = getCurrencyType(market_);
    }

    Market Order::getMarket() const {
        return market_;
    }

    double Order::getQuantity() const {
        return quantity_;
    }

    double Order::getPrice() const {
        return price_;
    }

    double Order::getTotal() const {
        return total_;
    }

    Ticket Order::getAsset() const {
        return asset_;
    }

    Ticket Order::getCurrency() const {
        return currency_;
    }

    double Order::getFee() const {
        return fee_;
    }

    void Order::execute() {
        // Implement the logic to execute the order
    }

    std::string Order::description() const {
        std::ostringstream oss;
        oss << stringfy(quantity_, asset_, true)
            << " at  " << stringfy(price_, currency_, true)
            << " for " << stringfy(total_, currency_, true)
            << " (" << stringfy(fee_, currency_, true) << " in fees)";
        return oss.str();
    }

    std::string Order::str() const {
        std::string str = Operation::str();
        std::ostringstream oss;
        oss <<  " - " << quantity_ << " " << getTicketName(asset_)
            << ", price: " << price_ << " " << getTicketName(currency_)
            << ", total: " << total_ << " " << getTicketName(currency_)
            << ", fee: " << fee_ << " " << getTicketName(currency_)
            << " [" << getMarketName(market_) << "]";
        return oss.str();
    }


    void Order::write(std::ostream& os) const {
        // Write the id length and the id
        std::size_t idLength = id_.size();
        os.write(reinterpret_cast<const char*>(&idLength), sizeof(idLength));
        os.write(id_.c_str(), idLength);
        os.write(reinterpret_cast<const char*>(&timePoint_), sizeof(timePoint_));
        os.write(reinterpret_cast<const char*>(&market_), sizeof(market_));
        os.write(reinterpret_cast<const char*>(&quantity_), sizeof(quantity_));
        os.write(reinterpret_cast<const char*>(&price_), sizeof(price_));
        os.write(reinterpret_cast<const char*>(&total_), sizeof(total_));
        os.write(reinterpret_cast<const char*>(&asset_), sizeof(asset_));
        os.write(reinterpret_cast<const char*>(&currency_), sizeof(currency_));
        os.write(reinterpret_cast<const char*>(&fee_), sizeof(fee_));
    }

    void Order::read(std::istream& is) {
        is.read(reinterpret_cast<char*>(&timePoint_), sizeof(timePoint_));
        is.read(reinterpret_cast<char*>(&market_), sizeof(market_));
        is.read(reinterpret_cast<char*>(&quantity_), sizeof(quantity_));
        is.read(reinterpret_cast<char*>(&price_), sizeof(price_));
        is.read(reinterpret_cast<char*>(&total_), sizeof(total_));
        is.read(reinterpret_cast<char*>(&asset_), sizeof(asset_));
        is.read(reinterpret_cast<char*>(&currency_), sizeof(currency_));
        is.read(reinterpret_cast<char*>(&fee_), sizeof(fee_));
    }

    void Order::read_1_3(std::istream& is) {
        // Read the id length and the id
        std::size_t idLength;
        is.read(reinterpret_cast<char*>(&idLength), sizeof(idLength));
        id_.resize(idLength);
        is.read(const_cast<char*>(id_.c_str()), idLength);
        read(is);
    }

    std::string Order::serialize() const {
        std::time_t timePoint = std::chrono::system_clock::to_time_t(getTimePoint());
        std::tm local_tm;
        localtime_s(&local_tm, &timePoint);
        std::ostringstream oss;
        oss << "{ \"type\": \"" << getOperationName(getType())
            << "\", \"timePoint\": \"" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S")
            << "\", \"quantity\": " << quantity_
            << ", \"asset\": \"" << getTicketName(asset_)
            << "\", \"price\": " << price_
            << ", \"currency\": \"" << getTicketName(currency_)
            << "\", \"fee\": " << fee_
            << ", \"market\": \"" << getMarketName(market_) << " }";
        return oss.str();
    }

    void Order::deserialize(const std::string& json) {
        // Implement the logic to deserialize the json string
    }


    // Implementation of Buy class
    Buy::Buy() : Order(Market::BTC_EUR, 0, 0, 0) {}

    Buy::Buy(Market market, double quantity, double price, double total)
        : Order(market, quantity, price, total) {
        calculateFields_();
    }

    Buy::Buy(Market market, double quantity, double price, double total, std::chrono::system_clock::time_point timePoint)
        : Order(market, quantity, price, total, timePoint) {
        calculateFields_();
    }

    OperationType Buy::getType() const {
        return OperationType::Buy;
    }

    void Buy::accept(OperationVisitor& visitor) {
        visitor.visit(*this);
    }

    void Buy::calculateFields_() {
        // Calculate the fields of the buy operation
        fee_ = total_ - quantity_ * price_;
    }


    // Implementation of Sell class
    Sell::Sell() : Order(Market::BTC_EUR, 0, 0, 0) {}

    Sell::Sell(Market market, double quantity, double price, double total)
        : Order(market, quantity, price, total) {
        calculateFields_();
    }

    Sell::Sell(Market market, double quantity, double price, double total, std::chrono::system_clock::time_point timePoint)
        : Order(market, quantity, price, total, timePoint) {
        calculateFields_();
    }

    OperationType Sell::getType() const {
        return OperationType::Sell;
    }

    void Sell::accept(OperationVisitor& visitor) {
        visitor.visit(*this);
    }

    void Sell::calculateFields_() {
        // Calculate the fields of the sell operation
        fee_ = (quantity_ * price_) - total_;
    }


       // Implementation of Transfer class

    Transfer::Transfer(double quantity, Ticket asset, double fee)
        : Operation(), quantity_(quantity), asset_(asset), fee_(fee) {}

    Transfer::Transfer(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint)
        : Operation(timePoint), quantity_(quantity), asset_(asset), fee_(fee) {}

    double Transfer::getQuantity() const {
        return quantity_;
    }

    Ticket Transfer::getAsset() const {
        return asset_;
    }

    double Transfer::getFee() const {
        return fee_;
    }

    void Transfer::execute() {
        // Implement the logic to execute the transfer
    }

    std::string Transfer::description() const {
        std::ostringstream oss;
        oss << stringfy(quantity_, asset_, true) << " transferred " 
            << " (" << stringfy(fee_, asset_, true) << " in fees)";
        return oss.str();
    }

    std::string Transfer::str() const {
        std::string str = Operation::str();
        std::ostringstream oss;
        oss << " - " << quantity_  << " " << getTicketName(asset_)
                  << ", fee: " << fee_ << " " << getTicketName(asset_);
        return oss.str();
    }


    void Transfer::write(std::ostream& os) const {
        // Write the id length and the id
        std::size_t idLength = id_.size();
        os.write(reinterpret_cast<const char*>(&idLength), sizeof(idLength));
        os.write(id_.c_str(), idLength);
        os.write(reinterpret_cast<const char*>(&timePoint_), sizeof(timePoint_));
        os.write(reinterpret_cast<const char*>(&quantity_), sizeof(quantity_));
        os.write(reinterpret_cast<const char*>(&asset_), sizeof(asset_));
        os.write(reinterpret_cast<const char*>(&fee_), sizeof(fee_));
    }

    void Transfer::read(std::istream& is) {
        is.read(reinterpret_cast<char*>(&timePoint_), sizeof(timePoint_));
        is.read(reinterpret_cast<char*>(&quantity_), sizeof(quantity_));
        is.read(reinterpret_cast<char*>(&asset_), sizeof(asset_));
        is.read(reinterpret_cast<char*>(&fee_), sizeof(fee_));
    }

    void Transfer::read_1_3(std::istream& is) {
        // Read the id length and the id
        std::size_t idLength;
        is.read(reinterpret_cast<char*>(&idLength), sizeof(idLength));
        id_.resize(idLength);
        is.read(const_cast<char*>(id_.c_str()), idLength);
        read(is);
    }

    std::string Transfer::serialize() const {
        std::time_t timePoint = std::chrono::system_clock::to_time_t(getTimePoint());
        std::tm local_tm;
        localtime_s(&local_tm, &timePoint);
        std::ostringstream oss;
        oss << "{ \"type\": \"" << getOperationName(getType())
            << "\", \"timePoint\": \"" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S")
            << "\", \"quantity\": " << quantity_
            << ", \"asset\": \"" << getTicketName(asset_)
            << "\", \"fee\": " << fee_ << " }";
        return oss.str();
    }

    void Transfer::deserialize(const std::string& json) {
        // Implement the logic to deserialize the transfer
    }
    
    // Implementation of Deposit class
    Deposit::Deposit() : Transfer(0, Ticket::BTC, 0) {}

    Deposit::Deposit(double quantity, Ticket asset, double fee)
        : Transfer(quantity, asset, fee) {}

    Deposit::Deposit(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint)
        : Transfer(quantity, asset, fee, timePoint) {}

    OperationType Deposit::getType() const {
        return OperationType::Deposit;
    }

    void Deposit::accept(OperationVisitor& visitor) {
        visitor.visit(*this);
    }

    // Implementation of Withdraw class
    Withdraw::Withdraw() : Transfer(0, Ticket::BTC, 0) {}

    Withdraw::Withdraw(double quantity, Ticket asset, double fee)
        : Transfer(quantity, asset, fee) {}

    Withdraw::Withdraw(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint)
        : Transfer(quantity, asset, fee, timePoint) {}

    OperationType Withdraw::getType() const {
        return OperationType::Withdraw;
    }

    void Withdraw::accept(OperationVisitor& visitor) {
        visitor.visit(*this);
    }


    // Implementation of OperationFactory class
    std::shared_ptr<Buy> OperationFactory::createBuyOperation() {
        return std::make_shared<Buy>();
    }

    std::shared_ptr<Buy> OperationFactory::createBuyOperation(Market market, double quantity, double price, double total) {
        return std::make_shared<Buy>(market, quantity, price, total);
    }

    std::shared_ptr<Buy> OperationFactory::createBuyOperation(Market market, double quantity, double price, double total, std::chrono::system_clock::time_point timePoint) {
        return std::make_shared<Buy>(market, quantity, price, total, timePoint);
    }

    std::shared_ptr<Sell> OperationFactory::createSellOperation() {
        return std::make_shared<Sell>();
    }

    std::shared_ptr<Sell> OperationFactory::createSellOperation(Market market, double quantity, double price, double total) {
        return std::make_shared<Sell>(market, quantity, price, total);
    }

    std::shared_ptr<Sell> OperationFactory::createSellOperation(Market market, double quantity, double price, double total, std::chrono::system_clock::time_point timePoint) {
        return std::make_shared<Sell>(market, quantity, price, total, timePoint);
    }

    std::shared_ptr<Deposit> OperationFactory::createDepositOperation() {
        return std::make_shared<Deposit>();
    }

    std::shared_ptr<Deposit> OperationFactory::createDepositOperation(double quantity, Ticket asset, double fee) {
        return std::make_shared<Deposit>(quantity, asset, fee);
    }

    std::shared_ptr<Deposit> OperationFactory::createDepositOperation(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint) {
        return std::make_shared<Deposit>(quantity, asset, fee, timePoint);
    }

    std::shared_ptr<Withdraw> OperationFactory::createWithdrawOperation() {
        return std::make_shared<Withdraw>();
    }

    std::shared_ptr<Withdraw> OperationFactory::createWithdrawOperation(double quantity, Ticket asset, double fee) {
        return std::make_shared<Withdraw>(quantity, asset, fee);
    }

    std::shared_ptr<Withdraw> OperationFactory::createWithdrawOperation(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint) {
        return std::make_shared<Withdraw>(quantity, asset, fee, timePoint);
    }




    // Implementation of StatsVisitor methods
    StatsVisitor::StatsVisitor(Ticket referenceCurrency) : stats_(referenceCurrency) {}

    void StatsVisitor::visit(Buy& buy) {
        Investment investment = {buy.getAsset(), buy.getQuantity(), buy.getPrice(), buy.getFee(), buy.getCurrency()};
        stats_.addInvestment(buy.getMarket(), investment);
        double netValue = buy.getQuantity() * buy.getPrice();
        double total = netValue + buy.getFee();
        stats_.addVolume(buy.getMarket(), total);
    }

    void StatsVisitor::visit(Sell& sell) {
        double sellQuantity = sell.getQuantity();
        double sellPrice = sell.getPrice();
        double sellFee = sell.getFee();
        double total = sell.getQuantity() * sell.getPrice();
        double netValue = total - sell.getFee();
        
        auto &queue = stats_.getInvestment(sell.getMarket());
        while (sellQuantity > 0 && !queue.empty()) {
            Investment& investment = queue.front();
            if (investment.quantity <= sellQuantity) {
                double buyPartial = investment.quantity * investment.price + investment.fee;
                double partialSellFee = sellFee * (investment.quantity / sellQuantity);
                double sellPartial = investment.quantity * sellPrice - partialSellFee;
                stats_.addCost(sell.getMarket(), buyPartial);
                stats_.addProfit(sellPartial - buyPartial);
                queue.pop();
                sellFee -= partialSellFee;
                sellQuantity -= investment.quantity;
            } else {
                double partialBuyFee = investment.fee * (sellQuantity / investment.quantity);
                double buyPartial = sellQuantity * investment.price + partialBuyFee;
                double sellPartial = sellQuantity * sellPrice - sellFee;
                stats_.addCost(sell.getMarket(), buyPartial);
                stats_.addProfit(sellPartial - buyPartial);
                investment.quantity -= sellQuantity;
                investment.fee -= partialBuyFee;
                sellQuantity = 0;
            }
        }
        stats_.addWorth(sell.getMarket(), netValue);
        stats_.addVolume(sell.getMarket(), total);
    }

    void StatsVisitor::visit(Deposit& deposit) {
        // Deposits do not affect profit nor investment
    }

    void StatsVisitor::visit(Withdraw& withdraw) {
        // Withdrawals do not affect profit nor investment
    }


    // Set the version of the portfolio
    const std::string Portfolio::currentVersion_ = "1.3";



    // Class Portfolio implementation
    Portfolio::Portfolio()
        : version_(currentVersion_)
        , filename_()
        , modified_(false)
        , referenceCurrency_(Ticket::EUR)
        , capital_(0)
        , market_(Market::BTC_EUR)
        , locked_(true)
    {
        setName("MyPortfolio");
    }

    Portfolio::Portfolio(const std::string& name, Ticket referenceCurrency, double capital, bool locked)
        : version_(currentVersion_)
        , filename_()
        , modified_(false)
        , referenceCurrency_(referenceCurrency)
        , capital_(capital)
        , market_(Market::BTC_EUR)
        , locked_(locked)
    {
        setName(name);
    }

    std::shared_ptr<Operation> Portfolio::getOperation(unsigned int index) const
    {
        if (index < operations_.size()) {
            return operations_[index];
        }
        return nullptr;
    }

    std::shared_ptr<Operation> Portfolio::buy(Market market, double price, double quantity, double total) {
        return addOperation_(OperationFactory::createBuyOperation(market, quantity, price, total, std::chrono::system_clock::now()));
    }

    std::shared_ptr<Operation> Portfolio::buy(Market market, double price, double quantity, double total, std::chrono::system_clock::time_point timePoint) {
        if(isLocked()) {
            throw std::runtime_error("The portfolio is locked");
        }
        return addOperation_(OperationFactory::createBuyOperation(market, quantity, price, total, timePoint));
    }


    std::shared_ptr<Operation> Portfolio::sell(Market market, double price, double quantity, double total) {
        return addOperation_(OperationFactory::createSellOperation(market, quantity, price, total, std::chrono::system_clock::now()));
    }

    std::shared_ptr<Operation> Portfolio::sell(Market market, double price, double quantity, double total, std::chrono::system_clock::time_point timePoint) {
        if(isLocked()) {
            throw std::runtime_error("The portfolio is locked");
        }
        return addOperation_(OperationFactory::createSellOperation(market, quantity, price, total, timePoint));
    }


    std::shared_ptr<Operation> Portfolio::deposit(double quantity, Ticket asset, double fee) {
        return addOperation_(OperationFactory::createDepositOperation(quantity, asset, fee));
    }

    std::shared_ptr<Operation> Portfolio::deposit(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint) {
        if(isLocked()) {
            throw std::runtime_error("The portfolio is locked");
        }
        return addOperation_(OperationFactory::createDepositOperation(quantity, asset, fee, timePoint));
    }

    std::shared_ptr<Operation> Portfolio::withdraw(double quantity, Ticket asset, double fee) {
        return addOperation_(OperationFactory::createWithdrawOperation(quantity, asset, fee));
    }

    std::shared_ptr<Operation> Portfolio::withdraw(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint) {
        if(isLocked()) {
            throw std::runtime_error("The portfolio is locked");
        }
        return addOperation_(OperationFactory::createWithdrawOperation(quantity, asset, fee, timePoint));
    }

    void Portfolio::setLocked(bool locked) {
        locked_ = locked;
    }

    std::chrono::system_clock::time_point  Portfolio::removeLastOperation() {
        // Delete the last operation of the vector
        if(isLocked()) {
            throw std::runtime_error("The portfolio is locked");
        }
        if (operations_.empty()) {
            throw std::runtime_error("No operations to remove in portfolio '" + name_ + "'");
        }
        // get the time point of the last operation
        std::chrono::system_clock::time_point timePoint = operations_.back()->getTimePoint();
        operations_.pop_back();
        modified_ = true;
        return timePoint;

    }

        // open(): Lee de un fichero binario con el nombre pasado como argumento el número de operaciones y el detalle de cada una de ellas
        // The metod throw exceptions if there is any error opening or reading the file.
        // The method returns the number of operations written to the file.
        void Portfolio::open(const std::string& filename) {
            std::ifstream file(filename, std::ios::binary);
            if (!file.is_open()) {
                throw std::runtime_error("Error opening file: " + filename);
            }
            // Load the version of the portfolio
            char buffer[PORTFOLIO_VERSION_MAX_SIZE+1];
            file.read(buffer, PORTFOLIO_VERSION_MAX_SIZE);
            if (!file) {
                throw std::runtime_error("Error reading the portfolio version from file: " + filename);
            }
            // Compare the readed value agains the managed versions of the portfolio
            unsigned int numOperations;
            buffer[PORTFOLIO_VERSION_MAX_SIZE] = '\0';
            setVersion(buffer);
            if (getVersion() == "1.0") {
                numOperations = loadData_1_0_(file, filename);
            } else if (getVersion() == "1.1") {
                numOperations = loadData_1_1_(file, filename);
            } else if (getVersion() == "1.2") {
                numOperations = loadData_1_2_(file, filename);
            } else if (getVersion() == "1.3") {
                numOperations = loadData_1_3_(file, filename);
            } else {
                throw std::runtime_error("Unknow portfolio version : " + getVersion() + " (Current version: '" + currentVersion_ + "')");
            }
            file.close();
            setFilename(filename);
/* DELETEME: Modification (on the fly) for testing purposes
            Withdraw* withdraw = dynamic_cast<Withdraw*>(operations_.at(30).get());
            withdraw->setQuantity(309.05);
*/
        }

        unsigned int Portfolio::loadData_1_0_(std::ifstream& file, const std::string& filename) {
            // Load the name of the portfolio from the file reading the name size first and use it to set the portfolio name
            unsigned int nameSize = 0;
            file.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
            if (!file) {
                throw std::runtime_error("Error reading the portfolio name size from file: " + filename);
            }
            if (nameSize > 0) {
                char buffer[PORTFOLIO_NAME_MAX_SIZE+1];
                if (nameSize >= sizeof(buffer)) {
                    throw std::runtime_error("Portfolio name size exceeds buffer size");
                }
                file.read(buffer, nameSize);
                if (!file) {
                    throw std::runtime_error("Error reading the portfolio name from file: " + filename);
                }
                buffer[nameSize] = '\0';
                setName(buffer);
            }
            // Load the reference currency
            file.read(reinterpret_cast<char*>(&referenceCurrency_), sizeof(referenceCurrency_));
            if (!file) {
                throw std::runtime_error("Error reading the reference currency from file: " + filename);
            }
            // Load the locked flag
            file.read(reinterpret_cast<char*>(&locked_), sizeof(locked_));
            if (!file) {
                throw std::runtime_error("Error reading the locked flag from file: " + filename);
            }
            // Load the number of operations
            unsigned int numOperations = 0;
            file.read(reinterpret_cast<char*>(&numOperations), sizeof(numOperations));
            if (!file) {
                throw std::runtime_error("Error reading the number of operations from file: " + filename);
            }
            operations_.clear();
            for (unsigned int i = 0; i < numOperations; ++i) {
                unsigned int type = 0;
                file.read(reinterpret_cast<char*>(&type), sizeof(type));
                if (!file) {
                    throw std::runtime_error("Error reading the operation type from file: " + filename);
                }
                OperationType operationType = static_cast<OperationType>(type);
                std::shared_ptr<Operation> operation;
                switch (operationType) {
                    case OperationType::Buy:
                        operation = OperationFactory::createBuyOperation();
                        break;
                    case OperationType::Sell:
                        operation = OperationFactory::createSellOperation();
                        break;
                    case OperationType::Deposit:
                        operation = OperationFactory::createDepositOperation();
                        break;
                    case OperationType::Withdraw:
                        operation = OperationFactory::createWithdrawOperation();
                        break;
                    default:
                        throw std::runtime_error("Unknown operation type in file: " + filename);
                }
                operation->read(file);
                if (!file) {
                    throw std::runtime_error("Error reading the operation details from file: " + filename);
                }
                operations_.push_back(operation);
            }
            return numOperations;
        }

        unsigned int Portfolio::loadData_1_1_(std::ifstream& file, const std::string& filename) {
            // Load the name of the portfolio from the file reading the name size first and use it to set the portfolio name
            unsigned int nameSize = 0;
            file.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
            if (!file) {
                throw std::runtime_error("Error reading the portfolio name size from file: " + filename);
            }
            if (nameSize > 0) {
                char buffer[PORTFOLIO_NAME_MAX_SIZE+1];
                if (nameSize >= sizeof(buffer)) {
                    throw std::runtime_error("Portfolio the name size exceeds buffer size");
                }
                file.read(buffer, nameSize);
                if (!file) {
                    throw std::runtime_error("Error reading the portfolio name from file: " + filename);
                }
                buffer[nameSize] = '\0';
                setName(buffer);
            }
            // Load the reference currency
            file.read(reinterpret_cast<char*>(&referenceCurrency_), sizeof(referenceCurrency_));
            if (!file) {
                throw std::runtime_error("Error reading the reference currency from file: " + filename);
            }
            // Load the number of operations
            unsigned int numOperations = 0;
            file.read(reinterpret_cast<char*>(&numOperations), sizeof(numOperations));
            if (!file) {
                throw std::runtime_error("Error reading the number of operations from file: " + filename);
            }
            operations_.clear();
            for (unsigned int i = 0; i < numOperations; ++i) {
                unsigned int type = 0;
                file.read(reinterpret_cast<char*>(&type), sizeof(type));
                if (!file) {
                    throw std::runtime_error("Error reading the operation type from file: " + filename);
                }
                OperationType operationType = static_cast<OperationType>(type);
                std::shared_ptr<Operation> operation;
                switch (operationType) {
                    case OperationType::Buy:
                        operation = OperationFactory::createBuyOperation();
                        break;
                    case OperationType::Sell:
                        operation = OperationFactory::createSellOperation();
                        break;
                    case OperationType::Deposit:
                        operation = OperationFactory::createDepositOperation();
                        break;
                    case OperationType::Withdraw:
                        operation = OperationFactory::createWithdrawOperation();
                        break;
                    default:
                        throw std::runtime_error("Unknown operation type in file: " + filename);
                }
                operation->read(file);
                if (!file) {
                    throw std::runtime_error("Error reading the operation details from file: " + filename);
                }
                operations_.push_back(operation);
            }
            return numOperations;
        }

        unsigned int Portfolio::loadData_1_2_(std::ifstream& file, const std::string& filename) {
            // Load the name of the portfolio from the file reading the name size first and use it to set the portfolio name
            unsigned int nameSize = 0;
            file.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
            if (!file) {
                throw std::runtime_error("Error reading the portfolio name size from file: " + filename);
            }
            if (nameSize > 0) {
                char buffer[PORTFOLIO_NAME_MAX_SIZE+1];
                if (nameSize >= sizeof(buffer)) {
                    throw std::runtime_error("Portfolio the name size exceeds buffer size");
                }
                file.read(buffer, nameSize);
                if (!file) {
                    throw std::runtime_error("Error reading the portfolio name from file: " + filename);
                }
                buffer[nameSize] = '\0';
                setName(buffer);
            }
            // Load the reference currency
            file.read(reinterpret_cast<char*>(&referenceCurrency_), sizeof(referenceCurrency_));
            if (!file) {
                throw std::runtime_error("Error reading the reference currency from file: " + filename);
            }
            // Load the capital
            file.read(reinterpret_cast<char*>(&capital_), sizeof(capital_));
            if (!file) {
                throw std::runtime_error("Error reading the capital from file: " + filename);
            }
            // Load the number of operations
            unsigned int numOperations = 0;
            file.read(reinterpret_cast<char*>(&numOperations), sizeof(numOperations));
            if (!file) {
                throw std::runtime_error("Error reading the number of operations from file: " + filename);
            }
            operations_.clear();
            for (unsigned int i = 0; i < numOperations; ++i) {
                unsigned int type = 0;
                file.read(reinterpret_cast<char*>(&type), sizeof(type));
                if (!file) {
                    throw std::runtime_error("Error reading the operation type from file: " + filename);
                }
                OperationType operationType = static_cast<OperationType>(type);
                std::shared_ptr<Operation> operation;
                switch (operationType) {
                    case OperationType::Buy:
                        operation = OperationFactory::createBuyOperation();
                        break;
                    case OperationType::Sell:
                        operation = OperationFactory::createSellOperation();
                        break;
                    case OperationType::Deposit:
                        operation = OperationFactory::createDepositOperation();
                        break;
                    case OperationType::Withdraw:
                        operation = OperationFactory::createWithdrawOperation();
                        break;
                    default:
                        throw std::runtime_error("Unknown operation type in file: " + filename);
                }
                operation->read(file);
                if (!file) {
                    throw std::runtime_error("Error reading the operation details from file: " + filename);
                }
                operations_.push_back(operation);
            }

            return numOperations;
        }

        unsigned int Portfolio::loadData_1_3_(std::ifstream& file, const std::string& filename) {
            // Load the name of the portfolio from the file reading the name size first and use it to set the portfolio name
            unsigned int nameSize = 0;
            file.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
            if (!file) {
                throw std::runtime_error("Error reading the portfolio name size from file: " + filename);
            }
            if (nameSize > 0) {
                char buffer[PORTFOLIO_NAME_MAX_SIZE+1];
                if (nameSize >= sizeof(buffer)) {
                    throw std::runtime_error("Portfolio the name size exceeds buffer size");
                }
                file.read(buffer, nameSize);
                if (!file) {
                    throw std::runtime_error("Error reading the portfolio name from file: " + filename);
                }
                buffer[nameSize] = '\0';
                setName(buffer);
            }
            // Load the reference currency
            file.read(reinterpret_cast<char*>(&referenceCurrency_), sizeof(referenceCurrency_));
            if (!file) {
                throw std::runtime_error("Error reading the reference currency from file: " + filename);
            }
            // Load the capital
            file.read(reinterpret_cast<char*>(&capital_), sizeof(capital_));
            if (!file) {
                throw std::runtime_error("Error reading the capital from file: " + filename);
            }
            // Load the number of operations
            unsigned int numOperations = 0;
            file.read(reinterpret_cast<char*>(&numOperations), sizeof(numOperations));
            if (!file) {
                throw std::runtime_error("Error reading the number of operations from file: " + filename);
            }
            operations_.clear();
            for (unsigned int i = 0; i < numOperations; ++i) {
                unsigned int type = 0;
                file.read(reinterpret_cast<char*>(&type), sizeof(type));
                if (!file) {
                    throw std::runtime_error("Error reading the operation type from file: " + filename);
                }
                OperationType operationType = static_cast<OperationType>(type);
                std::shared_ptr<Operation> operation;
                switch (operationType) {
                    case OperationType::Buy:
                        operation = OperationFactory::createBuyOperation();
                        break;
                    case OperationType::Sell:
                        operation = OperationFactory::createSellOperation();
                        break;
                    case OperationType::Deposit:
                        operation = OperationFactory::createDepositOperation();
                        break;
                    case OperationType::Withdraw:
                        operation = OperationFactory::createWithdrawOperation();
                        break;
                    default:
                        throw std::runtime_error("Unknown operation type in file: " + filename);
                }
                operation->read_1_3(file);
                if (!file) {
                    throw std::runtime_error("Error reading the operation details from file: " + filename);
                }
                operations_.push_back(operation);
            }
/* DELETEME
            std::shared_ptr<Operation> operation = operations_[6];
            auto & oper = dynamic_cast<Deposit&>(*operation);
            std::tm tm = {};
            std::istringstream ss("2019-11-06 11:00:27");
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            if (ss.fail()) {
                throw std::runtime_error("Failed to parse time string");
            }
            std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            oper.setTimePoint(timePoint);
*/
            return numOperations;
        }

        // save() : Crea un fichero binario con el nombre pasado como argumento y escribe en él el número de operaciones y el detalle de cada una de ellas
        // The metod throw exceptions if there is any error opening or reading the file.
        // The method returns the number of operations written to the file.
        unsigned int Portfolio::save() {
            if (filename_.empty()) {
                throw std::runtime_error("Filename not set for portfolio '" + name_ + "'");
            }
            std::ofstream file(filename_, std::ios::binary);
            if (!file.is_open()) {
                throw std::runtime_error("Error opening file: " + filename_);
            }
            // Save the version of the portfolio
            file.write(currentVersion_.c_str(), currentVersion_.size());
            if (!file) {
                throw std::runtime_error("Error writing portfolio version to file: " + filename_);
            }
            // Save the name of the portfolio: PORTFOLIO_NAME_MAX_SIZE characters maximum
            unsigned int nameSize = static_cast<unsigned int>(name_.size());
            if (nameSize > PORTFOLIO_NAME_MAX_SIZE) {
                throw std::runtime_error("Portfolio name exceeds maximum length of " + std::to_string(PORTFOLIO_NAME_MAX_SIZE) + " characters");
            }
            // Save the name size and the name
            file.write(reinterpret_cast<const char*>(&nameSize), sizeof(nameSize));
            if (!file) {
                throw std::runtime_error("Error writing portfolio name size to file: " + filename_);
            }
            file.write(name_.c_str(), nameSize);
            if (!file) {
                throw std::runtime_error("Error writing portfolio name to file: " + filename_);
            }
            // Save the reference currency
            file.write(reinterpret_cast<const char*>(&referenceCurrency_), sizeof(referenceCurrency_));
            if (!file) {
                throw std::runtime_error("Error writing reference currency to file: " + filename_);
            }
            // Save the portfolio capital
            file.write(reinterpret_cast<const char*>(&capital_), sizeof(capital_));
            if (!file) {
                throw std::runtime_error("Error writing capital to file: " + filename_);
            }
            // Save the number of operations
            unsigned int numOperations = static_cast<unsigned int>(operations_.size());
            file.write(reinterpret_cast<const char*>(&numOperations), sizeof(numOperations));
            if (!file) {
                throw std::runtime_error("Error writing number of operations to file: " + filename_);
            }
            for (const auto& operation : operations_) {
                auto type = operation->getType();
                file.write(reinterpret_cast<const char*>(&type), sizeof(type));
                if (!file) {
                    throw std::runtime_error("Error writing operation type to file: " + filename_);
                }
                operation->write(file);
                if (!file) {
                    throw std::runtime_error("Error writing operation details to file: " + filename_);
                }
            }
            file.close();
            modified_ = false;
            return numOperations;
        }

        // serialize() : devuelve una cadena de texto con el detalle de todas las operaciones en formato json
        std::string Portfolio::serialize() const {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < operations_.size(); ++i) {
                oss << operations_[i]->serialize();
                if (i < operations_.size() - 1) {
                    oss << ", ";
                }
            }
            oss << "]";
            return oss.str();
        }

        // deserialize() : recibe una cadena de texto con el detalle de todas las operaciones en formato json y actualiza el vector de operaciones
        void Portfolio::deserialize(const std::string& json) {}

        // Portfolio method to apply visitor to all operations
        void Portfolio::applyVisitor(OperationVisitor& visitor) const {
            for (auto& operation : operations_) {
                operation->accept(visitor);
            }
        }


} // namespace exchange

