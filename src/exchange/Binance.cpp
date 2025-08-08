// Header
#include "Binance.h"

// cURL headers
#include <curl/curl.h>

// Stl headers
#include <iostream>


namespace exchange {

    namespace binance {

        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }

        struct HeaderData {
            int responseCode;
            unsigned long long usedWeight;

            HeaderData() : responseCode(-1), usedWeight(0) {}
        };

        // The header callback function
        size_t headerCallback(void* contents, size_t size, size_t nmemb, void* userp) {
            std::string header;
            header.assign((char*)contents, size * nmemb);
            HeaderData* headerData = (HeaderData*)userp;
            if (header.find("HTTP") != std::string::npos) {
                // Get the version
                std::string version = header.substr(5, 3);
                //std::cout << " -BINANCE-> HTTP version: " << version << std::endl;
                // Get the response code
                std::string responseCode = header.substr(9, 3);
                //std::cout << " -BINANCE-> Response code: " << responseCode << std::endl;
                headerData->responseCode = std::stoi(responseCode);
            }
            if (header.find("x-mbx-used-weight:") != std::string::npos) {
                std::string usedWeight = header.substr(19, header.size() - 19);
                //std::cout << " -BINANCE-> Used weight: " << usedWeight << std::endl;
                headerData->usedWeight = std::stoull(usedWeight);
            }
            return size * nmemb;
        }

        Response getResponseType(int responseCode) {
            switch (responseCode) {
                case 200:
                    return Response::Ok;
                case 429:
                    return Response::Stop;
                case 418:
                    return Response::Banned;
                default:
                    if (responseCode >= 500) {
                        return Response::Error;
                    }
                    return Response::Warning;
            }
        }

    } // namespace binance
    
    Response Binance::getSystemStatus(SystemStatus& systemStatus) {
        return Response::Error;
    }

    Response Binance::getTickersInformation(TickersInformation& tickersInformation, const std::map<Market, std::string>& pairs) {
        CURL* curl;
        CURLcode res;
        Response response;
        binance::HeaderData headerData;
        const std::string baseUrl = "https://api.binance.com/api/v3/ticker/24hr";
        curl = curl_easy_init();
        if(curl) {
            std::string url = baseUrl + "?symbols=" + getPairList_(pairs);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, binance::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, binance::headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                response = binance::getResponseType(headerData.responseCode);
                if (response == Response::Ok) {
                    json jo = json::parse(readBuffer);
                    if(jo.is_array()) {
                        for (const auto& jticker : jo) {
                            TickerInformation tickerInfo;
                            if(jticker.find("symbol") != jticker.end()) {
                                std::string symbol = jticker["symbol"].get<std::string>();
                                Market market = getMarketType_(symbol);
                                assign_(tickerInfo, jticker);
                                tickerInfo.asset = getAssetType(market);
                                tickerInfo.currency = getCurrencyType(market);
                                tickersInformation.add(market, tickerInfo);
                            }
                        }
                    }
                } else {
                    std::cout << " -BINANCE-> ERROR: " << readBuffer << " (url: " << url << ")" << std::endl;
                }
            } else {
                throw std::runtime_error(curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return response;
    }

    Response Binance::getOrderBook(OrderBook& orderBook, unsigned int count) {
        CURL* curl;
        CURLcode res;
        Response response;
        binance::HeaderData headerData;
        const std::string baseUrl = "https://api.binance.com/api/v3/depth";
        curl = curl_easy_init();
        if(curl) {
            // https://api.binance.com/api/v3/depth?symbol=BTCEUR&limit=500
            std::string pair = getPairName_(orderBook.getMarket());
            std::string url = baseUrl + "?symbol=" + pair + "&limit=500";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, binance::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, binance::headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                response = binance::getResponseType(headerData.responseCode);
                if (response == Response::Ok) {
                    json jo = json::parse(readBuffer);
                    assign_(orderBook, jo, count);
                } else {
                    std::cout << " -BINANCE-> ERROR: " << readBuffer << " (url: " << url << ")" << std::endl;
                }
            }
            if (res != CURLE_OK) {
                throw std::runtime_error(curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return response;
    }

    Response Binance::getRecentTrades(RecentTrades& recentTrades, unsigned int count) {
        CURL* curl;
        CURLcode res;
        Response response;
        binance::HeaderData headerData;
        const std::string baseUrl = "https://api.binance.com/api/v3/trades";
        curl = curl_easy_init();
        if(curl) {
            // https://api.binance.com/api/v3/trades?symbol=BTCEUR&limit=500
            std::string pair = getPairName_(recentTrades.getMarket());
            std::string url = baseUrl + "?symbol=" + pair + "&limit=" + std::to_string(count);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, binance::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, binance::headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                response = binance::getResponseType(headerData.responseCode);
                if (response == Response::Ok) {
                    json jo = json::parse(readBuffer);
                    for (const auto& jtrade : jo) {
                        Trade trade;
                        assign_(trade, jtrade);
                        recentTrades.addTrade(trade);
                    }
                } else {
                    std::cout << " -BINANCE-> ERROR: " << readBuffer << " (url: " << url << ")" << std::endl;
                }
            } else {
                throw std::runtime_error(curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return response;
    }

    // PRIVATE
    std::string Binance::getPairName_(Market market) {
        auto it = BinancePairs.find(market);
        if (it != BinancePairs.end()) {
            return it->second;
        }
        std::string msg = "Unknown market: " + getMarketName(market) + " (" + std::to_string(static_cast<int>(market)) + ")";
        throw std::invalid_argument(msg);
    }

    Market Binance::getMarketType_(const std::string& pair) {
        for (const auto& it : BinancePairs) {
            if (it.second == pair) {
                return it.first;
            }
        }
        std::string msg = "Unknown pair: " + pair;
        throw std::invalid_argument(msg);
    }

    std::string Binance::getPairList_(const std::map<Market, std::string>& pairs) {
        std::string pairList = "[";
        for (const auto& pair : pairs) {
            pairList += "\"" + getPairName_(pair.first) + "\",";
        }
        pairList.pop_back();
        pairList += "]";
        return pairList;
    }

    void Binance::assign_(TickerInformation& tickerInfo, const json& jo) {
        tickerInfo.ask = std::stod(jo["askPrice"].get<std::string>());
        tickerInfo.bid = std::stod(jo["bidPrice"].get<std::string>());
        tickerInfo.last = std::stod(jo["lastPrice"].get<std::string>());
        tickerInfo.lastLotVolume = std::stod(jo["lastQty"].get<std::string>());
        tickerInfo.volume = std::stod(jo["volume"].get<std::string>());
        tickerInfo.volumeWeightedAverage = std::stod(jo["weightedAvgPrice"].get<std::string>());
        tickerInfo.trades = jo["count"].get<unsigned int>();
        tickerInfo.low = std::stod(jo["lowPrice"].get<std::string>());
        tickerInfo.high = std::stod(jo["highPrice"].get<std::string>());
        tickerInfo.openingPrice = std::stod(jo["openPrice"].get<std::string>());
    }

    void Binance::assign_(OrderBook& orderBook, const json& jo, unsigned int count) {
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

    void Binance::assign_(Ask& ask, const json& jask) {
        ask.price = std::stod(jask[0].get<std::string>());
        ask.volume = std::stod(jask[1].get<std::string>());
    }

    void Binance::assign_(Bid& bid, const json& jbid) {
        bid.price = std::stod(jbid[0].get<std::string>());
        bid.volume = std::stod(jbid[1].get<std::string>());
    }


    // RecentTrades
    void Binance::assign_(Trade& trade, const json& jtrade) {
        trade.id = jtrade["id"].get<unsigned long long>();
        trade.price = std::stod(jtrade["price"].get<std::string>());
        trade.volume = std::stod(jtrade["qty"].get<std::string>());
        trade.worth = std::stod(jtrade["quoteQty"].get<std::string>());
        trade.timestamp = std::chrono::system_clock::time_point(std::chrono::milliseconds(jtrade["time"].get<unsigned long long>()));
        trade.buy = jtrade["isBuyerMaker"].get<bool>();
    }

} // namespace exchange
