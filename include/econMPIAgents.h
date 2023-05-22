#ifndef ECONOMICS_MPI_AGENTS_H
#define ECONOMICS_MPI_AGENTS_H

#include "simulation.h"
#include "economics.h"
#include <vector>
#include <random>

class MPITrader;

class MPIMarket: public Agent {
private:
    int buyOrders = 0;
    int sellOrders = 0;
    double stockPrice = 100;
    Stock* stock; 
    double dividend = 0;
    std::vector<MPITrader*> traders = {};

public:    
    MPIMarket(int id) : Agent(id) {}

    void updateTraders(std::vector<MPITrader*> traders) {
        this->traders.insert(this->traders.begin(), traders.begin(), traders.end());
        int totalTraders = (this->traders).size();
        stock = new Stock(0.1 / totalTraders);
    }

    void traderAction(Action action) {
        if (action == BUY) {
            buyOrders += 1;
        } 
        if (action == SELL) {
            sellOrders += 1;
        }
    }

    virtual int step();
};

class MPITrader: public Agent {
private:
    WealthManagement* wealth; 
    MPIMarket* market;
    Action traderAction = NO_ACTION;
    int currentRule = 1;

    // rule, strength
    std::unordered_map<int, int> learnRule = {};
    std::mt19937 gen; 
    std::uniform_int_distribution<int> distribution;

    Action eval(int rule, double stockPrice, std::vector<MarketState> marketState, double cash, double shares) {
        Action action = NO_ACTION; 
        switch (rule) {
            case 1: 
                if (marketState.at(0)==INCREASE && stockPrice < cash){
                    action = BUY;
                } else if (marketState.at(0)==2 && shares >= 1) {
                    action = SELL;
                } else {
                    action = NO_ACTION;
                }
                break;
            case 2:
                if (marketState.at(1) == INCREASE && shares >= 1){
                    action = SELL;
                } else if (stockPrice < cash && marketState.at(2) == DECREASE){
                    action = BUY;
                } else {
                    action = NO_ACTION;
                }
                break; 
            case 3:
                if (marketState.at(1) == INCREASE && stockPrice < cash){
                    action = BUY;
                } else if (marketState.at(1) == INCREASE && shares >= 1){
                    action = SELL;
                } else {
                    action = NO_ACTION;
                }
                break;
            case 4:
                if (distribution(gen) < 3){
                    if (stockPrice < cash) {
                        action = BUY;
                    } else {
                        action = NO_ACTION;
                    }
                } else {
                    if (shares >= 1) {
                        action = SELL;
                    } else {
                        action = NO_ACTION;
                    }
                }    
                break;
            case 5:
                if (marketState.at(2) == INCREASE && shares >= 1){
                    action = SELL;
                } else if (marketState.at(2) == DECREASE && stockPrice < cash){
                    action = BUY;
                } else {
                    action = NO_ACTION;
                }
                break;
            default:
                break;
        }
        return action;
    }

public:
    MPITrader(int id) : Agent(id) {
        std::random_device rd;
        std::mt19937 gen(rd());
        this->gen = gen;
        std::uniform_int_distribution<int> distribution(1, 5);
        this->distribution = distribution;
        this->wealth = new WealthManagement(1000, 0.001);
    }

    void inform(double stockPrice, double dividend, std::vector<MarketState> market) {
        this->wealth->addDividends(dividend);
        double updatedWealth = this->wealth->estimateWealth(stockPrice);
        // increase the strength if wealth has increased
        if (updatedWealth > this->wealth->wealth) {
            learnRule[currentRule] += 1;
        }
        // apply the next rule. 30% random, rest max strength
        if (distribution(gen) < 3) {
            currentRule = distribution(gen);
        } else {
            for (auto rule_strength: learnRule) {
                if (rule_strength.second > learnRule[currentRule]) {
                    currentRule = rule_strength.first;
                }
            }
        }
        this->traderAction = eval(currentRule, stockPrice, market, this->wealth->cash, this->wealth->shares);
        if (traderAction == BUY) {
            this->wealth->buyStock(stockPrice);
        } else if (traderAction == SELL) {
            this->wealth->sellStock(stockPrice);
        }
    }

    void updateMarket(MPIMarket* market) {
        this->market = market;
    }

    virtual int step() {
        int receivedMessages = 0;
        std::optional<Message> m = receive();
        while (m.has_value()) {
            std::vector<double> retrievedContent = m.value().getContent();
            double stockPrice = retrievedContent.at(0);
            double dividend = retrievedContent.at(1);
            std::vector<MarketState> markets = {};
            retrievedContent.erase(retrievedContent.begin(), retrievedContent.begin()+1);
            for (const auto & c: retrievedContent) {
                markets.push_back(static_cast<MarketState>(static_cast<int>(c)));
            }
            inform(stockPrice, dividend, markets);
            m = receive();
            receivedMessages += 1;
        }
        // std::cout << "Trader agent runs! receive msg " << receivedMessages << std::endl;

        std::vector<double> msg = {static_cast<double>(this->traderAction)};
        Message m1(msg);
        send(this->market->id, m1);
        return 1;
    }
};

int MPIMarket::step() {
    std::optional<Message> m = receive();
    int receivedMessages = 0;

    while (m.has_value()) {
        std::vector<double> retrievedContent = m.value().getContent();
        Action act = static_cast<Action>(static_cast<int>(retrievedContent.at(0)));
        traderAction(act);
        m = receive();
        receivedMessages += 1; 
    }

    // std::cout << "Market agent receives " << receivedMessages << std::endl;

    this->dividend = stock->getDividend();
    this->stockPrice = this->stock->priceAdjustment(buyOrders, sellOrders);
    this->dividend = this->stock->getDividend();
    std::vector<double> msg = {this->stockPrice, this->dividend};
    std::vector<MarketState> stockInfo = stock->getStockStates(stockPrice, dividend);
    for (const auto & c: stockInfo) {
        msg.push_back(static_cast<double>(static_cast<int>(c)));
    }
    Message m1(msg);

    for (const auto & trader : this->traders) {
        send(trader->id, m1);
    }
    return 1;
}

#endif