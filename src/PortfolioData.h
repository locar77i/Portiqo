#ifndef EXCHANGE_PORTFOLIODATA_H
#define EXCHANGE_PORTFOLIODATA_H

// Stl  headers
#include <string>
#include <map>
#include <queue>

// Headers
#include "Types.h"

// exchange headers
#include "exchange/Exchange.h"


namespace exchange {


    // Struct to store the stats resumen of a portfolio
    struct StatsSummary {
        double cost;
        double worth;
        double worthPercentage;
        double gainLoss;
        double gainLossPercentage;
        double taxes;
        double taxesPercentage;
        double netProfit;
        double volume;
    };

    struct StatsOverview {
        StatsSummary materialized;
        StatsSummary unmaterialized;
        Ticket referenceCurrency;
    };


    // Class to store the statistics data of a portfolio
    class StatsData {
    public:
        StatsData(Ticket referenceCurrency=Ticket::EUR)
            : referenceCurrency_(referenceCurrency), investments_(), totalProfit_(0), cost_(), worth_(), volume_() {
        }

        ~StatsData() {}

    public:
        Ticket getReferenceCurrency() const { return referenceCurrency_; }

        void addInvestment(Market market, const Investment& investment);
        std::queue<Investment>& getInvestment(Market market) {
            return investments_[market];
        }
    

        void addCost(Market market, double total);
        double getCost(Market market) const {
            auto it = cost_.find(market);
            if (it != cost_.end()) {
                return it->second;
            }
            return 0;
        }
        double getCost() const {
            double cost = 0;
            for (const auto& pair : cost_) {
                cost += pair.second;
            }
            return cost;
        }

        void addWorth(Market market, double total);
        double getWorth(Market market) const {
            auto it = worth_.find(market);
            if (it != worth_.end()) {
                return it->second;
            }
            return 0;
        }
        double getWorth() const {
            double worth = 0;
            for (const auto& pair : worth_) {
                worth += pair.second;
            }
            return worth;
        }

        void addVolume(Market market, double total);
        double volume(Market market) const {
            auto it = volume_.find(market);
            if (it != volume_.end()) {
                return it->second;
            }
            return 0;
        }

        void addProfit(double profit) { totalProfit_ += profit; }
        double getTotalProfit() const { return totalProfit_; }
        
        double getInvestmentCost(Market market) const;
        double getInvestmentCost() const;

        double getInvestmentAveragePrice(Market market) const;

        double getInvestmentWorth(Market market, float lastPrice, float multiplier, float increment) const;
        double getInvestmentWorth(const TickersInformation& tickersInformation, float multiplier, const std::map<exchange::Ticket, float>& increments) const;

        inline double getMaterializedTaxes() const {
            return getProfitTax(getWorth() - getCost());
        }

        double estimateTaxesAfterSelling(Market market, double total, double lastPrice, double multiplier, double increment) const;

        StatsOverview getOverview(const TickersInformation& tickersInformation, float multiplier, const std::map<exchange::Ticket, float>& increments) const;

    private:
        double getCost_(std::queue<Investment> investments) const;
        double getWorth_(std::queue<Investment> investments, double lastPrice, float multiplier, float increment) const;

    private:
        Ticket referenceCurrency_;
        std::map<Market, std::queue<Investment>> investments_;
        double totalProfit_;
        std::map<Market, double> cost_;
        std::map<Market, double> worth_;
        std::map<Market, double> volume_;
    };


    // Struct to store the data of a portfolio
    struct PortfolioData {
        std::string version;
        std::string name;
        std::string filename;
        Ticket referenceCurrency;
        std::vector<std::shared_ptr<Operation>> operations;
        GeneralBalance balance;
        StatsData stats;
    };

    // Struct to update the data of a portfolio
    struct PortfolioUpdate {
        std::size_t count;
        std::shared_ptr<Operation> operation;
        GeneralBalance balance;
        StatsData stats;
    };

    // Struct to remove the data of a portfolio
    struct PortfolioRemove {
        std::size_t count;
        std::chrono::system_clock::time_point timePoint; // The id of the removed operation
        GeneralBalance balance;
        StatsData stats;
    };

} // namespace exchange


#endif // EXCHANGE_PORTFOLIODATA_H
