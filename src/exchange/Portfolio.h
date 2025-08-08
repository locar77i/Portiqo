#ifndef EXCHANGE_PORTFOLIO_H
#define EXCHANGE_PORTFOLIO_H

// Stl  headers
#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>

// headers
#include "Types.h"
#include "PortfolioData.h"


#define PORTFOLIO_VERSION_MAX_SIZE 3
#define PORTFOLIO_NAME_MAX_SIZE 32


namespace exchange {
    

    /*
    En el namespace exchange:
    Clase Order, derivada de Operation, con los siguientes atributos:
    - quantity de tipo double
    - asset de tipo Ticket
    - price de tipo double
    - currency de tipo Ticket
    . total de tipo double
    - fee de tipo double
    - market de tipo Market
    Tiene dos constructores al igual que la clase Operation, y deben inferir el mercado en base al activo y la moneda y calcular el total como la cantidad por el precio más la comisión.:
    - Un constructor que reciba el tipo de operación, la cantidad, el activo, el precio, la moneda y la comisión.
    - Un constructor que reciba el tipo de operación, la cantidad, el activo, el precio, la moneda, la comisión y la fecha.
    Tiene los siguientes métodos:
    */
    class Order : public Operation {
    public:
        Order(Market market, double quantity, double price, double total);
        Order(Market market, double quantity, double price, double total, std::chrono::system_clock::time_point timePoint);

        virtual OperationType getType() const = 0;

        Market getMarket() const;
        double getQuantity() const;
        double getPrice() const;
        double getTotal() const;
        Ticket getAsset() const;
        Ticket getCurrency() const;
        double getFee() const;

        void execute() override;
        virtual std::string description() const;
        virtual std::string str() const;
        void write(std::ostream& os) const;
        void read(std::istream& is);
        void read_1_3(std::istream& is);
        std::string serialize() const;
        void deserialize(const std::string& json);

    protected:
        Market market_;
        double quantity_;
        double price_;
        double total_;
        Ticket asset_;
        Ticket currency_;
        double fee_;
    };


    
    class Buy : public Order {
    public:
        // Constructor por defecto
        Buy();

        Buy(Market market, double quantity, double price, double total);

        Buy(Market market, double quantity, double price, double total, std::chrono::system_clock::time_point timePoint);

        OperationType getType() const override;

        void accept(OperationVisitor& visitor) override;

    private:
        void calculateFields_();
    };

    

    class Sell : public Order {
    public:
        // Constructor por defecto
        Sell();

        Sell(Market market, double quantity, double price, double total);

        Sell(Market market, double quantity, double price, double total, std::chrono::system_clock::time_point timePoint);

        OperationType getType() const override;

        void accept(OperationVisitor& visitor) override;

    private:
        void calculateFields_();
    };

    


    
    class Transfer : public Operation {
    public:
        Transfer(double quantity, Ticket asset, double fee);
        Transfer(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint);

        virtual OperationType getType() const = 0;

        double getQuantity() const;  void setQuantity(double quantity) { quantity_ = quantity; }
        Ticket getAsset() const;
        double getFee() const;

        void execute() override;
        virtual std::string description() const;
        virtual std::string str() const;
        void write(std::ostream& os) const;
        void read(std::istream& is);
        void read_1_3(std::istream& is);
        std::string serialize() const;
        void deserialize(const std::string& json);

    private:
        double quantity_;
        Ticket asset_;
        double fee_;
    };

    
    /*
    En el namespace exchange:
    Clases Deposit y Withdraw, derivadas de Transfer, con varios constructores:
    - Un constructor que reciba la cantidad, el activo y la comisión. El constructor debe pasar el tipo de operación al constructor de la clase base.
    - Un constructor que reciba la cantidad, el activo, la comisión y la fecha. El constructor debe pasar el tipo de operación y la fecha al constructor de la clase base.
    */
    class Deposit : public Transfer {
    public:
        // Constructor por defecto
        Deposit();

        Deposit(double quantity, Ticket asset, double fee);

        Deposit(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint);

        OperationType getType() const override;

        void accept(OperationVisitor& visitor) override;
    };


    class Withdraw : public Transfer {
    public:
        // Constructor por defecto
        Withdraw();

        Withdraw(double quantity, Ticket asset, double fee);

        Withdraw(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint);

        OperationType getType() const override;

        void accept(OperationVisitor& visitor) override;
    };

    

    // Class OperationFactory: Factory class to create operations.
    // The factory class will have static methods called create<Operation>Operation() where <Operation> is the name of the operation.
    // The methods will receive the necessary parameters to create the operation and return a shared pointer to the operation according all operation constructors, including default constructors.
    class OperationFactory {
    public:
        static std::shared_ptr<Buy> createBuyOperation();
        static std::shared_ptr<Buy> createBuyOperation(Market market, double quantity, double price, double total);
        static std::shared_ptr<Buy> createBuyOperation(Market market, double quantity, double price, double total, std::chrono::system_clock::time_point timePoint);
        static std::shared_ptr<Sell> createSellOperation();
        static std::shared_ptr<Sell> createSellOperation(Market market, double quantity, double price, double total);
        static std::shared_ptr<Sell> createSellOperation(Market market, double quantity, double price, double total, std::chrono::system_clock::time_point timePoint);
        static std::shared_ptr<Deposit> createDepositOperation();
        static std::shared_ptr<Deposit> createDepositOperation(double quantity, Ticket asset, double fee);
        static std::shared_ptr<Deposit> createDepositOperation(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint);
        static std::shared_ptr<Withdraw> createWithdrawOperation();
        static std::shared_ptr<Withdraw> createWithdrawOperation(double quantity, Ticket asset, double fee);
        static std::shared_ptr<Withdraw> createWithdrawOperation(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint);
    };




    /*
        Implementa el GeneralBalanceVisitor para calcular el balance de todos los activos, calculado sobre todas las operaciones que involucran a cada uno de ellos:
        - Si el activo no se ha operado, el balance será 0.
        - Para un activo cualquiera, las operaciones Deposit deben sumarse al balance.
        - Para un activo cualquiera, las operaciones Withdraw deben ser restadas al balance.
        - Para cada operación Buy, si el activo es el mismo que el del balance, la cantidad debe ser sumada al balance,
        - Además, en cada operación Buy, si la moneda es la misma que la del balance, el total debe ser restado al balance. 
        - Para cada operación Sell, si el activo es el mismo que el del balance, el total debe ser restada al balance.
        - Además, en cada operación Sell, si la moneda es la misma que la del balance, la cantidad debe ser sumada al balance.
        - El visitor debe almacenar todos los datos en un objeto de tipo GeneralBalance.
    */
    class GeneralBalanceVisitor : public OperationVisitor {
    public:
        GeneralBalanceVisitor(exchange::Ticket referenceCurrency, double capital)
            : referenceCurrency_(referenceCurrency), generalbalance_(capital)
        {
            generalbalance_.addWalletEntry(referenceCurrency_, capital);
        }

        void visit(Buy& buy) override {
            Ticket asset = buy.getAsset();
            Ticket currency = buy.getCurrency();
            double quantity = buy.getQuantity();
            double total = buy.getTotal();
            generalbalance_.addBalanceEntry(asset, {buy.getId(), buy.getTimePoint(), buy.getType(), quantity});
            generalbalance_.addBalanceEntry(currency, {buy.getId(), buy.getTimePoint(), buy.getType(), -total});
        }

        void visit(Sell& sell) override {
            Ticket asset = sell.getAsset();
            Ticket currency = sell.getCurrency();
            double quantity = sell.getQuantity();
            double total = sell.getTotal();
            generalbalance_.addBalanceEntry(asset, {sell.getId(), sell.getTimePoint(), sell.getType(), -quantity});
            generalbalance_.addBalanceEntry(currency, {sell.getId(), sell.getTimePoint(), sell.getType(), total});
        }

        void visit(Deposit& deposit) override {
            Ticket asset = deposit.getAsset();
            double netValue = deposit.getQuantity();
            double grossValue = netValue + deposit.getFee();
            generalbalance_.addBalanceEntry(asset, {deposit.getId(), deposit.getTimePoint(), deposit.getType(), netValue});
            generalbalance_.addWalletEntry(asset, -grossValue);
        }

        void visit(Withdraw& withdraw) override {
            Ticket asset = withdraw.getAsset();
            double netValue = withdraw.getQuantity();
            double grossValue = netValue + withdraw.getFee();
            generalbalance_.addBalanceEntry(asset, {withdraw.getId(), withdraw.getTimePoint(), withdraw.getType(), -grossValue});
            generalbalance_.addWalletEntry(asset, netValue);
        }

    public:
        const GeneralBalance& getGeneralBalance() const {
            return generalbalance_;
        }
        
    private:
        exchange::Ticket referenceCurrency_;
        GeneralBalance generalbalance_;
    };


    


     
    /*
        Implements the StatsVisitor to calculate the profit of all buy/sell operations visited.
        - For every Buy operation visited, the visitor must store the asset, the quantity, the price and the fee in a queue of Investments.
        - For every Sell operation visited, the visitor must calculate the profit/lost of the operation adding it to the total profit.
        - The profit/lost of a sell operation is calculated as the quantity * price - fee - (quantity * buyPrice + buyFee). 
        - The visitor must store the total profit in a private attribute accessible through a public method.
    */
    class StatsVisitor : public OperationVisitor {
    public:
        StatsVisitor(Ticket referenceCurrency=Ticket::EUR);

        void visit(Buy& buy) override;
        void visit(Sell& sell) override;
        void visit(Deposit& deposit) override;
        void visit(Withdraw& withdraw) override;

        const StatsData& getStatsData() const {
            return stats_;
        }

    private:
        StatsData stats_;
    };


    /*
    En el namespace exchange:
    Clase Portfolio, encargada de crear y gestionar las operaciones.
    La clase tendrá un vector de punteros de tipo std::shared_ptr a Operation y debe ser threadsafe. El identificador de cada operación será su posición en el vector + 1.
    La clase tendrá  los siguientes métodos:
    - buy: que reciba la cantidad, el precio y la comisión y añada una nueva operación de tipo Buy al vector. Devuelve el id de la operacion.
    - buy: que reciba la cantidad, el precio, la comisión y la fecha y añada una nueva operación de tipo Buy al vector. Devuelve el id de la operacion.
    - sell: que reciba la cantidad, el precio y la comisión y añada una nueva operación de tipo Sell al vector. Devuelve el id de la operacion.
    - sell: que reciba la cantidad, el precio, la comisión y la fecha y añada una nueva operación de tipo Sell al vector. Devuelve el id de la operacion.
    - deposit: que reciba la cantidad, el activo y la comisión y añada una nueva operación de tipo Deposit al vector. Devuelve el id de la operacion.
    - deposit: que reciba la cantidad, el activo, la comisión y la fecha y añada una nueva operación de tipo Deposit al vector. Devuelve el id de la operacion.
    - withdraw: que reciba la cantidad, el activo y la comisión y añada una nueva operación de tipo Withdraw al vector. Devuelve el id de la operacion.
    - withdraw: que reciba la cantidad, el activo, la comisión y la fecha y añada una nueva operación de tipo Withdraw al vector. Devuelve el id de la operacion.
    Todos los atributos privados deben terminar con el caracter'_'.
    No usar la directiva using.
    */
    class Portfolio {
    public:
        Portfolio();
        Portfolio(const std::string& name, Ticket referenceCurrency, double capital, bool locked = true);

        void setVersion(const std::string& version) {
            version_ = version;
        }
        std::string getVersion() const {
            return version_;
        }

        void setName(const std::string& name) {
            if (name.size() > PORTFOLIO_NAME_MAX_SIZE) { // truncate name if it exceeds PORTFOLIO_NAME_MAX_SIZE characters
                name_ = name.substr(0, PORTFOLIO_NAME_MAX_SIZE);
            } else {
                name_ = name;
            }
        }
        std::string getName() const { return name_; }

        void setFilename(const std::string& filename) { filename_ = filename; }
        std::string getFilename() const { return filename_; }

        Ticket getReferenceCurrency() const { return referenceCurrency_; }

        double getCapital() const { return capital_; }

        bool isLocked() const {
            return locked_;
        }

        bool isModified() const {
            return modified_;
        }

        std::vector<std::shared_ptr<Operation>> getOperations() const { return operations_; }
        std::shared_ptr<Operation> getOperation(unsigned int id) const;

        bool isEmpty() const {
            return operations_.empty();
        }

        std::shared_ptr<Operation> buy(Market market, double price, double quantity, double total);
        std::shared_ptr<Operation> buy(Market market, double price, double quantity, double total, std::chrono::system_clock::time_point timePoint);
        
        std::shared_ptr<Operation> sell(Market market, double price, double quantity, double total);
        std::shared_ptr<Operation> sell(Market market, double price, double quantity, double total, std::chrono::system_clock::time_point timePoint);

        std::shared_ptr<Operation> deposit(double quantity, Ticket asset, double fee);
        std::shared_ptr<Operation> deposit(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint);

        std::shared_ptr<Operation> withdraw(double quantity, Ticket asset, double fee);
        std::shared_ptr<Operation> withdraw(double quantity, Ticket asset, double fee, std::chrono::system_clock::time_point timePoint);

        void setLocked(bool locked);
        std::chrono::system_clock::time_point removeLastOperation();

        void open(const std::string& filename);
        unsigned int save();
        unsigned int saveAs(const std::string& filename) {
            setFilename(filename);
            return save();
        }

        std::string serialize() const;
        void deserialize(const std::string& json);

        void applyVisitor(OperationVisitor& visitor) const;

    private:
        static const std::string currentVersion_;
        unsigned int loadData_1_0_(std::ifstream& file, const std::string& filename);
        unsigned int loadData_1_1_(std::ifstream& file, const std::string& filename);
        unsigned int loadData_1_2_(std::ifstream& file, const std::string& filename);
        unsigned int loadData_1_3_(std::ifstream& file, const std::string& filename);

    private:
        std::shared_ptr<Operation> addOperation_(std::shared_ptr<Operation> operation) {
            operations_.emplace_back(operation);
            modified_ = true;
            return operations_.back();
        }
    
    private:
        std::string version_;
        std::string name_;
        std::string filename_;
        Ticket referenceCurrency_;
        double capital_;
        bool modified_;
        std::vector<std::shared_ptr<Operation>> operations_;
        mutable Market market_;
        mutable bool locked_;
    };


} // namespace exchange

#endif // EXCHANGE_PORTFOLIO_H
