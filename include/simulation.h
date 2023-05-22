#ifndef SIMULATION_H
#define SIMULATION_H

#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>
#include <deque>
#include <climits>
#include <chrono>

class Message {
    private:
        std::vector<double> content;
    
    public:
        Message(const std::vector<double>& value) {
            this->content = value;
        }
        std::vector<double> getContent() const {
            return content;
        }
};

// Class declaration
class Agent {
private:
    std::deque<Message> mailbox;

public:
    int id;
    std::unordered_map<int, std::deque<Message>> outbox;

    // Constructor
    Agent(int number) {
        this->id = number;
    }

    void send(int rid, const Message & message) {
        auto it = this->outbox.find(rid);
        if (it != this->outbox.end()) {
            it->second.push_back(message);
        } else {
            std::deque<Message> msgs = {message};
            this->outbox.emplace(rid, msgs);
        }
        // std::cout << id << " sends a message to " << rid << std::endl;
        // printOutbox();
    }
    
    void addToMailbox(std::deque<Message>& messages) {
        this->mailbox.insert(this->mailbox.end(), messages.begin(), messages.end());
    }

    std::optional<Message> receive() {
        if (this->mailbox.size() > 0) {
            Message removedMessage = this->mailbox.front();
            this->mailbox.pop_front();
            // Message removedMessage = this->mailbox[0];
            // this->mailbox.erase(this->mailbox.begin());
            return removedMessage;
        } else {
            return std::nullopt;; // Return null in case mailbox is empty
        }
    }

    virtual int step() { 
        return 1; 
    }

    void printId() {
        std::cout << "Agent id is: " << id;
    }

    void printOutbox() {
        if (this->outbox.size() > 0) {
            for (const auto& pair : this->outbox) {
                int key = pair.first;
                const std::deque<Message>& messages = pair.second;

                std::cout << "Key: " << key << std::endl;
                for (const auto& message : messages) {
                    std::cout << "Message ";
                    std::vector<double> retrievedContent = message.getContent();
                    for (const auto& element : retrievedContent) {
                        std::cout << element << " ";
                    }
                }
            }
        } else {
            std::cout << id << " outbox is empty! " << std::endl;
        }
    }
};

class Simulate {
private:
    std::unordered_map<int, std::deque<Message>> collectedMessages;
    int currentRound = 0;

public:
    std::unordered_map<int, Agent*> indexedAgents;
    int maxRounds;

    // Constructors
    Simulate(){}

    Simulate(std::vector<Agent*> agents, int total) {
        for (auto agent : agents) {
            this->indexedAgents.emplace(agent->id, agent);
        }
        this->maxRounds = total;
    }

    bool hasAgent(int id) {
        return this->indexedAgents.count(id) > 0;
    }

    void printCollectedMessage() {
        std::cout << "Collected messages in round " << currentRound << std::endl;
        for (const auto& pair : this->collectedMessages) {
            int key = pair.first;
            const std::deque<Message>& messages = pair.second;
            std::cout << "Key: " << key << std::endl;
            for (const auto& message : messages) {
                std::cout << "Message ";
                std::vector<double> retrievedContent = message.getContent();
                for (const auto& element : retrievedContent) {
                    std::cout << element << " ";
                }
            }
        }
    }

    void run(){
        auto initTime = std::chrono::high_resolution_clock::now();
        std::cout << "Simulation has " << indexedAgents.size() << " agents " << std::endl;

        while (currentRound < maxRounds) {
            auto startTime = std::chrono::high_resolution_clock::now();
            int aggregatedProposedRound = INT_MAX;
            for (const auto & index_agent : indexedAgents) {
                // deliver messages to each agent
                index_agent.second->addToMailbox(this->collectedMessages[index_agent.first]);
                this->collectedMessages.erase(index_agent.first);
                // execute each agent for 1 round
                int proposedRound = index_agent.second->step();
                // collect sent messages from agent
                for (const auto & index_message: index_agent.second->outbox) {
                    for (const auto & msg: index_message.second) {
                        this->collectedMessages[index_message.first].push_back(msg);
                    }
                }
                // clear agents' outbox
                index_agent.second->outbox.clear();
                if (proposedRound < aggregatedProposedRound) {
                    aggregatedProposedRound = proposedRound;
                }
            }
            std::cout << "Round " << currentRound << " takes " << 
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << " ms" << std::endl;
            currentRound += aggregatedProposedRound;
        }
        std::cout << "Average time per round: " << 
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - initTime).count() / currentRound << " ms" << std::endl;
    }
};
#endif