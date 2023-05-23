#ifndef ECONOMICS_DMA_AGENTS_H
#define ECONOMICS_DMA_AGENTS_H

#include "simulation.h"
#include "economics.h"
#include <vector>
#include <random>

class DMATrader;

class DMAMarket: public Agent {
private:
    int buyOrders = 0;
    int sellOrders = 0;
    double stockPrice = 100;
    Stock* stock; 
    double dividend = 0;
    std::vector<DMATrader*> traders = {};

public:    
    DMAMarket(int id) : Agent(id) {}

    void updateTraders(std::vector<DMATrader*> traders) {
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

class DMATrader: public Agent {
private:
    WealthManagement* wealth; 
    DMAMarket* market;
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
    DMATrader(int id) : Agent(id) {
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

    void updateMarket(DMAMarket* market) {
        this->market = market;
    }

    virtual int step() {
        // std::cout << "DMA trader agent " << id << " runs!"<< std::endl;
        int act = this->traderAction;
        (this->market)->traderAction(act);
        // std::cout << "DMA trader agent " << id << " completes!"<< std::endl;
        return 1;
    }
};

int DMAMarket::step() {
    // std::cout << "DMA Market agent runs!"<< std::endl;
    std::vector<int> stockInfo = stock->getStockStates(stockPrice, dividend);
    this->dividend = stock->getDividend();
    for (const auto & trader : this->traders) {
        // std::cout << "DMA Market agent informs trader " << trader->id << std::endl;
        trader->inform(this->stockPrice, this->dividend, stockInfo);
    }
    this->stockPrice = this->stock->priceAdjustment(buyOrders, sellOrders);
    this->dividend = this->stock->getDividend();
    // std::cout << "DMA Market agent completes!"<< std::endl;
    return 1;
}

#endif