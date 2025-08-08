#ifndef EXCHANGE_RECENTTRADES_H
#define EXCHANGE_RECENTTRADES_H


// Stl  headers
#include <string>
#include <chrono>

// exchange headers
#include "exchange/CommonTypes.h"


namespace exchange {

    struct Trade {
        std::chrono::system_clock::time_point timestamp;
        unsigned long long id;
        double price;
        double volume;
        double worth;
        bool buy;

        Trade()
            : timestamp(), id(), price(), volume(), worth(), buy() {}

        Trade(std::chrono::system_clock::time_point timestamp, unsigned long long id, double price, double volume, double worth, bool buy)
            : timestamp(timestamp), id(id), price(price), volume(volume), worth(worth), buy(buy) {}
    };

    struct RecentTradesSummary {
        std::chrono::system_clock::time_point timestamp;
        unsigned long long firstId;
        double averagePrice;
        unsigned int buyers;
        double buyVolume;
        unsigned int sellers;
        double sellVolume;
        double lastPrice;
        double lastVolume;
        unsigned int gap;
        bool joined;

        RecentTradesSummary()
            : timestamp(), firstId(), averagePrice(), buyers(), buyVolume(), sellers(), sellVolume(), lastPrice(), lastVolume(), gap(), joined() {}

        RecentTradesSummary(std::chrono::system_clock::time_point timestamp, unsigned long long firstId, double averagePrice, unsigned int buyers, double buyVolume, unsigned int sellers, double sellVolume, double lastPrice, double lastVolume, unsigned int gap, bool joined)
            : timestamp(timestamp), firstId(firstId), averagePrice(averagePrice), buyers(buyers), buyVolume(buyVolume), sellers(sellers), sellVolume(sellVolume), lastPrice(lastPrice), lastVolume(lastVolume), gap(gap), joined(joined) {}
        
        bool isEmpty() const {
            return (buyers == 0) && (sellers == 0);
        }

        static std::string csvHeader(bool ts = true) {
            const char* header = "AveragePrice,Buyers,BuyVolume,Sellers,SellVolume,Gap,Joined";
            if(ts) {
                return std::string("Timestamp,") + header;
            }
            return header;
        }

        std::string csv(bool ts = true) const {
            std::ostringstream oss;
            if(ts) {
                oss << std::chrono::system_clock::to_time_t(timestamp) << "," ;
            }
            oss << averagePrice
                << "," << buyers
                << "," << buyVolume
                << "," << sellers
                << "," << sellVolume
                << "," << gap
                << "," << joined
            ;
            return oss.str();
        }

        static std::string label() {
            return "RecentTradesSummary";
        }
    };


    class RecentTrades {
    public:
        RecentTrades(exchange::Market market = exchange::Market::BTC_EUR)
            : market_(market)
            , trades_()
            , lastTradeId_()
            , initialTradeId_(0)
            , gap_(0)
        {}

    public:
        exchange::Market getMarket() const {
            return market_;
        }

        void start() {
            trades_.clear();
            joined_ = true;
            initialTradeId_ = 0;
            gap_ = 0;
        }

        void addTrade(const Trade& trade) {
            if(trade.id > lastTradeId_) {
                if(!initialTradeId_) {
                    initialTradeId_ = trade.id;
                    if(lastTradeId_ != 0) {
                        gap_ = static_cast<unsigned int>(trade.id - lastTradeId_ - 1);
                    }
                }
                joined_ = (!trades_.empty() && trade.id == (trades_.back().id + 1));
                trades_.push_back(trade);
                lastTradeId_ = trade.id;
            }
        }

        RecentTradesSummary getSummary() const {
            RecentTradesSummary summary;
            if(!trades_.empty()) {
                summary.timestamp = trades_.back().timestamp;
                summary.firstId = initialTradeId_;
                summary.averagePrice = 0;
                summary.buyers = 0;
                summary.buyVolume = 0;
                summary.sellers = 0;
                summary.sellVolume = 0;
                summary.lastPrice = 0;
                summary.lastVolume = 0;
                summary.gap = gap_;
                summary.joined = joined_;
                for(const auto& trade : trades_) {
                    summary.averagePrice += trade.price;
                    if(trade.buy) {
                        summary.buyers++;
                        summary.buyVolume += trade.volume;
                    } else {
                        summary.sellers++;
                        summary.sellVolume += trade.volume;
                    }
                    summary.lastPrice = trade.price;
                    summary.lastVolume = trade.volume;
                }
                if(!trades_.empty()) {
                    summary.lastPrice = trades_.back().price;
                    summary.lastVolume = trades_.back().volume;
                    summary.averagePrice /= trades_.size();
                }
            }
            return summary;
        }

    private:
        exchange::Market market_;
        std::vector<Trade> trades_;
        unsigned long long lastTradeId_;
        unsigned long long initialTradeId_;
        unsigned int gap_;
        bool joined_;
    };


} // namespace exchange

#endif // EXCHANGE_RECENCTRADES_H
