// Header
#include "PortfolioData.h"

#include <iostream>


namespace exchange {

    void StatsData::addInvestment(Market market, const Investment& investment) {
        // find the market in the investments map
        auto it = investments_.find(market);
        if (it != investments_.end()) {
            it->second.push(investment);
        } else {
            std::queue<Investment> investments;
            investments.push(investment);
            investments_[market] = investments;
        }
    }

    void StatsData::addCost(Market market, double total) {
        auto it = cost_.find(market);
        if (it != cost_.end()) {
            it->second += total;
        } else {
            cost_[market] = total;
        }
    }

    void StatsData::addWorth(Market market, double total) {
        auto it = worth_.find(market);
        if (it != worth_.end()) {
            it->second += total;
        } else {
            worth_[market] = total;
        }
    }

    void StatsData::addVolume(Market market, double total) {

        auto it = volume_.find(market);
        if (it != volume_.end()) {
            it->second += total;
        } else {
            volume_[market] = total;
        }
    }

    double StatsData::getInvestmentCost(Market market) const {
        double total = 0;
        auto it = investments_.find(market);
        if (it != investments_.end()) {
            total = getCost_(it->second);
        }
        return total;
    }

    double StatsData::getInvestmentCost() const {
        double total = 0;
        for (const auto& pair : investments_) {
            total += getCost_(pair.second);
        }
        return total;
    }

    double StatsData::getInvestmentAveragePrice(Market market) const {
        double quantity = 0;
        double total = 0;
        double averagePrice = 0;
        auto it = investments_.find(market);
        if (it != investments_.end()) {
            std::queue<Investment> investments = it->second;
            while (!investments.empty()) {
                const auto& investment = investments.front();
                total += investment.quantity * investment.price + investment.fee;
                quantity += investment.quantity;
                investments.pop();
            }
            averagePrice = total / quantity;
        }
        return averagePrice;
    }

    double StatsData::getInvestmentWorth(Market market, float lastPrice, float multiplier, float increment) const {
        double total = 0;
        auto it = investments_.find(market);
        if (it != investments_.end()) {
            std::queue<Investment> investments = it->second;
            total = getWorth_(investments, lastPrice, multiplier, increment);
        }
        return total;
    }

    double StatsData::getInvestmentWorth(const TickersInformation& tickersInformation, float multiplier, const std::map<exchange::Ticket, float>& increments) const {
        double total = 0;
        for (const auto& pair : investments_) {
            std::queue<Investment> investments = pair.second;
            Market market = pair.first;
            Ticket asset = getAssetType(market);
            double lastPrice = tickersInformation.getLastPrice(market);
            auto it = increments.find(asset);
            float increment = (it != increments.end()? it->second : 0);
            total += getWorth_(investments, lastPrice, multiplier, increment);
        }
        return total;
    }

    double calculateTaxexStage_(std::queue<Investment>& investments, double total, double lastPrice, double multiplier, double increment) {
        double estimatedTaxes = 0;
        double totalWorth = 0;
        double totalCost = 0;
            while (totalWorth < total && !investments.empty()) {
                double cost = (investments.front().quantity * investments.front().price + investments.front().fee);
                double worth = (investments.front().quantity * (lastPrice * (multiplier + increment)) * 0.9975);
                if(totalWorth + worth > total) {
                    double partialWorth = total - totalWorth;
                    double partialCost = cost * (partialWorth / worth);
                    totalWorth += partialWorth;
                    totalCost += partialCost;
                    double quantity = partialWorth / (lastPrice * (multiplier + increment));
                    investments.front().quantity -= quantity;
                } else {
                    totalWorth += worth;
                    totalCost += cost;
                    investments.pop();
                }
            }
            if (totalWorth > 0) {
                estimatedTaxes = getProfitTax(totalWorth - totalCost);
                if(estimatedTaxes > 0,01) {
                    estimatedTaxes += calculateTaxexStage_(investments, estimatedTaxes, lastPrice, multiplier, increment);
                }
            }
        return estimatedTaxes;
    }

    double StatsData::estimateTaxesAfterSelling(Market market, double total, double lastPrice, double multiplier, double increment) const {
        double estimatedTaxes = 0;
        auto it = investments_.find(market);
        if (it != investments_.end()) {
            std::queue<Investment> investments = it->second;
            estimatedTaxes = calculateTaxexStage_(investments, total, lastPrice, multiplier, increment);
        }
        return estimatedTaxes;
    }

    StatsOverview StatsData::getOverview(const TickersInformation& tickersInformation, float multiplier, const std::map<exchange::Ticket, float>& increments) const {
        StatsOverview overview;
        // The materialized summary
        overview.materialized.cost = getCost();
        overview.materialized.worth = getWorth();
        overview.materialized.worthPercentage = overview.materialized.worth / overview.materialized.cost * 100;
        overview.materialized.gainLoss = overview.materialized.worth - overview.materialized.cost;
        overview.materialized.gainLossPercentage = overview.materialized.gainLoss / overview.materialized.cost * 100;
        overview.materialized.taxes = getProfitTax(overview.materialized.gainLoss);
        overview.materialized.taxesPercentage = overview.materialized.taxes / overview.materialized.gainLoss * 100;
        overview.materialized.netProfit = overview.materialized.gainLoss - overview.materialized.taxes;
        overview.materialized.volume = overview.materialized.cost + getInvestmentCost() + overview.materialized.worth;  // Same as getVolume(referenceCurrency_)
        // The unmaterialized summary
        overview.unmaterialized.cost = getInvestmentCost();
        overview.unmaterialized.worth = getInvestmentWorth(tickersInformation, multiplier, increments);
        overview.unmaterialized.worthPercentage = overview.unmaterialized.worth / overview.unmaterialized.cost * 100;
        overview.unmaterialized.gainLoss = overview.unmaterialized.worth - overview.unmaterialized.cost;
        overview.unmaterialized.gainLossPercentage = overview.unmaterialized.gainLoss / overview.unmaterialized.cost * 100;
        overview.unmaterialized.taxes = getProfitTax(overview.unmaterialized.gainLoss);
        overview.unmaterialized.taxesPercentage = overview.unmaterialized.taxes / overview.unmaterialized.gainLoss * 100;
        overview.unmaterialized.netProfit = overview.unmaterialized.gainLoss - overview.unmaterialized.taxes;
        overview.unmaterialized.volume = overview.materialized.volume + overview.unmaterialized.worth;
        overview.referenceCurrency = referenceCurrency_;
        return overview;
    }


    // Private methods
    double StatsData::getCost_(std::queue<Investment> investments) const {
        double total = 0;
        while (!investments.empty()) {
            const Investment& investment = investments.front();
            total += investment.quantity * investment.price + investment.fee;
            investments.pop();
        }
        return total;
    }

    double StatsData::getWorth_(std::queue<Investment> investments, double lastPrice, float multiplier, float increment) const {
        double total = 0;
        while (!investments.empty()) {
            total += investments.front().quantity * (lastPrice * (multiplier + increment));
            investments.pop();
        }
        return total;
    }


} // namespace exchange
