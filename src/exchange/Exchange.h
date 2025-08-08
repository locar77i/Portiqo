#ifndef EXCHANGE_EXCHANGE_H
#define EXCHANGE_EXCHANGE_H


// Stl  headers

// exchange headers
#include "exchange/CommonTypes.h"
#include "exchange/ProxyTypes.h"
#include "exchange/ProxyStatistics.h"
#include "exchange/Kraken.h"
#include "exchange/Binance.h"
#include "exchange/Coinbase.h"

// json headers
#include <nlohmann/json.hpp>
using json = nlohmann::json;


namespace exchange {

    const std::map<Service, std::map<Market, std::string>> AvailablePairs = {
        {Service::Binance, BinancePairs},
        {Service::Kraken, KrakenPairs},
        {Service::Coinbase, CoinbasePairs}
    };


    struct MarketDataSummary {
        std::chrono::system_clock::time_point timestamp;
        Ticker ticker;
        RecentTradesSummary recentTradesSummary;
        OrderBookSummary orderBookSummary;

        MarketDataSummary() : timestamp(), ticker(), recentTradesSummary(), orderBookSummary() {}
        MarketDataSummary(const Ticker& ticker, const RecentTradesSummary& recentTradesSummary, const OrderBookSummary& orderBookSummary)
            : timestamp(ticker.timestamp)
            , ticker(ticker)
            , recentTradesSummary(recentTradesSummary)
            , orderBookSummary(orderBookSummary) {
            std::cout << "Ticker: " << ticker.timestamp.time_since_epoch().count() << " s" << std::endl;
            std::cout << "RecentTradesSummary: " << recentTradesSummary.timestamp.time_since_epoch().count() << " s" << std::endl;
            std::cout << "OrderBookSummary: " << orderBookSummary.timestamp.time_since_epoch().count() << " s" << std::endl;
        }

        
        static std::string csvHeader() {
            return std::string("Timestamp")
                + "," + Ticker::csvHeader(false)
                + "," + RecentTradesSummary::csvHeader(false)
                + "," + OrderBookSummary::csvHeader(false)
                ;
        }

        std::string csv() const {
            std::ostringstream oss;
            oss << std::chrono::system_clock::to_time_t(timestamp)
                << "," << ticker.csv(false)
                << "," << recentTradesSummary.csv(false)
                << "," << orderBookSummary.csv(false)
            ;
            return oss.str();
        }

        static std::string label() {
            return "MarketDataSummary";
        }
    };

} // namespace exchange

#endif // EXCHANGE_EXCHANGE_H
