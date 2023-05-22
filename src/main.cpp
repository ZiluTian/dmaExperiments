#include <iostream>
#include <vector>
#include <random>

#include "simulation.h"
#include "economics.h"
#include "econDMAAgents.h"
#include "econMPIAgents.h"

void MPIEcon(int totalRounds);
void DMAEcon(int totalRounds);

// Main function
int main() {
    int totalRounds = 200;
    DMAEcon(totalRounds);
    return 0;
}

void MPIEcon(int totalRounds){
    MPIMarket* market = new MPIMarket(0);
    int traderIdOffset = 1;

    // std::vector<int> simTraders = {999, 9999, 99999};
    std::vector<int> simTraders = {9999};
    
    std::vector<MPITrader*> traderAgents = {};
    for (const auto & totalTraders: simTraders) {
        
        // Initialize trader and market agents
        int i = 0;
        while (i < totalTraders) {
            traderAgents.push_back(new MPITrader(i+traderIdOffset));    
            i++;
        }
        for (const auto & trader: traderAgents) {
            trader->updateMarket(market);
        }
        market->updateTraders(traderAgents);
        std::vector<Agent*> agents = {};
        agents.push_back(market);
        agents.insert(agents.end(), traderAgents.begin(), traderAgents.end());
        Simulate simulation(agents, totalRounds);
        simulation.run();
    }
}

void DMAEcon(int totalRounds){
    DMAMarket* market = new DMAMarket(0);
    int traderIdOffset = 1;

    std::vector<int> simTraders = {999, 9999, 99999};
    // std::vector<int> simTraders = {99999};
    
    std::vector<DMATrader*> traderAgents = {};
    for (const auto & totalTraders: simTraders) {
        
        // Initialize trader and market agents
        int i = 0;
        while (i < totalTraders) {
            traderAgents.push_back(new DMATrader(i+traderIdOffset));    
            i++;
        }
        for (const auto & trader: traderAgents) {
            trader->updateMarket(market);
        }
        market->updateTraders(traderAgents);
        std::vector<Agent*> agents = {};
        agents.push_back(market);
        agents.insert(agents.end(), traderAgents.begin(), traderAgents.end());
        Simulate simulation1(agents, totalRounds);
        simulation1.run();
    }
}

