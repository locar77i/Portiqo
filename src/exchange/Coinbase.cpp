// Header
#include "Coinbase.h"

// cURL headers
#include <curl/curl.h>

// Stl headers
#include <iostream>
#include <ctime>


namespace exchange {

    namespace coinbase {
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
            //std::cout << " -COINBASE-> HEADER: " << header << std::endl; 
            if (header.find("HTTP") != std::string::npos) {
                // Get the version
                std::string version = header.substr(5, 3);
                //std::cout << " -COINBASE-> HTTP version: " << version << std::endl;
                // Get the response code
                std::string responseCode = header.substr(9, 3);
                //std::cout << " -COINBASE-> Response code: " << responseCode << std::endl;
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

    } // namespace coinbase

    Response Coinbase::getSystemStatus(SystemStatus& systemStatus) {
        return coinbase::getResponseType(500);
    }

    Response Coinbase::getTicker(Ticker& ticker, const std::pair<Market, std::string>& pair) {
        CURL* curl;
        CURLcode res;
        Response response;
        coinbase::HeaderData headerData;
        const std::string baseUrl = "https://api.exchange.coinbase.com/products/" + pair.second + "/ticker";
        curl = curl_easy_init();
        if(curl) {
            // https://api.exchange.coinbase.com/products/BTC-EUR/ticker
            std::string url = baseUrl;
            //std::cout << " -> URL: " << url << std::endl;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, coinbase::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            // Set header 'Content-Type: application/json'
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "User-Agent: Locar/1.0");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, coinbase::headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            res = curl_easy_perform(curl);
            // Clean up the headers
            curl_slist_free_all(headers);
            if (res == CURLE_OK) {
                response = coinbase::getResponseType(headerData.responseCode);
                if (response == Response::Ok) {
                    json jo = json::parse(readBuffer);
                    assign_(ticker, jo);
                }
                else {
                    std::cout << " -COINBASE-> ERROR: " << readBuffer << "(url: " << url << ")" << std::endl;
                }
            } else {
                throw std::runtime_error(curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return response;
    }
    

    Response Coinbase::getOrderBook(OrderBook& orderBook, unsigned int count) {
        CURL* curl;
        CURLcode res;
        Response response;
        coinbase::HeaderData headerData;
        std::string pair = getPairName_(orderBook.getMarket());
        const std::string baseUrl = "https://api.exchange.coinbase.com/products/" + pair + "/book";
        curl = curl_easy_init();
        if(curl) {
            // https://api.exchange.coinbase.com/products/BTC-EUR/book?level=2
            std::string url = baseUrl + "?level=2";
            //std::cout << " -> URL: " << url << std::endl;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, coinbase::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            // Set header 'Content-Type: application/json'
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "User-Agent: Locar/1.0");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, coinbase::headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            res = curl_easy_perform(curl);
            // Clean up the headers
            curl_slist_free_all(headers);
            if (res == CURLE_OK) {
                response = coinbase::getResponseType(headerData.responseCode);
                if (response == Response::Ok) {
                    json jo = json::parse(readBuffer);
                    assign_(orderBook, jo, count);
                }
                else {
                    std::cout << " -COINBASE-> ERROR: " << readBuffer << "(url: " << url << ")" << std::endl;
                }
            } else {
                throw std::runtime_error(curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return response;
    }

    Response Coinbase::getRecentTrades(RecentTrades& recentTrades, unsigned int count) {
        CURL* curl;
        CURLcode res;
        Response response;
        coinbase::HeaderData headerData;
        std::string pair = getPairName_(recentTrades.getMarket());
        const std::string baseUrl = "https://api.exchange.coinbase.com/products/" + pair + "/trades";
        curl = curl_easy_init();
        if(curl) {
            // https://api.exchange.coinbase.com/products/BTC-EUR/trades?limit=500
            std::string url = baseUrl + "?limit=" + std::to_string(count);
            //std::cout << " -> URL: " << url << std::endl;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, coinbase::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            // Set header 'Content-Type: application/json'
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "User-Agent: Locar/1.0");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, coinbase::headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            res = curl_easy_perform(curl);
            // Clean up the headers
            curl_slist_free_all(headers);
            if (res == CURLE_OK) {
                response = coinbase::getResponseType(headerData.responseCode);
                if (response == Response::Ok) {
                    json jo = json::parse(readBuffer);
                    // iterate in reverse order
                    for (auto it = jo.rbegin(); it != jo.rend(); ++it) {
                        Trade trade;
                        assign_(trade, *it);
                        recentTrades.addTrade(trade);
                    }
                }
                else {
                    std::cout << " -COINBASE-> ERROR: " << readBuffer << "(url: " << url << ")" << std::endl;
                }
            } else {
                throw std::runtime_error(curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return response;
    }

        // PRIVATE
        std::string Coinbase::getPairName_(Market market) {
            auto it = CoinbasePairs.find(market);
            if (it != CoinbasePairs.end()) {
                return it->second;
            }
            std::string msg = "Unknown market: " + getMarketName(market) + " (" + std::to_string(static_cast<int>(market)) + ")";
            throw std::invalid_argument(msg);
        }

        std::string Coinbase::getPairList_(const std::vector<Market>& markets) {
            std::string pairList;
            for (const auto& market : markets) {
                pairList += getPairName_(market) + ",";
            }
            pairList.pop_back();
            return pairList;
        }

    void Coinbase::assign_(Ticker& ticker, const json& jo) {
        ticker.ask = std::stod(jo["ask"].get<std::string>());
        ticker.bid = std::stod(jo["bid"].get<std::string>());
        ticker.last = std::stod(jo["price"].get<std::string>());
        ticker.lastLotVolume = std::stod(jo["size"].get<std::string>());
        ticker.volume = std::stod(jo["volume"].get<std::string>());
    }

    void Coinbase::assign_(OrderBook& orderBook, const json& jo, unsigned int count) {
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
        unsigned int asksCount = static_cast<unsigned int>(count / 2);
        if (jo.contains("asks") && jo["asks"].is_array() && !jo["asks"].empty()) {
            for (const auto& ask : jo["asks"]) {
                Ask a;
                assign_(a, ask);
                orderBook.addAsk(a);
                if(orderBook.getAsks().size() >= asksCount) {
                    break;
                }
            }
        }
        unsigned int bidsCount = count - asksCount;
        if (jo.contains("bids") && jo["bids"].is_array() && !jo["bids"].empty()) {
            for (const auto& bid : jo["bids"]) {
                Bid b;
                assign_(b, bid);
                orderBook.addBid(b);
                if(orderBook.getBids().size() >= bidsCount) {
                    break;
                }
            }
        }
*/
    }

    void Coinbase::assign_(Ask& ask, const json& jask) {
        ask.price = std::stod(jask[0].get<std::string>());
        ask.volume = std::stod(jask[1].get<std::string>());
    }

    void Coinbase::assign_(Bid& bid, const json& jbid) {
        bid.price = std::stod(jbid[0].get<std::string>());
        bid.volume = std::stod(jbid[1].get<std::string>());
    }


    // RecentTrades
    void Coinbase::assign_(Trade& trade, const json& jtrade) {
        trade.id = jtrade["trade_id"].get<unsigned long long>();
        trade.price = std::stod(jtrade["price"].get<std::string>());
        trade.volume = std::stod(jtrade["size"].get<std::string>());
        trade.worth = trade.price * trade.volume;
        std::string timestamp = jtrade["time"].get<std::string>();
        // Convert string timestamp with fortmat "2021-10-01T00:00:00.000Z" to std::chrono::system_clock::time_point
        std::tm tm = {};
        std::istringstream ss(timestamp);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        std::time_t t = std::mktime(&tm);
        trade.timestamp = std::chrono::system_clock::from_time_t(t);
        trade.buy = jtrade["side"].get<std::string>() == "buy";
    }

} // namespace exchange
