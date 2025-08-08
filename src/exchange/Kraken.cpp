// Header
#include "Kraken.h"

// cURL headers
#include <curl/curl.h>

// Stl headers
#include <iostream>


namespace exchange {

    namespace kraken {
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }

        struct HeaderData {
            int responseCode;

            HeaderData() : responseCode(-1) {}
        };

        // The header callback function
        size_t headerCallback(void* contents, size_t size, size_t nmemb, void* userp) {
            std::string header;
            header.assign((char*)contents, size * nmemb);
            HeaderData* headerData = (HeaderData*)userp;
            if (header.find("HTTP") != std::string::npos) {
                // Get the version
                std::string version = header.substr(5, 3);
                //std::cout << " -KRAKEN-> HTTP version: " << version << std::endl;
                // Get the response code
                std::string responseCode = header.substr(9, 3);
                //std::cout << " -KRAKEN-> Response code: " << responseCode << std::endl;
                headerData->responseCode = std::stoi(responseCode);
            }
            return size * nmemb;
        }

        Response getResponseType(int responseCode) {
            switch (responseCode) {
                case 200:
                    return Response::Ok;
                default:
                    if (responseCode >= 500) {
                        return Response::Error;
                    }
                    return Response::Warning;
            }
        }

    } // namespace Kraken

    
    // Kraken class
    Response Kraken::getSystemStatus(SystemStatus& systemStatus) {
        CURL* curl;
        CURLcode res;
        Response response;
        kraken::HeaderData headerData;
        const std::string url = "https://api.kraken.com/0/public/SystemStatus";
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, kraken::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, kraken::headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                response = kraken::getResponseType(headerData.responseCode);
                if (response == Response::Ok) {
                    systemStatus = getSystemStatus_(readBuffer);
                } else {
                    std::cout << " -KRAKEN-> ERROR: " << readBuffer << "(url: " << url << ")" << std::endl;
                }
            } else {
                throw std::runtime_error(curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return response;
    }


    Response Kraken::getTickersInformation(TickersInformation& tickersInformation, const std::map<Market, std::string>& pairs) {
        CURL* curl;
        CURLcode res;
        Response response;
        kraken::HeaderData headerData;
        std::string baseUrl = "https://api.kraken.com/0/public/Ticker";
        curl = curl_easy_init();
        if(curl) {
            std::string url = baseUrl + "?pair=" + getPairList_(pairs);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, kraken::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, kraken::headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                response = kraken::getResponseType(headerData.responseCode);
                if (response == Response::Ok) {
                    json jo = json::parse(readBuffer);
                    auto result_it = jo.find("result");
                    if (result_it != jo.end()) {
                        const auto& result = *result_it;
                        for (const auto it : pairs) {
                            Market market = it.first;
                            std::string pair = getPairName(market);
                            auto pair_it = result.find(pair);
                            if (pair_it != result.end()) {
                                const auto& pair = *pair_it;
                                TickerInformation tickerInfo;
                                tickerInfo.asset = getAssetType(market);
                                tickerInfo.currency = getCurrencyType(market);
                                assign_(tickerInfo, pair);
                                tickersInformation.add(market, tickerInfo);
                            }
                            else {
                                throw std::invalid_argument("Pair " + pair + " not found (url: " + url + ")");
                            }
                        }
                    }
                    else {
                        throw std::invalid_argument("Result not found (url: " + url + ")");
                    }
                } else {
                    std::cout << " -KRAKEN-> ERROR: " << readBuffer << "(url: " << url << ")" << std::endl;
                }
            } else {
                throw std::runtime_error(curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return response;
    }

    Response Kraken::getOrderBook(OrderBook& orderBook, unsigned int count) {
        CURL* curl;
        CURLcode res;
        Response response;
        kraken::HeaderData headerData;
        const std::string baseUrl = "https://api.kraken.com/0/public/Depth";
        curl = curl_easy_init();
        if(curl) {
            // https://api.kraken.com/0/public/Depth?pair=XBTEUR&count=500
            std::string pair = getPairName(orderBook.getMarket());
            std::string url = baseUrl + "?pair=" + pair + "&count=500";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, kraken::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, kraken::headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                response = kraken::getResponseType(headerData.responseCode);
                if (response == Response::Ok) {
                    json jo = json::parse(readBuffer);
                    auto result_it = jo.find("result");
                    if (result_it != jo.end()) {
                        const auto& result = *result_it;
                        auto pair_it = result.find(pair);
                        if (pair_it != result.end()) {
                            const auto& pair = *pair_it;
                            assign_(orderBook, pair, count);
                        }
                        else {
                            throw std::invalid_argument("Pair " + pair + " not found (url: " + url + ")");
                        }
                    }
                    else {
                        throw std::invalid_argument("Result not found (url: " + url + ")");
                    }
                } else {
                    std::cout << " -KRAKEN-> ERROR: " << readBuffer << "(url: " << url << ")" << std::endl;
                }
            } else {
                throw std::runtime_error(curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return response;
    }

    Response Kraken::getRecentTrades(RecentTrades& recentTrades, unsigned int count) {
        CURL* curl;
        CURLcode res;
        Response response;
        kraken::HeaderData headerData;
        const std::string baseUrl = "https://api.kraken.com/0/public/Trades";
        curl = curl_easy_init();
        if(curl) {
            // https://api.kraken.com/0/public/Trades?pair=XBTEUR&count=500
            std::string pair = getPairName(recentTrades.getMarket());
            std::string url = baseUrl + "?pair=" + pair + "&count=" + std::to_string(count);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, kraken::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, kraken::headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                response = kraken::getResponseType(headerData.responseCode);
                if (response == Response::Ok) {
                    json jo = json::parse(readBuffer);
                    auto result_it = jo.find("result");
                    if (result_it != jo.end()) {
                        const auto& result = *result_it;
                        auto pair_it = result.find(pair);
                        if (pair_it != result.end()) {
                            const auto& pair = *pair_it;
                            for (const auto& jtrade : pair) {
                                Trade trade;
                                assign_(trade, jtrade);
                                recentTrades.addTrade(trade);
                            }
                        }
                        else {
                            throw std::invalid_argument("Pair " + pair + " not found (url: " + url + ")");
                        }
                    }
                    else {
                        throw std::invalid_argument("Result not found (url: " + url + ")");
                    }
                } else {
                    std::cout << " -KRAKEN-> ERROR: " << readBuffer << "(url: " << url << ")" << std::endl;
                }
            } else {
                throw std::runtime_error(curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return response;
    }

    Response Kraken::getOhlcData(OhlcData& ohlcData, unsigned int interval, unsigned int since) {
        CURL* curl;
        CURLcode res;
        Response response;
        kraken::HeaderData headerData;
        const std::string baseUrl = "https://api.kraken.com/0/public/OHLC";
        curl = curl_easy_init();
        if(curl) {
            // https://api.kraken.com/0/public/OHLC?pair=XBTEUR&interval=1
            std::string pair = getPairName(ohlcData.getMarket());
            std::string url = baseUrl + "?pair=" + pair + "&interval=" + std::to_string(interval);
            if(since > 0) {
                url += "&since=" + std::to_string(since);
            }
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, kraken::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, kraken::headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                response = kraken::getResponseType(headerData.responseCode);
                if (response == Response::Ok) {
                    json jo = json::parse(readBuffer);
                    auto result_it = jo.find("result");
                    if (result_it != jo.end()) {
                        const auto& result = *result_it;
                        assign_(ohlcData, result, pair);
                    }
                    else {
                        throw std::invalid_argument("Result not found (url: " + url + ")");
                    }
                } else {
                    std::cout << " -KRAKEN-> ERROR: " << readBuffer << "(url: " << url << ")" << std::endl;
                }
            } else {
                throw std::runtime_error(curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return response;
    }

        // PRIVATE
        Status Kraken::getStatus_(const std::string& str) {
            if (str == "online") {
                return Status::Online;
            } else if (str == "maintenance" || str == "cancel_only" || str == "post_only") {
                return Status::Offline;
            } else {
                return Status::Unknown;
            }
        }

        std::chrono::system_clock::time_point Kraken::getTimePoint_(const std::string& str) {
            std::tm tm = {};
            std::istringstream is(str);
            is >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
            return std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }

        SystemStatus Kraken::getSystemStatus_(const std::string& jstr) {
            json jo = json::parse(jstr);
            auto result_it = jo.find("result");
            if (result_it == jo.end()) {
                return SystemStatus();
            }
            const auto& result = *result_it;
            std::vector<std::string> errors;
            auto error_it = jo.find("error");
            if (error_it != jo.end()) {
                for (const auto& e : *error_it) {
                    errors.push_back(e.get<std::string>());
                }
            }
            return SystemStatus(getStatus_(result["status"]), getTimePoint_(result["timestamp"]), errors);
        }

        std::string Kraken::getPairName(Market market) {
            auto it = KrakenPairs.find(market);
            if (it != KrakenPairs.end()) {
                return it->second;
            }
            std::string msg = "Unknown market: " + getMarketName(market) + " (" + std::to_string(static_cast<int>(market)) + ")";
            throw std::invalid_argument(msg);
        }


        std::string Kraken::getPairList_(const std::map<Market, std::string>& pairs) {
            std::string pairList;
            for (const auto it : pairs) {
                pairList += getPairName(it.first) + ",";
            }
            pairList.pop_back();
            return pairList;
        }

        void Kraken::assign_(TickerInformation& tickerInfo, const json& jo) {
            if (jo.contains("a") && jo["a"].is_array() && !jo["a"].empty()) {
                tickerInfo.ask = std::stod(jo["a"][0].get<std::string>());
            }
            if (jo.contains("b") && jo["b"].is_array() && !jo["b"].empty()) {
                tickerInfo.bid = std::stod(jo["b"][0].get<std::string>());
            }
            if (jo.contains("c") && jo["c"].is_array() && !jo["c"].empty()) {
                tickerInfo.last = std::stod(jo["c"][0].get<std::string>());
                tickerInfo.lastLotVolume = std::stod(jo["c"][1].get<std::string>());
            }
            if (jo.contains("v") && jo["v"].is_array() && jo["v"].size() > 1) {
                tickerInfo.volume = std::stod(jo["v"][1].get<std::string>());
            }
            if (jo.contains("p") && jo["p"].is_array() && jo["p"].size() > 1) {
                tickerInfo.volumeWeightedAverage = std::stod(jo["p"][1].get<std::string>());
            }
            if (jo.contains("t") && jo["t"].is_array() && jo["t"].size() > 1) {
                tickerInfo.trades = jo["t"][1];
            }
            if (jo.contains("l") && jo["l"].is_array() && jo["l"].size() > 1) {
                tickerInfo.low = std::stod(jo["l"][1].get<std::string>());
            }
            if (jo.contains("h") && jo["h"].is_array() && jo["h"].size() > 1) {
                tickerInfo.high = std::stod(jo["h"][1].get<std::string>());
            }
            if (jo.contains("o")) {
                tickerInfo.openingPrice = std::stod(jo["o"].get<std::string>());
            }
        }

        // OrderBook
        void Kraken::assign_(OrderBook& orderBook, const json& jo, unsigned int count) {
            if (jo.contains("asks") && jo["asks"].is_array() && jo.contains("bids") && jo["bids"].is_array()) {
            double askInitialPrice = 0.0, askDistance = 0.0;
            double bidInitialPrice = 0.0, bidDistance = 0.0;
            auto asks_it = jo["asks"].begin();
            auto bids_it = jo["bids"].begin();
            Ask ask;
            if(asks_it != jo["asks"].end()) {
                assign_(ask, *asks_it);
                askInitialPrice = ask.price;
                orderBook.addAsk(ask);
                ++asks_it;
            }
            Bid bid;
            if(bids_it != jo["bids"].end()) {
                assign_(bid, *bids_it);
                bidInitialPrice = bid.price;
                orderBook.addBid(bid);
                ++bids_it;
            }
            unsigned int total = 2;
            while(total < count && (asks_it != jo["asks"].end() || bids_it != jo["bids"].end())) {
                if(asks_it != jo["asks"].end() && bids_it != jo["bids"].end()) {
                    assign_(ask, *asks_it);
                    assign_(bid, *bids_it);
                    askDistance = ask.price - askInitialPrice;
                    bidDistance = bidInitialPrice - bid.price;
                    if(askDistance < bidDistance) {
                        orderBook.addAsk(ask);
                        ++asks_it;
                    } else {
                        orderBook.addBid(bid);
                        ++bids_it;
                    }
                } else if(asks_it != jo["asks"].end()) {
                    assign_(ask, *asks_it);
                    orderBook.addAsk(ask);
                    ++asks_it;
                } else {
                    assign_(bid, *bids_it);
                    orderBook.addBid(bid);
                    ++bids_it;
                }
                ++total;
            }
        }
/*
            if (jo.contains("asks") && jo["asks"].is_array() && !jo["asks"].empty()) {
                for (const auto& ask : jo["asks"]) {
                    Ask a;
                    assign_(a, ask);
                    orderBook.addAsk(a);
                }
            }
            if (jo.contains("bids") && jo["bids"].is_array() && !jo["bids"].empty()) {
                for (const auto& bid : jo["bids"]) {
                    Bid b;
                    assign_(b, bid);
                    orderBook.addBid(b);
                }
            }
*/
        }

        void Kraken::assign_(Ask& ask, const json& jask) {
            ask.price = std::stod(jask[0].get<std::string>());
            ask.volume = std::stod(jask[1].get<std::string>());
            unsigned long long timestamp = jask[2].get<unsigned long long>();
        }

        void Kraken::assign_(Bid& bid, const json& jbid) {
            bid.price = std::stod(jbid[0].get<std::string>());
            bid.volume = std::stod(jbid[1].get<std::string>());
            unsigned long long timestamp = jbid[2].get<unsigned long long>();
        }


        // RecentTrades
        void Kraken::assign_(Trade& trade, const json& jtrade) {
            trade.id = jtrade[6].get<unsigned long long>();
            trade.price = std::stod(jtrade[0].get<std::string>());
            trade.volume = std::stod(jtrade[1].get<std::string>());
            trade.buy = jtrade[3].get<std::string>() == "b";
            unsigned long long timestamp = jtrade[2].get<unsigned long long>();
            trade.timestamp = std::chrono::system_clock::time_point(std::chrono::seconds(timestamp));
        }


        // OhlcData
        void Kraken::assign_(OhlcData& ohlcData, const json& result, const std::string& pairName) {
            auto pair_it = result.find(pairName);
            if (pair_it != result.end()) {
                const auto& pair = *pair_it;
                if (pair.is_array() && !pair.empty()) {
                    for (const auto& tick : pair) {
                        OhlcTick t;
                        assign_(t, tick);
                        ohlcData.addTick(t);
                    }
                }
            }
            else {
                throw std::invalid_argument("Pair " + pairName + " not found");
            }
            auto lats_it = result.find("last");
            if (lats_it != result.end()) {
                const auto& last = *lats_it;
                ohlcData.setLastTimestamp(std::chrono::system_clock::time_point(std::chrono::seconds(last.get<unsigned long long>())));
            }
            else {
                throw std::invalid_argument("Last timestamp not found");
            }
        }

        void Kraken::assign_(OhlcTick& ohlcTick, const json& tick) {
            ohlcTick.timestamp = std::chrono::system_clock::time_point(std::chrono::seconds(tick[0].get<unsigned long long>()));
            ohlcTick.open = std::stod(tick[1].get<std::string>());
            ohlcTick.high = std::stod(tick[2].get<std::string>());
            ohlcTick.low = std::stod(tick[3].get<std::string>());
            ohlcTick.close = std::stod(tick[4].get<std::string>());
            ohlcTick.volume = std::stod(tick[5].get<std::string>());
            ohlcTick.volumeWeightedAverage = std::stod(tick[6].get<std::string>());
            ohlcTick.count = tick[7].get<unsigned int>();
        }	

} // namespace exchange
