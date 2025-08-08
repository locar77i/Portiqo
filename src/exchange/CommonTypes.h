#ifndef EXCHANGE_COMMONTYPES_H
#define EXCHANGE_COMMONTYPES_H


// Stl  headers
#include <string>
#include <chrono>
#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

// headers
#include "Utils.h"

// json headers
#include <nlohmann/json.hpp>
using json = nlohmann::json;


namespace exchange {

    enum class Ticket {
        EUR,
        USD,
        GBP,
        BTC,
        ETH,
        LTC,
        BCH,
        DASH,
        XRP,
        ADA,
        SOL,
        DOGE,
        SHIB,
        PEPE,
        USDT,
        FDUSD,
        USDC,
        Unknown
    };


    const std::map<Ticket, std::string> AvailableTickets = {
        {Ticket::EUR, "EUR"},
        {Ticket::USD, "USD"},
        {Ticket::GBP, "GBP"},
        {Ticket::BTC, "BTC"},
        {Ticket::ETH, "ETH"},
        {Ticket::LTC, "LTC"},
        {Ticket::BCH, "BCH"},
        {Ticket::DASH, "DASH"},
        {Ticket::XRP, "XRP"},
        {Ticket::ADA, "ADA"},
        {Ticket::SOL, "SOL"},
        {Ticket::DOGE, "DOGE"},
        {Ticket::SHIB, "SHIB"},
        {Ticket::PEPE, "PEPE"},
        {Ticket::USDT, "USDT"},
        {Ticket::FDUSD, "FDUSD"},
        {Ticket::USDC, "USDC"},
        {Ticket::Unknown, "Unknown.ticket"}
    };

    const std::map<std::string, Ticket> TicketTypes = {
        {"EUR", Ticket::EUR},
        {"USD", Ticket::USD},
        {"GBP", Ticket::GBP},
        {"BTC", Ticket::BTC},
        {"ETH", Ticket::ETH},
        {"LTC", Ticket::LTC},
        {"BCH", Ticket::BCH},
        {"DASH", Ticket::DASH},
        {"XRP", Ticket::XRP},
        {"ADA", Ticket::ADA},
        {"SOL", Ticket::SOL},
        {"DOGE", Ticket::DOGE},
        {"SHIB", Ticket::SHIB},
        {"PEPE", Ticket::PEPE},
        {"USDT", Ticket::USDT},
        {"FDUSD", Ticket::FDUSD},
        {"USDC", Ticket::USDC}
    };


    const std::vector<Ticket> CurrencyTickets = {
        Ticket::EUR,
        Ticket::USD,
        Ticket::GBP
    };

    std::string getTicketSymbol(Ticket ticket);
    std::string getTicketName(Ticket ticket);
    Ticket getTicketType(const std::string& str);



    enum class Market {
        BTC_EUR,
        ETH_EUR,
        LTC_EUR,
        BCH_EUR,
        DASH_EUR,
        XRP_EUR,
        ADA_EUR,
        SOL_EUR,
        DOGE_EUR,
        SHIB_EUR,
        PEPE_EUR,
        USDT_EUR,
        BTC_USD,
        ETH_USD,
        LTC_USD,
        BCH_USD,
        DASH_USD,
        XRP_USD,
        ADA_USD,
        SOL_USD,
        DOGE_USD,
        SHIB_USD,
        PEPE_USD,
        USDT_USD,
        BTC_GBP,
        ETH_GBP,
        BTC_USDT,
        BTC_FDUSD,
        BTC_USDC,
        ETH_BTC,
        ETH_USDT,
        ETH_FDUSD,
        ETH_USDC,
        LTC_BTC,
        LTC_USDT,
        LTC_FDUSD,
        LTC_USDC,
        Unknown
    };


    const std::map<Market, std::string> AvailableMarkets = {
        {Market::BTC_EUR, "BTC.EUR"},
        {Market::ETH_EUR, "ETH.EUR"},
        {Market::LTC_EUR, "LTC.EUR"},
        {Market::BCH_EUR, "BCH.EUR"},
        {Market::DASH_EUR, "DASH.EUR"},
        {Market::XRP_EUR, "XRP.EUR"},
        {Market::ADA_EUR, "ADA.EUR"},
        {Market::SOL_EUR, "SOL.EUR"},
        {Market::DOGE_EUR, "DOGE.EUR"},
        {Market::SHIB_EUR, "SHIB.EUR"},
        {Market::PEPE_EUR, "PEPE.EUR"},
        {Market::USDT_EUR, "USDT.EUR"},
        {Market::BTC_USD, "BTC.USD"},
        {Market::ETH_USD, "ETH.USD"},
        {Market::LTC_USD, "LTC.USD"},
        {Market::BCH_USD, "BCH.USD"},
        {Market::DASH_USD, "DASH.USD"},
        {Market::XRP_USD, "XRP.USD"},
        {Market::ADA_USD, "ADA.USD"},
        {Market::SOL_USD, "SOL.USD"},
        {Market::DOGE_USD, "DOGE.USD"},
        {Market::SHIB_USD, "SHIB.USD"},
        {Market::PEPE_USD, "PEPE.USD"},
        {Market::USDT_USD, "USDT.USD"},
        {Market::BTC_GBP, "BTC.GBP"},
        {Market::ETH_GBP, "ETH.GBP"},
        {Market::BTC_USDT, "BTC.USDT"},
        {Market::BTC_FDUSD, "BTC.FDUSD"},
        {Market::BTC_USDC, "BTC.USDC"},
        {Market::ETH_BTC, "ETH.BTC"},
        {Market::ETH_USDT, "ETH.USDT"},
        {Market::ETH_FDUSD, "ETH.FDUSD"},
        {Market::ETH_USDC, "ETH.USDC"},
        {Market::LTC_BTC, "LTC.BTC"},
        {Market::LTC_USDT, "LTC.USDT"},
        {Market::LTC_FDUSD, "LTC.FDUSD"},
        {Market::LTC_USDC, "LTC.USDC"},
        {Market::Unknown, "Unknown.market"}
    };

    const std::map<std::string, Market> MarketTypes = {
        {"BTC.EUR", Market::BTC_EUR},
        {"ETH.EUR", Market::ETH_EUR},
        {"LTC.EUR", Market::LTC_EUR},
        {"BCH.EUR", Market::BCH_EUR},
        {"DASH.EUR", Market::DASH_EUR},
        {"XRP.EUR", Market::XRP_EUR},
        {"ADA.EUR", Market::ADA_EUR},
        {"SOL.EUR", Market::SOL_EUR},
        {"DOGE.EUR", Market::DOGE_EUR},
        {"SHIB.EUR", Market::SHIB_EUR},
        {"PEPE.EUR", Market::PEPE_EUR},
        {"USDT.EUR", Market::USDT_EUR},
        {"BTC.USD", Market::BTC_USD},
        {"ETH.USD", Market::ETH_USD},
        {"LTC.USD", Market::LTC_USD},
        {"BCH.USD", Market::BCH_USD},
        {"DASH.USD", Market::DASH_USD},
        {"XRP.USD", Market::XRP_USD},
        {"ADA.USD", Market::ADA_USD},
        {"SOL.USD", Market::SOL_USD},
        {"DOGE.USD", Market::DOGE_USD},
        {"SHIB.USD", Market::SHIB_USD},
        {"PEPE.USD", Market::PEPE_USD},
        {"USDT.USD", Market::USDT_USD},
        {"BTC.GBP", Market::BTC_GBP},
        {"ETH.GBP", Market::ETH_GBP},
        {"BTC.USDT", Market::BTC_USDT},
        {"BTC.FDUSD", Market::BTC_FDUSD},
        {"BTC.USDC", Market::BTC_USDC},
        {"ETH.BTC", Market::ETH_BTC},
        {"ETH.USDT", Market::ETH_USDT},
        {"ETH.FDUSD", Market::ETH_FDUSD},
        {"ETH.USDC", Market::ETH_USDC},
        {"LTC.BTC", Market::LTC_BTC},
        {"LTC.USDT", Market::LTC_USDT},
        {"LTC.FDUSD", Market::LTC_FDUSD},
        {"LTC.USDC", Market::LTC_USDC}
    };


    std::string getMarketName(Market market);
    Market getMarketType(const std::string& str);
    Ticket getAssetType(Market market);
    Ticket getCurrencyType(Market market);
    Market getMarket(Ticket asset, Ticket currency);


    enum class Response {
        Ok,
        Warning,
        Error,
        Stop,
        Banned,
        Unknown
    };

    enum class ProxyStatus {
        Running,
        SlowedDown,
        SpeededUp,
        Paused,
        Unknown
    };


    enum class Status {
        Online,
        Offline,
        Error,
        Unknown
    };


    std::string getStatusName(Status status);


    // enum Service
    enum class Service {
        Binance,
        Bybit,
        Coinbase,
        OKX,
        Bitget,
        Kraken,
        KuCoin,
        MEXC,
        Bitfinex,
        Gate_io,
        Unknown
    };

    const std::map<Service, std::string> AvailableServices = {
        {Service::Binance, "Binance"},
        {Service::Bybit, "Bybit"},
        {Service::Coinbase, "Coinbase"},
        {Service::OKX, "OKX"},
        {Service::Bitget, "Bitget"},
        {Service::Kraken, "Kraken"},
        {Service::KuCoin, "KuCoin"},
        {Service::MEXC, "MEXC"},
        {Service::Bitfinex, "Bitfinex"},
        {Service::Gate_io, "Gate.io"},
        {Service::Unknown, "Unknown.service"}
    };

    std::string getServiceName(Service service);
    Service getServiceType(const std::string& name);


    // struct SystemStatus
    struct SystemStatus {
        Status value;
        std::chrono::system_clock::time_point timestamp;
        std::vector<std::string> errors;

        SystemStatus() : value(Status::Unknown), timestamp(), errors() {}
        SystemStatus(Status value, std::chrono::system_clock::time_point timestamp, const std::vector<std::string>& errors)
            : value(value), timestamp(timestamp), errors(errors) {}

        std::string description() const {
            std::ostringstream oss;
            oss << "Status: " << getStatusName(value);
            if(value == Status::Online || value == Status::Offline) {
                std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
                std::tm local_tm;
                localtime_s(&local_tm, &time);
                oss << " - last update on " << std::put_time(&local_tm, "%A %d of %B at %H:%M") << "  (Server time)";
            }
            return oss.str();
        }
    };

    struct Ticker {
        std::chrono::system_clock::time_point timestamp;
        double ask;
        double bid;
        double last;
        double lastLotVolume;
        double volume;

        Ticker() : timestamp(std::chrono::system_clock::now()), ask(), bid(), last(), lastLotVolume(), volume() {}
        Ticker(double ask, double bid, double last, double lastLotVolume, double volume)
            : timestamp(std::chrono::system_clock::now()), ask(ask), bid(bid), last(last), lastLotVolume(lastLotVolume), volume(volume) {}

        static std::string csvHeader(bool ts = true) {
            const char* header = "Ask,Bid,Last,LastLotVolume,Volume";
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
            oss << ask
                << "," << bid
                << "," << last
                << "," << lastLotVolume
                << "," << volume
                ;
            return oss.str();
        }

        static std::string label() {
            return "Ticker";
        }
    };

    struct TickerInformation : public Ticker {
        Ticket asset;
        Ticket currency;
        double volumeWeightedAverage;
        unsigned int trades;
        double low;
        double high;
        double openingPrice;

        TickerInformation()
            : Ticker(), asset(Ticket::BTC), currency(Ticket::EUR)
            , volumeWeightedAverage(), trades(), low(), high(), openingPrice() {}

        TickerInformation(const Ticker ticker)
            : Ticker(ticker), asset(Ticket::BTC), currency(Ticket::EUR)
            , volumeWeightedAverage(), trades(), low(), high(), openingPrice() {}

        static std::string csvHeader() {
            return Ticker::csvHeader() + ",VolumeWeightedAverage,Trades,Low,High,OpeningPrice";
        }

        std::string csv() const {
            std::ostringstream oss;
            oss << Ticker::csv()
                << "," << volumeWeightedAverage
                << "," << trades
                << "," << low
                << "," << high
                << "," << openingPrice
            ;
            return oss.str();
        }

        static std::string label() {
            return "TickerInfo";
        }
    };


    class TickersInformation {
    public:
        TickersInformation() {}
        TickersInformation(const std::map<Market, TickerInformation>& tickersInformation) : tickersInformation_(tickersInformation) {}

        inline bool isEmpty() const {
            return tickersInformation_.empty();
        }

        inline void add(Market market, const TickerInformation& tickerInfo) {
            tickersInformation_[market] = tickerInfo;
        }

        inline const TickerInformation& get(Market market) {
            return tickersInformation_[market];
        }

        inline const std::map<Market, TickerInformation>& get() const {
            return tickersInformation_;
        }

    public:
        double getLastPrice(Market market) const {
            auto it = tickersInformation_.find(market);
            if(it != tickersInformation_.end()) {
                return it->second.last;
            }
            return 0;
        }

        double getLastPrice(Market market, float multiplier, float increment) const {
            auto it = tickersInformation_.find(market);
            if(it != tickersInformation_.end()) {
                return it->second.last * (multiplier + increment);
            }
            return 0;
        }

    private:
        std::map<Market, TickerInformation> tickersInformation_;
    };




    template <typename T>
    class RealTimeDataManager {
        public:
            RealTimeDataManager(Service service, std::filesystem::path path, unsigned int capacity=20)
                : path_(path.string() + "/" + getServiceName(service) + "/" + T::label())
                , service_(service)
                , datamap_()
                , capacity_(capacity) {
                if (!std::filesystem::exists(path_)) {
                    std::filesystem::create_directories(path_);
                }
            }
    
            ~RealTimeDataManager() {
                saveAll();
            }

        public:
            void addData(Market market, const T& data) {
                auto it = datamap_.find(market);
                if(it != datamap_.end()) {
                    std::queue<T>& queue = it->second;
                    auto lastDataTimestamp = queue.back().timestamp;
                    queue.push(data);
                    if(queue.size() == capacity_ || isDifferentDay_(lastDataTimestamp, data.timestamp)) {
                        save(market);
                    }
                } else {
                    std::queue<T> queue;
                    queue.push(data);
                    datamap_[market] = queue;
                }
            }

            const T& getLastData(Market market) {
                auto it = datamap_.find(market);
                if(it != datamap_.end()) {
                    return it->second.back();
                }
                throw std::runtime_error("No data found for market " + getMarketName(market));
            }

            void save(Market market, bool flush=false) {
                auto it = datamap_.find(market);
                if(it != datamap_.end()) {
                    std::queue<T>& queue = it->second;
                    std::string filename = toString(queue.front().timestamp, "%Y%m%d") + ".csv";
                    std::string filepath = path_.string() + "/" + getMarketName(market) + "/" + filename;
                    std::filesystem::path fsp(filepath);
                    if(!std::filesystem::exists(fsp.parent_path())) {
                        std::filesystem::create_directories(fsp.parent_path());
                    }
                    if(std::filesystem::exists(fsp)) {
                        //std::cout << "Adding ticker data to file: " << filepath << std::endl;
                        append_(queue, filepath, flush);
                    } else {
                        std::cout << "Saving " << T::label() << " data to file: " << filepath << std::endl;
                        save_(queue, filepath, flush);
                    }
                }
            }

            void saveAll() {
                for(const auto& pair : datamap_) {
                    std::cout << "Saving " << T::label() << " data for market " << getMarketName(pair.first) << " ..." << std::endl;
                    save(pair.first, true);
                }
            }

        private:
            void save_(std::queue<T>& queue, const std::string& filepath, bool flush) {
                // Save the information on a csv file
                std::ofstream file(filepath);
                if (file.is_open()) {
                    file << T::csvHeader() << std::endl;
                    while(queue.size() > 1 || flush && !queue.empty()) {
                        const T& data = queue.front();
                        file << data.csv() << std::endl;
                        queue.pop();
                    }
                    file.close();
                } else {
                    std::string error = "Unable to open " + T::label() + " data file: " + filepath;
                    throw std::runtime_error(error.c_str());
                }
            }

            void append_(std::queue<T>& queue, const std::string& filepath, bool flush) {
                // Append the information on a csv file
                std::ofstream file(filepath, std::ios::app);
                if (file.is_open()) {
                    while(queue.size() > 1 || flush && !queue.empty()) {
                        const T& data = queue.front();
                        file << data.csv() << std::endl;
                        queue.pop();
                    }
                    file.close();
                } else {
                    std::string error = "Unable to open " + T::label() + " data file: " + filepath;
                    throw std::runtime_error(error.c_str());
                }
            }

            // Function to extract the date part from a time_point
            std::tm extractDate_(const std::chrono::system_clock::time_point& tp) const {
                std::time_t time = std::chrono::system_clock::to_time_t(tp);
                std::tm tm;
                localtime_s(&tm, &time);
                return tm;
            }

            // Function to check if two time_points are on different days
            bool isDifferentDay_(const std::chrono::system_clock::time_point& tp1, const std::chrono::system_clock::time_point& tp2) const {
                std::tm date1 = extractDate_(tp1);
                std::tm date2 = extractDate_(tp2);
                return (date1.tm_year != date2.tm_year) || (date1.tm_mon != date2.tm_mon) || (date1.tm_mday != date2.tm_mday);
            }

        private:
            std::filesystem::path path_;
            Service service_;
            std::map<Market, std::queue<T>> datamap_;
            unsigned int capacity_;
    };


    struct OhlcDataSummary {
        unsigned int count;
        double last;
        double high;
        double low;
        double volume;
        std::chrono::system_clock::time_point start;
        std::chrono::system_clock::time_point end;
        std::chrono::system_clock::time_point lastTimestamp;

        OhlcDataSummary() : count(), last(), high(), low(), volume(), start(), end(), lastTimestamp() {}
        OhlcDataSummary(unsigned int count, double last, double high, double low, double volume, std::chrono::system_clock::time_point start, std::chrono::system_clock::time_point end, std::chrono::system_clock::time_point lastTimestamp)
            : count(count), last(last), high(high), low(low), volume(volume), start(start), end(end), lastTimestamp(lastTimestamp) {}

        std::string description() const {
            std::ostringstream oss;
            oss << " - Count: " << count;
            oss << " - Last: " << last;
            oss << " - High: " << high;
            oss << " - Low: " << low;
            oss << " - Volume: " << volume;
            oss << " - Start: " << toString(start);
            oss << " - End: " << toString(end);
            oss << " - Last Timestamp: " << toString(lastTimestamp);
            return oss.str();
        }
    };


    struct OhlcTick {
        std::chrono::system_clock::time_point timestamp;
        double open;
        double high;
        double low;
        double close;
        double volumeWeightedAverage;
        double volume;
        unsigned int count;

        OhlcTick() : timestamp(), open(), high(), low(), close(), volume(), volumeWeightedAverage(), count() {}
        OhlcTick(std::chrono::system_clock::time_point timestamp, double open, double high, double low, double close, double volume, double volumeWeightedAverage, unsigned int count)
            : timestamp(timestamp), open(open), high(high), low(low), close(close), volume(volume), volumeWeightedAverage(volumeWeightedAverage), count(count) {}
    };


    class OhlcData {
    public:
        OhlcData(exchange::Market market = exchange::Market::BTC_EUR) : market_(market) {}
        OhlcData(exchange::Market market, const std::vector<OhlcTick>& ticks)
            : market_(market), ticks_(ticks) {}

    public:
        exchange::Market getMarket() const {
            return market_;
        }

        inline void addTick(const OhlcTick& tick) {
            ticks_.push_back(tick);
        }

        inline const std::vector<OhlcTick>& getTicks() const {
            return ticks_;
        }

        inline void setLastTimestamp(std::chrono::system_clock::time_point last) {
            last_ = last;
        }

        inline std::chrono::system_clock::time_point getLastTimestamp() const {
            return last_;
        }

        OhlcDataSummary getSummary() const {
            if(ticks_.empty()) {
                return OhlcDataSummary();
            }
            unsigned int count = static_cast<unsigned int>(ticks_.size());
            double last = ticks_.back().close;
            double high = ticks_.front().high;
            double low = ticks_.front().low;
            double volume = 0;
            for(const auto& tick : ticks_) {
                volume += tick.volume;
                if(tick.high > high) {
                    high = tick.high;
                }
                if(tick.low < low) {
                    low = tick.low;
                }
            }
            return OhlcDataSummary(count, last, high, low, volume, ticks_.front().timestamp, ticks_.back().timestamp, last_);
        }

        std::string description() const {
            std::ostringstream oss;
            for(const auto& tick : ticks_) {
                oss << " - Timestamp: " << std::chrono::system_clock::to_time_t(tick.timestamp);
                oss << " - Open: " << tick.open;
                oss << " - High: " << tick.high;
                oss << " - Low: " << tick.low;
                oss << " - Close: " << tick.close;
                oss << " - Volume: " << tick.volume;
                oss << " - Volume Weighted Average: " << tick.volumeWeightedAverage;
                oss << " - Count: " << tick.count;
            }
            return oss.str();
        }

        void save(const std::string& filepath) {
            // Save the information on a csv file
            std::ofstream file(filepath);
            if (file.is_open()) {
                file << "Timestamp,Open,High,Low,Close,Volume,VolumeWeightedAverage,Count\n";
                for (const auto& tick : ticks_) {
                    file << std::chrono::system_clock::to_time_t(tick.timestamp) << ","
                         << tick.open << ","
                         << tick.high << ","
                         << tick.low << ","
                         << tick.close << ","
                         << tick.volume << ","
                         << tick.volumeWeightedAverage << ","
                         << tick.count << "\n";
                }
                file.close();
            } else {
                throw std::runtime_error("Unable to open file");
            }
        }

    private:    
        exchange::Market market_;
        std::vector<OhlcTick> ticks_;
        std::chrono::system_clock::time_point last_;
    };


} // namespace exchange

#endif // EXCHANGE_COMMONTYPES_H
