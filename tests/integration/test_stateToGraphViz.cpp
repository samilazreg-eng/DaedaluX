#include <gtest/gtest.h>
#include <memory>
#include <daedalux/visualizer/stateToGraphViz.hpp>
#include <daedalux/core/semantic/variable/state/composite.hpp>
#include <daedalux/core/symbol/symTable.hpp>
#include <daedalux/core/automata/fsm.hpp>  // Include the necessary headers
#include <daedalux/core/automata/fsmEdge.hpp>
#include <daedalux/core/automata/fsmNode.hpp>


class StateToGraphVizTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize any common objects or variables needed for the tests
        auto table = std::make_unique<symTable>("global", nullptr);
        myFSM = std::make_unique<fsm>(table.get(), ADD());
        fsmNode * node1 = myFSM->createFsmNode(0, 1);
        fsmNode * node2 = myFSM->createFsmNode(1, 2);
        fsmNode * node3 = myFSM->createFsmNode(2, 3);

        EXPECT_EQ(node1, myFSM->getNode(1));
        EXPECT_EQ(node2, myFSM->getNode(2));
        EXPECT_EQ(node3, myFSM->getNode(3));
        EXPECT_EQ(nullptr, myFSM->getNode(30));
        
        EXPECT_EQ(3, myFSM->getNodes().size());

        // Create some fsmEdge objects
        fsmEdge* edge1 = new fsmEdge(node1, node2, nullptr, 10, false);
        fsmEdge* edge2 = new fsmEdge(node2, node3, nullptr, 20, false);
        fsmEdge* edge3 = new fsmEdge(node1, node3, nullptr, 30, false);

        node1->addTransition(edge1);
        node2->addTransition(edge2);
        node1->addTransition(edge3);

        node2->addInputTransition(edge1);
        node3->addInputTransition(edge2);
        node3->addInputTransition(edge3);

        // Add the edges to the FSM
        myFSM->addTransition(edge1);
        myFSM->addTransition(edge2);
        myFSM->addTransition(edge3);
    }

    void TearDown() override {
        // Clean up any common objects or variables used for the tests
    }

    std::unique_ptr<fsm> myFSM;
};

TEST_F(StateToGraphVizTest, Construction) {
    // Test the construction of stateToGraphViz object
    auto graphViz = std::make_unique<stateToGraphViz>(myFSM.get());
    // Add assertions as needed
}

TEST_F(StateToGraphVizTest, PrintGraphViz) {
    // Test the printGraphViz method with a valid state and depth
    auto graphViz = std::make_unique<stateToGraphViz>(myFSM.get());
    auto testState = new composite("test_variable");
    auto testDepth = 0;
    graphViz->printGraphViz(testState, testDepth);
    // Add assertions to verify the generated GraphViz content or file
}