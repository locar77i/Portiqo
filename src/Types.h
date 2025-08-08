#ifndef HMI_TYPES_H
#define HMI_TYPES_H


// Stl  headers
#include <string>
#include <chrono>
#include <vector>
#include <map>
#include <queue>
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>


// exchange headers
#include "exchange/CommonTypes.h"


namespace exchange {

/*
Crea un tipo enumerado definido en el namespace exchange asociado a los diferentes tipos de operaciones que se pueden realizar en un exchange: Buy, Sell, Deposit, Withdraw.
Crea una función llamada getOperationName que reciba un valor del enumerado anterior y devuelva el nombre de la operación asociada.
*/
    enum class OperationType {
        Buy,
        Sell,
        Deposit,
        Withdraw
    };

    std::string getOperationName(OperationType type);

    // Función getOperationType que reciba un string con el nombre de la operación y devuelva el tipo de operación asociado.
    OperationType getOperationType(const std::string& str);



    unsigned short getDecimals(Ticket asset);
    unsigned short getPriceDecimals(Ticket asset, Ticket currency);
    std::string stringfy(double value, Ticket ticket, bool symbol = false);


    

    // Implementar un patron visitor en la clase Portfolio para ejecutar funcionalidad variada sobre el vector de operaciones
    class Buy;
    class Sell;
    class Deposit;
    class Withdraw;
    // Visitor interface
    class OperationVisitor {
    public:
        virtual void visit(Buy& buy) = 0;
        virtual void visit(Sell& sell) = 0;
        virtual void visit(Deposit& deposit) = 0;
        virtual void visit(Withdraw& withdraw) = 0;
    };

    

    /*
    Crea una clase abstracta llamada Operation definida en el namespace exchange que contenga el valor enumerado asociado a la operacion, el cual se debe pasar siempre al constructor.
    La clase tendrá los siguientes atributos:
    - type de tipo OperationType
    - Date de tipo std::chrono::system_clock::time_point
    La clase tendrá dos constructores:
    - Un constructor que reciba el tipo de operación y lo pase al constructor de la clase. La fecha se inicializará con la fecha actual.
    - Un constructor que reciba el tipo de operación y la fecha.
    La clase tendrá los siguientes métodos:
    getType() que devuelva el tipo de la operación.
    getTimePoint() que devuelva la fecha de la operación.
    execute() que ejecute la operación.
    */
    class Operation {
    public:
        Operation();
        Operation(std::chrono::system_clock::time_point timePoint);
        virtual ~Operation() = default;

        const std::string& getId() const { return id_; }

        virtual OperationType getType() const = 0;
        std::string getName() const { return getOperationName(getType()); }
        std::chrono::system_clock::time_point getTimePoint() const { return timePoint_; }
        virtual Ticket getAsset() const = 0;
        std::string getAssetName() const { return exchange::getTicketName(getAsset()); }
        virtual double getQuantity() const = 0;
        virtual double getFee() const = 0;
        virtual void accept(OperationVisitor& visitor) = 0;
        virtual void execute() = 0;
        virtual std::string description() const = 0;
        virtual std::string str() const;
        virtual void write(std::ostream& os) const = 0;
        virtual void read(std::istream& is) = 0;
        virtual void read_1_3(std::istream& is) = 0;
        virtual std::string serialize() const = 0;
        virtual void deserialize(const std::string& json) = 0;

    protected:
        std::string id_;
        std::chrono::system_clock::time_point timePoint_;
    };



    // Struct to store the order data
    struct OrderData {
        Market market;
        double price;
        double quantity;
        double total;

        Ticket getAsset() const { return getAssetType(market); }
        Ticket getCurrency() const { return getCurrencyType(market); }
        std::string description() const {
            return "Order " + getMarketName(market) + " at " + std::to_string(price) + " for " + std::to_string(quantity) + " with total " + std::to_string(total);
        }
    };

    // Struct to store a simulated order
    struct DummyOrder {
        OrderData data;
        float multiplier;
        float increment;
        bool autoDate;
        unsigned long long timePoint;

        DummyOrder() : data(), multiplier(1.0), increment(0.0), autoDate(true), timePoint(0) {}

        std::string description() const {
            return data.description() + " at " + std::to_string(timePoint);
        }
    };


    // Struct to store a transfer data
    struct TransferData {
        Ticket asset;
        double quantity;

        std::string description() const {
            return "Transfer " + std::to_string(quantity) + " " + getTicketName(asset);
        }
    };

    // Struct to store a simulated transfer
    struct DummyTransfer {
        TransferData data;
        float multiplier;
        float increment;
        bool autoDate;
        unsigned long long timePoint;

        DummyTransfer() : data(), multiplier(1.0), increment(0.0), autoDate(true), timePoint(0) {}

        std::string description() const {
            return data.description() + " at " + std::to_string(timePoint);
        }
    };



    // struct Investment: struct to store the asset, the quantity, the price and the fee of a Buy operation.
    struct Investment {
        Ticket asset;
        double quantity;
        double price;
        double fee;
        Ticket currency;
    };


    /*
        getProfitTax(): function to calculate the tax of a profit.
        - The function receives the profit and returns the tax of the profit.    
        - The tax is calculated as the 19% of the profit for the first 6000 euros.
        - The tax is calculated as the 21% of the profit for the next 44000 euros.
        - The tax is calculated as the 23% of the profit for the next 150000 euros.
        - The tax is calculated as the 27% of the profit for the next 100000 euros.
        - The tax is calculated as the 28% of the profit for the rest of the profit. 
    */
    double getProfitTax(double profit);




    // struct BalanceEntry: struct to store the balance of an asset after visiting an operation, including the date and the type of the operation visited.
    struct BalanceEntry {
        std::string id;
        std::chrono::system_clock::time_point date;
        OperationType type;
        double quantity;
    };



    class Balance {
    public:
        Balance() : asset_(exchange::Ticket::EUR), quantity_() {}
        Balance(Ticket asset) : asset_(asset), quantity_() {}

        bool empty() const {
            return balanceEntries_.empty();
        }

        Ticket getAsset() const { return asset_; }
        std::string getAssetName() const { return exchange::getTicketName(asset_); }

        double getQuantity() const { return quantity_; }

        void addBalanceEntry(const BalanceEntry& balanceEntry) {
            // Update the quantity of the balance
            quantity_ += balanceEntry.quantity;
            // Store the balance entry
            balanceEntries_.push_back(balanceEntry);
        }

        const std::vector<BalanceEntry>& getBalanceEntries() const {
            return balanceEntries_;
        }

    private:
        Ticket asset_;
        double quantity_;
        std::vector<BalanceEntry> balanceEntries_;
    };


    // The GeneralBalance class represents the balance of all assets in a portfolio.
    // The class will have the following attributes:
    // - balances of type std::map<Ticket, Balance>
    // The class will have the following methods:
    // - addBalanceEntry() that adds a balance to the general balance, by summing the quantity if the asset already exists.
    // - getBalances() that returns the balances of the general balance.
    // - getBalance() that returns the balance of an asset.
    class GeneralBalance {
    public:
        GeneralBalance(double capital=0.0) 
            : balances_()
            , wallet_()
            , capital_(capital) {
            if(capital_ > 0.01) {
                for(auto it : exchange::AvailableTickets) {
                    balances_[it.first] = Balance(it.first);
                    wallet_[it.first] = 0.0;
                }
            }
        }

    public:
        inline double getCapital() const {
            return capital_;
        }

        inline void addBalanceEntry(Ticket asset, const BalanceEntry& balanceEntry) {
            balances_[asset].addBalanceEntry(balanceEntry);
        }

        inline const std::map<Ticket, Balance>& getBalances() const {
            return balances_;
        }
        inline const Balance& getBalance(Ticket asset) const {
            return balances_.at(asset);
        }
        inline double getBalanceQuantity(Ticket asset) const {
            return balances_.at(asset).getQuantity();
        }

        void addWalletEntry(Ticket asset, double quantity) {
            wallet_[asset] += quantity;
        }

       inline double getWalletQuantity(Ticket asset) const {
            return wallet_.at(asset);
        }

        inline double getTotalQuantity(Ticket asset) const {
            return getBalanceQuantity(asset) + getWalletQuantity(asset);
        }

    private:
        double capital_;
        std::map<Ticket, Balance> balances_;
        std::map<Ticket, double> wallet_;
    };




    struct Fee {
        Ticket asset;
        double quantity;
        double price;

        Fee() : asset(Ticket::EUR), quantity(0), price(1) {}
        Fee(Ticket asset, double quantity, double price = 1) : asset(asset), quantity(quantity), price(price) {}

        inline double cost() const {
            return quantity * price;
        }

        std::string description() const {
            std::ostringstream oss;
            oss << " - Asset: " << getTicketName(asset)
                << " - Quantity: " << quantity
                << " - Price: " << price;
            return oss.str();
        }
    };

    Fee calculateBuyFee(Market market, double total, Ticket referenceCurrency, const TickersInformation& tickersInformation, float multiplier = 1, float increment = 0);
    Fee calculateSellFee(Market market, double quantity, Ticket referenceCurrency, const TickersInformation& tickersInformation, float multiplier = 1, float increment = 0);
    Fee calculateDepositFee(Ticket asset, Ticket referenceCurrency, const TickersInformation& tickersInformation, float multiplier = 1, float increment = 0);
    Fee calculateWithdrawFee(Ticket asset, Ticket referenceCurrency, const TickersInformation& tickersInformation, float multiplier = 1, float increment = 0);


} // namespace exchange


#endif // HMI_TYPES_H
