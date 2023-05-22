#ifndef ECONOMICS_H
#define ECONOMICS_H

#include <random>
#include <numeric>
#include <vector>

enum MarketState {
    INCREASE, DECREASE, NO_CHANGE
};

enum Action {
    BUY, SELL, NO_ACTION
};

class Stock {
private:
    std::vector<double> prices;
    double priceAdjustmentFactor = 0.01;
    double currentPrice = 100;
    double lastDividend = 0;
    double last10Avg = 0;
    double last50Avg = 0;

    MarketState dividendState = NO_CHANGE;
    MarketState last10AvgState = NO_CHANGE;
    MarketState last50AvgState = NO_CHANGE;
    
    // Mersenne twister PRNG, initialized with seed from previous random device instance
    std::mt19937 gen; 
    std::normal_distribution<double> distribution;
    
public:
    Stock(double priceAdjustmentFactor){
        this -> priceAdjustmentFactor = priceAdjustmentFactor;
            
        std::random_device rd; 
        // Mersenne twister PRNG, initialized with seed from previous random device instance
        std::mt19937 gen(rd()); 
        this->gen = gen;
        std::normal_distribution<double> distribution(0.0, 1.0);
        this->distribution = distribution;
    }

    void updateAvg() {
        int time = prices.size();
        if (time < 10) {
            this->last10AvgState = NO_CHANGE;
        } else {
            double avg10 = std::accumulate(prices.end()-10, prices.end(), 0.0) / 10;
            if (avg10 > last10Avg) {
                this->last10AvgState = INCREASE;
            } else if (avg10 == last10Avg) {
                this->last10AvgState = NO_CHANGE;
            } else {
                this->last10AvgState = DECREASE;
            }
            this->last10Avg = avg10;
        } 

        if (time < 50) {
            this->last50AvgState = NO_CHANGE;
        } else {
            double avg50 = std::accumulate(prices.end()-50, prices.end(), 0.0) / 50;
            if (avg50 > last50Avg) {
                this->last50AvgState = INCREASE;
            } else if (avg50 == last50Avg) {
                this->last50AvgState = NO_CHANGE;
            } else {
                this->last50AvgState = DECREASE;
            }
            this->last50Avg = avg50;
        }
    }

    std::vector<MarketState> getStockStates(double price, double dividend){
        this->currentPrice = price;
        this->prices.push_back(price);
        if (dividend > this->lastDividend) {
            dividendState = INCREASE;
        } else if (dividend < this->lastDividend){
            dividendState = DECREASE;
        } else {
            dividendState = NO_CHANGE;
        }
        this->lastDividend = dividend; 
        updateAvg();
        std::vector<MarketState> states = {dividendState, last10AvgState, last50AvgState};
        return states;
    }

    double priceAdjustment(int buyOrders, int sellOrders) {
        if (currentPrice <=0) {
            return 100;
        } else {
            double ans = currentPrice * (1 + priceAdjustmentFactor * (buyOrders - sellOrders));
            return ans;
        }
    }

    double getDividend() {
        double x = 0.1* distribution(gen);
        if (x < 0) {
            return 0;
        } else {
            return x;
        }
    }


};

class WealthManagement {
    public:
        double wealth = 0;
        double cash = 0;
        double shares = 0;
        double bankDeposit = 0;
        double interestRate = 0.01;

        WealthManagement(double initWealth, double interestRate) {
            this->bankDeposit = 0.5*initWealth;
            this->interestRate = interestRate;
            this->cash = initWealth - this->bankDeposit;
        }

        void buyStock(double stockPrice) {
            this->shares += 1;
            this->cash -= stockPrice;
        }

        void sellStock(double stockPrice) {
            this->shares -= 1;
            this->cash += stockPrice;
        }

        double estimateWealth(double stockPrice) {
            return stockPrice*this->shares + this->bankDeposit + this->cash;
        }

        void addInterest(){
            this->bankDeposit = this->bankDeposit * (1+this->interestRate);
        }

        void addDividends(double dividendPerShare) {
            this->cash += this->shares * dividendPerShare;
        }
};
#endif