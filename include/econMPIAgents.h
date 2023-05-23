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

    void traderAction(int action) {
        if (action == 1) {
            buyOrders += 1;
        } 
        if (action == 2) {
            sellOrders += 1;
        }
    }

    virtual int step();
};

class MPITrader: public Agent {
private:
    WealthManagement* wealth; 
    MPIMarket* market;
    int traderAction = 0;
    int currentRule = 1;

    // rule, strength
    std::unordered_map<int, int> learnRule = {};
    std::mt19937 gen; 
    std::uniform_int_distribution<int> distribution;

    int eval(int rule, double stockPrice, std::vector<int> marketState, double cash, double shares) {
        int action = 0; 
        switch (rule) {
            case 1: 
                if (marketState.at(0)==INCREASE && stockPrice < cash){
                    action = 1;
                } else if (marketState.at(0)==2 && shares >= 1) {
                    action = 2;
                } else {
                    action = 0;
                }
                break;
            case 2:
                if (marketState.at(1) == INCREASE && shares >= 1){
                    action = 2;
                } else if (stockPrice < cash && marketState.at(2) == DECREASE){
                    action = 1;
                } else {
                    action = 0;
                }
                break; 
            case 3:
                if (marketState.at(1) == INCREASE && stockPrice < cash){
                    action = 1;
                } else if (marketState.at(1) == INCREASE && shares >= 1){
                    action = 2;
                } else {
                    action = 0;
                }
                break;
            case 4:
                if (distribution(gen) < 3){
                    if (stockPrice < cash) {
                        action = 1;
                    } else {
                        action = 0;
                    }
                } else {
                    if (shares >= 1) {
                        action = 2;
                    } else {
                        action = 0;
                    }
                }    
                break;
            case 5:
                if (marketState.at(2) == INCREASE && shares >= 1){
                    action = 2;
                } else if (marketState.at(2) == DECREASE && stockPrice < cash){
                    action = 1;
                } else {
                    action = 0;
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

    void inform(double stockPrice, double dividend, std::vector<int> market) {
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
        if (traderAction == 1) {
            this->wealth->buyStock(stockPrice);
        } else if (traderAction == 2) {
            this->wealth->sellStock(stockPrice);
        }
    }

    void updateMarket(MPIMarket* market) {
        this->market = market;
    }

    virtual int step() {
        // int receivedMessages = 0;
        std::optional<Message> m = receive();
        while (m.has_value()) {
            const std::vector<double>* retrievedContent = m.value().getContent();
            std::vector<int> markets = {
                static_cast<int>((*retrievedContent)[2]), 
                 static_cast<int>((*retrievedContent)[3]), 
                 static_cast<int>((*retrievedContent)[4])};

            inform((*retrievedContent)[0], (*retrievedContent)[1], markets);
            m = receive();
            // receivedMessages += 1;
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
        const std::vector<double>* retrievedContent = m.value().getContent();
        int act = static_cast<int>((*retrievedContent)[0]);
        traderAction(act);
        m = receive();
        receivedMessages += 1; 
    }

    // std::cout << "Market agent receives " << receivedMessages << std::endl;

    this->dividend = stock->getDividend();
    this->stockPrice = this->stock->priceAdjustment(buyOrders, sellOrders);
    this->dividend = this->stock->getDividend();
    std::vector<double> msg = {this->stockPrice, this->dividend};
    std::vector<int> stockInfo = stock->getStockStates(stockPrice, dividend);
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