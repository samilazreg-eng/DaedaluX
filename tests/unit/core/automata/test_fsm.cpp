#include <gtest/gtest.h>
#include <daedalux/core/automata.hpp>
#include <daedalux/core/symbol/symTable.hpp>
#include <cudd/cuddObj.hh>


class FSMTest : public ::testing::Test {
protected:
    void SetUp() override {
        symTable* table = new symTable("global", nullptr);
        myFSM = new fsm(table, ADD());
    }

    void TearDown() override {
        delete myFSM;
    }

    fsm* myFSM;
};

TEST_F(FSMTest, BuildFSMTransitionsTest) {
    // Create an FSM object
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

    std::list<fsmEdge*> edges = myFSM->getTransitions();

    EXPECT_EQ(3, edges.size());

    // Call the getEndTransitions function
    std::list<fsmEdge*> endTransitions = myFSM->getEndTransitions();

    // Check the result
    ASSERT_EQ(endTransitions.size(), 0);

    auto nodes = myFSM->getNodes(node1);

    EXPECT_EQ(nodes.size(), 3);

    myFSM->deleteTransition(edge1);

    EXPECT_EQ(2, myFSM->getTransitions().size());

    myFSM->deleteNode(node1);

    EXPECT_EQ(2, myFSM->getNodes().size());

    EXPECT_EQ(nullptr, myFSM->getNode(1));
    EXPECT_EQ(1, myFSM->getTransitions().size());
} 


TEST_F(FSMTest, BuildFSM_usingCreateEdgeTest) {
    // Create an FSM object
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

    // Create some fsmEdge objects
    myFSM->addTransition(edge1);
    myFSM->addTransition(edge2);
    myFSM->addTransition(edge3);

    std::list<fsmEdge*> edges = myFSM->getTransitions();

    EXPECT_EQ(3, edges.size());

    // Call the getEndTransitions function
    std::list<fsmEdge*> endTransitions = myFSM->getEndTransitions();

    // Check the result
    EXPECT_EQ(endTransitions.size(), 0);

    auto nodes = myFSM->getNodes(node1);

    // EXPECT_EQ(nodes.size(), 3);
} 