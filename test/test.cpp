#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "simulation.h"

TEST_CASE("MessageTests - content") {
    std::vector<double> msg1 = {1, 2, 3, 4};
    Message m1(msg1);
    std::vector<double> v2 = {1, 2, 3, 4};
    CHECK(*(m1.getContent()) == msg1);
    CHECK(*(m1.getContent()) == v2);
}

TEST_CASE("AgentTests - addToMailbox/receive") {
    Agent A1(42);
    std::vector<double> msg1 = {1, 2, 3, 4};
    Message m1(msg1);

    int expectedTotalMessages = 5;
    int i = 0;
    while (i < expectedTotalMessages) {
        std::deque<Message> messages = {m1};
        A1.addToMailbox(messages);
        i += 1;
    }
    
    int totalMessages = 0;
    std::optional<Message> m = A1.receive();
    while (m.has_value()) {
        const std::vector<double>* retrievedContent = m.value().getContent();
        CHECK(*retrievedContent == msg1);
        m = A1.receive();
        totalMessages +=1 ;
    }
    CHECK(totalMessages == expectedTotalMessages);
}

TEST_CASE("AgentTests - send/collect") {
    Agent A1(42);
    std::vector<double> msg1 = {1, 2, 3, 4};
    Message m1(msg1);
    int expectedTotalMessages = 5;
    for (int i = 0; i < expectedTotalMessages; i++) {
        A1.send(i, m1);
    }

    std::unordered_map<int, std::deque<Message>> collectedMessages = A1.outbox;
    std::set<int> expectedRids = {0,1,2,3,4};

    int totalMessages = 0;
    for (const auto& pair : collectedMessages) {
        int key = pair.first;
        CHECK(expectedRids.count(key) == 1);
        const std::deque<Message>& messages = pair.second;
        for (const auto& message : messages) {
            CHECK(*message.getContent() == msg1);
            totalMessages += 1;
        }
    }

    CHECK(totalMessages == expectedTotalMessages);
}

TEST_CASE("SimulateTests - init") {
    Agent A1(0);
    Agent A2(1);

    std::vector<Agent*> agents = {&A1, &A2};

    Simulate sim1(agents, 10);
    CHECK(sim1.hasAgent(0) == true);
    CHECK(sim1.hasAgent(1) == true);
    CHECK(sim1.hasAgent(2) == false);
}

TEST_CASE("SimulateTests - deliver messages") {
    std::vector<Agent*> agents2;
    Agent* A1 = new Agent(1);
    Agent* A2 = new Agent(2);

    agents2.push_back(A1);
    agents2.push_back(A2);

    std::vector<double> msg1 = {1, 2, 3, 4};
    Message m1(msg1);

    std::cout << std::endl;
    int expectedTotalMessages = 5;

    for (int i = 0; i < expectedTotalMessages; i++) {
        A1->send(2, m1);
    }

    Simulate sim1(agents2, 5);
    sim1.run();

    int totalMessages = 0;
    std::optional<Message> m = A2->receive();
    while (m.has_value()) {
        m = A2->receive();
        totalMessages +=1 ;
    }
    CHECK(totalMessages == expectedTotalMessages);
}