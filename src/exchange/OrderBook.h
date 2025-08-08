#ifndef EXCHANGE_ORDERBOOK_H
#define EXCHANGE_ORDERBOOK_H


// Stl  headers
#include <string>
#include <chrono>
#include <vector>
#include <algorithm>

// exchange headers
#include "exchange/CommonTypes.h"

// json headers
#include <nlohmann/json.hpp>
using json = nlohmann::json;


namespace exchange {


    struct Ask {
        double price;
        double volume;

        Ask() : price(), volume() {}
        Ask(double price, double volume)
            : price(price), volume(volume) {}
    };

    struct Bid {
        double price;
        double volume;

        Bid() : price(), volume() {}
        Bid(double price, double volume)
            : price(price), volume(volume) {}
    };





    struct OrderBookSummary {
        std::chrono::system_clock::time_point timestamp;
        unsigned int askCount;
        double askVolume;
        double askWorth;
        double averageAskPrice;
        unsigned int bidCount;
        double bidVolume;
        double bidWorth;
        double averageBidPrice;
        double spread;
        double spreadPercentage;
        unsigned int lastAskCount;
        double lastAskVolume;
        double lastAskWorth;
        double lastAverageAskPrice;
        unsigned int lastBidCount;
        double lastBidVolume;
        double lastBidWorth;
        double lastAverageBidPrice;
        double lastSpread;
        double lastSpreadPercentage;

        OrderBookSummary() : timestamp()
            , askCount(), askVolume(), askWorth(), averageAskPrice(), bidCount(), bidVolume(), bidWorth(), averageBidPrice(), spread(), spreadPercentage()
            , lastAskCount(), lastAskVolume(), lastAskWorth(), lastAverageAskPrice(), lastBidCount(), lastBidVolume(), lastBidWorth(), lastAverageBidPrice(), lastSpread(), lastSpreadPercentage()
            {}
        OrderBookSummary(unsigned int askCount, double askVolume, double askWorth, unsigned int bidCount, double bidVolume, double bidWorth
                       , unsigned int lastAskCount, double lastAskVolume, double lastAskWorth, unsigned int lastBidCount, double lastBidVolume, double lastBidWorth)
            : timestamp(std::chrono::system_clock::now())
            , askCount(askCount), bidCount(bidCount), askVolume(askVolume), askWorth(askWorth), bidVolume(bidVolume), bidWorth(bidWorth)
            , lastAskCount(lastAskCount), lastBidCount(lastBidCount), lastAskVolume(lastAskVolume), lastAskWorth(lastAskWorth), lastBidVolume(lastBidVolume), lastBidWorth(lastBidWorth) {
                averageAskPrice = askWorth / askVolume;
                averageBidPrice = bidWorth / bidVolume;
                spread = averageAskPrice - averageBidPrice;
                spreadPercentage = (spread / averageBidPrice) * 100;

                lastAverageAskPrice = lastAskWorth / lastAskVolume;
                lastAverageBidPrice = lastBidWorth / lastBidVolume;
                lastSpread = lastAverageAskPrice - lastAverageBidPrice;
                lastSpreadPercentage = (lastSpread / lastAverageBidPrice) * 100;
            }

        static std::string csvHeader(bool ts = true) {
            const char* header = "AskCount,AskVolume,AskWorth,AverageAskPrice,BidCount,BidVolume,BidWorth,AverageBidPrice,Spread,SpreadPercentage,LastAskCount,LastAskVolume,LastAskWorth,LastAverageAskPrice,LastBidCount,LastBidVolume,LastBidWorth,LastAverageBidPrice,LastSpread,LastSpreadPercentage";
            if(ts) {
                return std::string("Timestamp,") + header;
            }
            return header;
        }

        std::string csv(bool ts = true) const {
            std::ostringstream oss;
            if(ts) {
                oss << std::chrono::system_clock::to_time_t(timestamp) << ",";
            }
            oss << askCount
                << "," << askVolume
                << "," << askWorth
                << "," << averageAskPrice
                << "," << bidCount
                << "," << bidVolume
                << "," << bidWorth
                << "," << averageBidPrice
                << "," << spread
                << "," << spreadPercentage
                << "," << lastAskCount
                << "," << lastAskVolume
                << "," << lastAskWorth
                << "," << lastAverageAskPrice
                << "," << lastBidCount
                << "," << lastBidVolume
                << "," << lastBidWorth
                << "," << lastAverageBidPrice
                << "," << lastSpread
                << "," << lastSpreadPercentage
            ;
            return oss.str();
        }

        static std::string label() {
            return "OrderBookSummary";
        }
    };



    class OrderBook {
    public:
        OrderBook(exchange::Market market = exchange::Market::BTC_EUR) : market_(market) {}
        OrderBook(exchange::Market market, const std::vector<Ask>& asks, const std::vector<Bid>& bids)
            : market_(market), asks_(asks), bids_(bids) {}

    public:
        exchange::Market getMarket() const {
            return market_;
        }

        inline void addAsk(const Ask& ask) {
            asks_.push_back(ask);
        }

        inline void addBid(const Bid& bid) {
            bids_.push_back(bid);
        }

        inline const std::vector<Ask>& getAsks() const {
            return asks_;
        }

        inline const std::vector<Bid>& getBids() const {
            return bids_;
        }

        OrderBookSummary getSummary() const {
            if(asks_.empty() || bids_.empty()) {
                return OrderBookSummary();
            }
            unsigned int askCount = static_cast<unsigned int>(asks_.size());
            double totalAskVolume = 0;
            double totalAskWorth = 0;
            for(const auto& ask : asks_) {
                totalAskVolume += ask.volume;
                totalAskWorth += ask.price * ask.volume;
            }
            unsigned int lastAskCount = askCount / 5;
            double lastAskVolume = 0;
            double lastAskWorth = 0;
            for(unsigned int ii = 0; ii < lastAskCount; ++ii) {
                lastAskVolume += asks_[ii].volume;
                lastAskWorth += asks_[ii].price * asks_[ii].volume;
            }
            unsigned int bidCount = static_cast<unsigned int>(bids_.size());
            double totalBidVolume = 0;
            double totalBidWorth = 0;
            for(const auto& bid : bids_) {
                totalBidVolume += bid.volume;
                totalBidWorth += bid.price * bid.volume;
            }
            unsigned int lastBidCount = bidCount / 5;
            double lastBidVolume = 0;
            double lastBidWorth = 0;
            for(unsigned int ii = 0; ii < lastBidCount; ++ii) {
                lastBidVolume += bids_[ii].volume;
                lastBidWorth += bids_[ii].price * bids_[ii].volume;
            }
            return OrderBookSummary(askCount, totalAskVolume, totalAskWorth, bidCount, totalBidVolume, totalBidWorth, lastAskCount, lastAskVolume, lastAskWorth, lastBidCount, lastBidVolume, lastBidWorth);
        }

    private:
        exchange::Market market_;
        std::vector<Ask> asks_;
        std::vector<Bid> bids_;
    };


} // namespace exchange

#endif // EXCHANGE_ORDERBOOK_H
