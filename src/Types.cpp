// Header
#include "Types.h"

// Stl headers
#include <cmath>

// headers
#include "ULIDGenerator.h"


namespace exchange {


    /*
        getProfitTax(): function to calculate the tax of a profit.
        - The function receives the profit and returns the tax of the profit.    
        - The tax is calculated as the 19% of the profit for the first 6000 euros.
        - The tax is calculated as the 21% of the profit for the next 44000 euros.
        - The tax is calculated as the 23% of the profit for the next 150000 euros.
        - The tax is calculated as the 27% of the profit for the next 100000 euros.
        - The tax is calculated as the 28% of the profit for the rest of the profit. 
    */
    double getProfitTax(double profit)
    {
        double tax = 0;
        if (profit <= 6000) {
            tax = profit * 0.19;
        } else if (profit <= 50000) {
            tax = 6000 * 0.19 + (profit - 6000) * 0.21;
        } else if (profit <= 200000) {
            tax = 6000 * 0.19 + 44000 * 0.21 + (profit - 50000) * 0.23;
        } else if (profit <= 300000) {
            tax = 6000 * 0.19 + 44000 * 0.21 + 150000 * 0.23 + (profit - 200000) * 0.27;
        } else {
            tax = 6000 * 0.19 + 44000 * 0.21 + 150000 * 0.23 + 100000 * 0.27 + (profit - 300000) * 0.28;
        }
        return tax;
    }


    std::string getOperationName(OperationType type) {
        switch (type) {
            case OperationType::Buy:
                return "Buy";
            case OperationType::Sell:
                return "Sell";
            case OperationType::Deposit:
                return "Deposit";
            case OperationType::Withdraw:
                return "Withdraw";
        }
        return "Unknown Operation";
    }

    // Función getOperationType que reciba un string con el nombre de la operación y devuelva el tipo de operación asociado.
    OperationType getOperationType(const std::string& str) {
        if (str == "Buy") {
            return OperationType::Buy;
        } else if (str == "Sell") {
            return OperationType::Sell;
        } else if (str == "Deposit") {
            return OperationType::Deposit;
        } else if (str == "Withdraw") {
            return OperationType::Withdraw;
        } else {
            std::string msg = "Unknown operation type: " + str;
            throw std::invalid_argument(msg);
        }
    }

    

    

    unsigned short getDecimals(Ticket asset) {
        switch (asset) {
            case Ticket::BTC:
            case Ticket::ETH:
            case Ticket::LTC:
            case Ticket::BCH:
            case Ticket::DASH:
            case Ticket::XRP:
            case Ticket::ADA:
            case Ticket::SOL:
            case Ticket::DOGE:
            case Ticket::SHIB:
            case Ticket::PEPE:
                return 8;
            case Ticket::EUR:
            case Ticket::USD:
            case Ticket::GBP:
            case Ticket::USDT:
            case Ticket::FDUSD:
            default:
                return 2;
        }
    }

    unsigned short getPriceDecimals(Ticket asset, Ticket currency) {
        switch (asset) {
            case Ticket::BTC:
                switch (currency) {
                    case Ticket::EUR:
                    case Ticket::USD:
                    case Ticket::GBP:
                    case Ticket::USDT:
                    case Ticket::FDUSD:
                        return 2;
                    case Ticket::ETH:
                    case Ticket::LTC:
                        return 8;
                }
            case Ticket::ETH:
                switch (currency) {
                    case Ticket::EUR:
                    case Ticket::USD:
                    case Ticket::GBP:
                    case Ticket::USDT:
                        return 2;
                    case Ticket::BTC:
                        return 8;
                }
            case Ticket::LTC:
                switch (currency) {
                    case Ticket::EUR:
                    case Ticket::USD:
                    case Ticket::USDT:
                        return 3;
                    case Ticket::BTC:
                        return 8;
                }
            case Ticket::BCH:
            case Ticket::DASH:
                return 3;
            case Ticket::XRP:
                return 6;
            case Ticket::ADA:
                return 6;
            case Ticket::SOL:
                return 3;
            case Ticket::DOGE:
                return 7;
            case Ticket::SHIB:
            case Ticket::PEPE:
                return 8;
            case Ticket::USDT:
            case Ticket::FDUSD:
            default:
                return 2;
        }
    }

    std::string stringfy(double value, Ticket ticket, bool symbol) {
        unsigned int decimals = getDecimals(ticket);
        std::ostringstream oss;
        // truncate the value to the number of decimals
        value = std::floor(value * std::pow(10, decimals)) / std::pow(10, decimals);
        oss << std::fixed << std::setprecision(decimals) << value;
        if (symbol) {
            oss << " " << getTicketSymbol(ticket);
        }
        return oss.str();
    }


    


    // Implementation of Operation class methods
    Operation::Operation()
        : timePoint_(std::chrono::system_clock::now())
        , id_(ULIDGenerator::next())
    {}

    Operation::Operation(std::chrono::system_clock::time_point timePoint)
        : timePoint_(timePoint)
        , id_(ULIDGenerator::next())
    {}

    std::string Operation::str() const {
        std::time_t timePoint = std::chrono::system_clock::to_time_t(timePoint_);
        std::tm local_tm;
        localtime_s(&local_tm, &timePoint);
        std::ostringstream oss;
        oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S")
              << " - " << std::setw(8) << std::left << getOperationName(getType());
        return oss.str();
    }





    Fee calculateBuyFee(Market market, double total, Ticket referenceCurrency, const TickersInformation& tickersInformation, float multiplier, float increment) {
        Fee fee;
        Ticket currency = getCurrencyType(market);
        if(currency == referenceCurrency) {
            fee.asset = currency;
            fee.quantity = total * 0.0025;
            fee.price = 1;
        } else {
            Market referenceMarket = getMarket(currency, referenceCurrency);
            double price = tickersInformation.getLastPrice(market, multiplier, increment);
            fee.asset = currency;
            fee.quantity = total * 0.0025;
            fee.price = price;
        }
        return fee;
    }

    Fee calculateSellFee(Market market, double quantity, Ticket referenceCurrency, const TickersInformation& tickersInformation, float multiplier, float increment) {
        Fee fee;
        Ticket asset = getAssetType(market);
        Ticket currency = getCurrencyType(market);
        Market referenceMarket = getMarket(asset, referenceCurrency);
        double price = tickersInformation.getLastPrice(market, multiplier, increment);
        fee.asset = asset;
        fee.quantity = quantity * 0.0025;
        fee.price = price;
        return fee;
    }

    double calculateDepositFee(Ticket asset, double price) {
        if(asset == Ticket::EUR || asset == Ticket::USD || asset == Ticket::GBP) {
            return 0;
        }
        if(price == 0) {
           throw std::invalid_argument("Price cannot be zero when calculating fee for a crypto currency");
        }
        // Calculate the fee and round the fee to the most significant decimal, using the getDecimals function
        double fee =  1 / price;
        unsigned int decimals = getDecimals(asset);
        // calculate the necesaary multiplier to set the most significant decimal of the fee value to units level. For example fee=0.00001234 -> multiplier=100000
        double multiplier = std::pow(10, std::ceil(-std::log10(fee)));
        fee = std::round(fee * multiplier) / multiplier;
        return fee;
    }

    Fee calculateDepositFee(Ticket asset, Ticket referenceCurrency, const TickersInformation& tickersInformation, float multiplier, float increment) {
        Fee fee(asset, 0);
        if(asset != Ticket::EUR && asset != Ticket::USD && asset != Ticket::GBP) {
            Market market = getMarket(asset, referenceCurrency);
            double price = tickersInformation.getLastPrice(market, multiplier, increment);
            fee.quantity = calculateDepositFee(asset, price);
            fee.price = price;
        }
        return fee;
    }

    double calculateWithdrawFee(Ticket asset, double price) {
        if(asset == Ticket::EUR || asset == Ticket::USD || asset == Ticket::GBP) {
            return 1;
        }
        if(price == 0) {
           throw std::invalid_argument("Price cannot be zero when calculating fee for a crypto currency");
        }
        // Calculate the fee and round the fee to the most significant decimal, using the getDecimals function
        double fee =  1 / price;
        unsigned int decimals = getDecimals(asset);
        // calculate the necesaary multiplier to set the most significant decimal of the fee value to units level. For example fee=0.00001234 -> multiplier=100000
        double multiplier = std::pow(10, std::ceil(-std::log10(fee)));
        fee = std::round(fee * multiplier) / multiplier;
        return fee;
    }

    Fee calculateWithdrawFee(Ticket asset, Ticket referenceCurrency, const TickersInformation& tickersInformation, float multiplier, float increment) {
        Fee fee(asset, 1);
        if(asset != Ticket::EUR && asset != Ticket::USD && asset != Ticket::GBP) {
            Market market = getMarket(asset, referenceCurrency);
            double price = tickersInformation.getLastPrice(market, multiplier, increment);
            fee.quantity = calculateWithdrawFee(asset, price);
            fee.price = price;
        }
        return fee;
    }

} // namespace exchange
