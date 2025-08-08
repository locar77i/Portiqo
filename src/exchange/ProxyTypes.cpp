// Header
#include "ProxyTypes.h"

// Stl  headers
#include <string>
#include <chrono>
#include <iostream>

// Qt headers

// exchange headers


namespace exchange {

    std::string getProxyTypeName(ProxyType type) {
        switch(type) {
            case ProxyType::SystemStatus:
                return "SystemStatus";
            case ProxyType::TickersInformation:
                return "TickersInformation";
            case ProxyType::OrderBook:
                return "OrderBook";
            case ProxyType::RecentTrades:
                return "RecentTrades";
            default:
                return "UnknownProxy";
        }
    }

    
} // namespace exchange

