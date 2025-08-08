#ifndef EXCHANGE_BINANCE_H
#define EXCHANGE_BINANCE_H


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

    const std::map<Market, std::string> BinancePairs = {
        {Market::BTC_EUR, "BTCEUR"},
        {Market::BTC_USDT, "BTCUSDT"},
        {Market::BTC_FDUSD, "BTCFDUSD"},
        {Market::BTC_USDC, "BTCUSDC"},
        {Market::ETH_EUR, "ETHEUR"},
        {Market::ETH_BTC, "ETHBTC"},
        {Market::ETH_USDT, "ETHUSDT"},
        {Market::LTC_EUR, "LTCEUR"},
        {Market::LTC_BTC, "LTCBTC"},
        {Market::LTC_USDT, "LTCUSDT"}
    };

    class Binance {
    public:

        static Response getSystemStatus(SystemStatus& systemStatus);
        static Response getTickersInformation(TickersInformation& tickersInformation, const std::map<Market, std::string>& pairs);
        static Response getOrderBook(OrderBook& orderBook, unsigned int count);
        static Response getRecentTrades(RecentTrades& recentTrades, unsigned int count);

    private:
        static std::string getPairName_(Market market);
        static Market getMarketType_(const std::string& pair);
        static std::string getPairList_(const std::map<Market, std::string>& pairs);
        // TickersInformation
        static void assign_(TickerInformation& tickerInfo, const json& jo);
        // OrderBook
        static void assign_(OrderBook& orderBook, const json& jo, unsigned int count);
        static void assign_(Ask& ask, const json& jask);
        static void assign_(Bid& bid, const json& jbid);
        // RecentTrades
        static void assign_(Trade& trade, const json& jtrade);
    };


} // namespace exchange

#endif // EXCHANGE_BINANCE_H
