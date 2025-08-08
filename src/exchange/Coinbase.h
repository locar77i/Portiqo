#ifndef EXCHANGE_COINBASE_H
#define EXCHANGE_COINBASE_H


// Stl  headers
#include <string>
#include <chrono>
#include <vector>

// exchange headers
#include "exchange/CommonTypes.h"
#include "exchange/OrderBook.h"
#include "exchange/RecentTrades.h"

// json headers
#include <nlohmann/json.hpp>
using json = nlohmann::json;


namespace exchange {

    const std::map<Market, std::string> CoinbasePairs = {
        {Market::BTC_EUR, "BTC-EUR"},
        {Market::BTC_USD, "BTC-USD"},
        {Market::BTC_USDT, "BTC-USDT"},
        {Market::ETH_EUR, "ETH-EUR"},
        {Market::ETH_USD, "ETH-USD"},
        {Market::ETH_BTC, "ETH-BTC"},
        {Market::LTC_EUR, "LTC-EUR"},
        {Market::LTC_USD, "LTC-USD"},
        {Market::LTC_BTC, "LTC-BTC"}
    };

    class Coinbase {
    public:

        static Response getSystemStatus(SystemStatus& systemStatus);
        static Response getTicker(Ticker& ticker, const std::pair<Market, std::string>& pair);
        static Response getOrderBook(OrderBook& orderBook, unsigned int count);
        static Response getRecentTrades(RecentTrades& recentTrades, unsigned int count);

    private:
        static std::string getPairName_(Market market);
        static std::string getPairList_(const std::vector<Market>& markets);
        // TickersInformation
        static void assign_(Ticker& ticker, const json& jo);
        // OrderBook
        static void assign_(OrderBook& orderBook, const json& jo, unsigned int count);
        static void assign_(Ask& ask, const json& jask);
        static void assign_(Bid& bid, const json& jbid);
        // RecentTrades
        static void assign_(Trade& trade, const json& jtrade);
    };


} // namespace exchange

#endif // EXCHANGE_COINBASE_H
