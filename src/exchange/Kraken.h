#ifndef EXCHANGE_KRAKEN_H
#define EXCHANGE_KRAKEN_H


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

    const std::map<Market, std::string> KrakenPairs = {
        {Market::BTC_EUR, "XXBTZEUR"},
        {Market::ETH_EUR, "XETHZEUR"},
        {Market::LTC_EUR, "XLTCZEUR"},
        {Market::BCH_EUR, "BCHEUR"},
        {Market::DASH_EUR, "DASHEUR"},
        {Market::XRP_EUR, "XXRPZEUR"},
        {Market::ADA_EUR, "ADAEUR"},
        {Market::SOL_EUR, "SOLEUR"},
        {Market::DOGE_EUR, "XDGEUR"},
        {Market::SHIB_EUR, "SHIBEUR"},
        {Market::PEPE_EUR, "PEPEEUR"},
        {Market::USDT_EUR, "USDTEUR"},
        {Market::BTC_USD, "XXBTZUSD"},
        {Market::ETH_USD, "XETHZUSD"},
        {Market::LTC_USD, "XLTCZUSD"},
        {Market::BCH_USD, "BCHUSD"},
        {Market::DASH_USD, "DASHUSD"},
        {Market::XRP_USD, "XXRPZUSD"},
        {Market::ADA_USD, "ADAUSD"},
        {Market::SOL_USD, "SOLUSD"},
        {Market::DOGE_USD, "XDGUSD"},
        {Market::SHIB_USD, "SHIBUSD"},
        {Market::PEPE_USD, "PEPEUSD"},
        {Market::USDT_USD, "USDTZUSD"},
        {Market::BTC_GBP, "XXBTZGBP"},
        {Market::ETH_GBP, "XETHZGBP"},
        {Market::BTC_USDT, "XXBTZUSD"},
        {Market::ETH_BTC, "XETHXXBT"},
        {Market::ETH_USDT, "ETHUSDT"},
        {Market::LTC_BTC, "XLTCXXBT"},
        {Market::LTC_USDT, "LTCUSDT"}
    };

    class Kraken {
    public:
        static Response getSystemStatus(SystemStatus& systemStatus);
        static Response getTickersInformation(TickersInformation& tickersInformation, const std::map<Market, std::string>& pairs);
        static Response getOrderBook(OrderBook& orderBook, unsigned int count);
        static Response getRecentTrades(RecentTrades& recentTrades, unsigned int count);
        static Response getOhlcData(OhlcData& ohlcData, unsigned int interval, unsigned int since = 0);

        static std::string getPairName(Market market);

    private:
        static Status getStatus_(const std::string& str);
        static std::chrono::system_clock::time_point getTimePoint_(const std::string& str);
        static SystemStatus getSystemStatus_(const std::string& jstr);
        static std::string getPairList_(const std::map<Market, std::string>& pairs);
        // TickerInformation
        static void assign_(TickerInformation& tickerInfo, const json& jo);
        // OrderBook
        static void assign_(OrderBook& orderBook, const json& jo, unsigned int count);
        static void assign_(Ask& ask, const json& jask);
        static void assign_(Bid& bid, const json& jbid);
        // RecentTrades
        static void assign_(Trade& trade, const json& jtrade);
        // OhlcData
        static void assign_(OhlcData& ohlcData, const json& result, const std::string& pairName);
        static void assign_(OhlcTick& ohlcTick, const json& tick);
    };


} // namespace exchange

#endif // EXCHANGE_KRAKEN_H
