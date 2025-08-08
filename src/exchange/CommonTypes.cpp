// Header
#include "CommonTypes.h"

// cURL headers
#include <curl/curl.h>

// Stl headers
#include <iostream>


namespace exchange {

    std::string getStatusName(Status status) {
        switch (status) {
            case Status::Online:
                return "Online";
            case Status::Offline:
                return "Offline";
            case Status::Error:
                return "Invalid status";
            //case Status::Unknown:
            default:
                return "Unknown status";
        }
    }


    std::string getServiceName(Service service) {
        auto it = AvailableServices.find(service);
        if(it != AvailableServices.end()) {
            return it->second;
        }
        return AvailableServices.at(Service::Unknown);
    }

    Service getServiceType(const std::string& name) {
        for(const auto& pair : AvailableServices) {
            if(pair.second == name) {
                return pair.first;
            }
        }
        return Service::Unknown;
    }



    std::string getTicketSymbol(Ticket ticket) {
        switch (ticket) {
        case Ticket::EUR:
            return "€";
        case Ticket::USD:
            return "$";
        case Ticket::GBP:
            return "£";
        case Ticket::BTC:
            return "₿";
        case Ticket::ETH:
            return "Ξ";
        case Ticket::LTC:
            return "Ł";
        case Ticket::BCH:
            return "₿";
        case Ticket::DASH:
            return "D";
        case Ticket::XRP:
            return "Ʀ";
        case Ticket::ADA:
            return "₳";
        case Ticket::SOL:
            return "◎";
        case Ticket::DOGE:
            return "Ð";
        case Ticket::SHIB:
            return "🐕";
        case Ticket::PEPE:
            return "🐸";
        case Ticket::USDT:
            return "₮";
        case Ticket::FDUSD:
            return "$F";
        case Ticket::USDC:
            return "$C";
        }
        return "--";
    }
    

    std::string getTicketName(Ticket ticket)
    {
        for(const auto& pair : AvailableTickets) {
            if(pair.first == ticket) {
                return pair.second;
            }
        }
        return AvailableTickets.at(Ticket::Unknown);
    }

    Ticket getTicketType(const std::string& str)
    {
        for(const auto& pair : AvailableTickets) {
            if(pair.second == str) {
                return pair.first;
            }
        }
        return Ticket::Unknown;
    }



    Market getMarket(Ticket asset, Ticket currency) {
        std::string assetName = getTicketName(asset);
        std::string currencyName = getTicketName(currency);
        std::string marketName = assetName + "." + currencyName;
        return getMarketType(marketName);
    }

    std::string getMarketName(Market market) {
        auto it = AvailableMarkets.find(market);
        if (it != AvailableMarkets.end()) {
            return it->second;
        }
        return AvailableMarkets.at(Market::Unknown);
    }


    // Función getMarketType que reciba un string con el nombre del mercado y devuelva el tipo de Market asociado.
    Market getMarketType(const std::string& str) {
        auto it = MarketTypes.find(str);
        if (it != MarketTypes.end()) {
            return it->second;
        }
        return Market::Unknown;
    }

    std::string getAssetNameFrom(Market market) {
        std::string marketName = getMarketName(market);
        return marketName.substr(0, marketName.find_first_of('.')); // Extract the asset name from the market name
    }

    std::string getCurrencyNameFrom(Market market) {
        std::string marketName = getMarketName(market);
        return marketName.substr(marketName.find_first_of('.') + 1); // Extract the currency name from the market name
    }

    // Función getAssetType que reciba un mercado y devuelva el tipo de activo asociado.
    Ticket getAssetType(Market market) {
        auto it = TicketTypes.find(getAssetNameFrom(market));
        if (it != TicketTypes.end()) {
            return it->second;
        }
        return Ticket::Unknown;
    }

    // Función getCurrencyType que reciba un mercado y devuelva el tipo de Currency asociado.
    Ticket getCurrencyType(Market market) {
        auto it = TicketTypes.find(getCurrencyNameFrom(market));
        if (it != TicketTypes.end()) {
            return it->second;
        }
        return Ticket::Unknown;
    }

} // namespace exchange
