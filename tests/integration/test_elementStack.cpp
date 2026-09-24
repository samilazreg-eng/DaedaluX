#include "gtest/gtest.h"
#include <iostream>
#include <memory>

#include <daedalux/algorithm/elementStack.hpp>
#include <daedalux/core/semantic/variable/state/composite.hpp>

// Test fixture for the elementStack class
class ElementStackTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Optional: Setup code to run before each test
        s = new elementStack();
    }

    void TearDown() override {
        // Optional: Teardown code to run after each test
        delete s;
    }
    elementStack * s;
};

// Test the push and top functions
TEST_F(ElementStackTest, PushAndTop) {
    std::shared_ptr<state> initialState = std::make_shared<composite>("init_state");
    std::cout << "initialState: " << initialState->getFullName() << std::endl;
    s->push(initialState, 0);
    std::cout << "top: " << s->top()->current_state->getFullName() << std::endl;
    ASSERT_FALSE(s->empty());

    auto topElement = s->top();
    ASSERT_TRUE(topElement != nullptr);
    ASSERT_TRUE(topElement->init);
    ASSERT_EQ(topElement->depth, 0);
    ASSERT_EQ(topElement->current_state, initialState);

    // Cleanup
    s->pop();
}

// Test the isIn function
TEST_F(ElementStackTest, IsIn) {
    std::shared_ptr<state> initialState = std::make_shared<composite>("init_state");

    s->push(initialState, 0);
    ASSERT_TRUE(s->isIn(initialState->hash()));

    // Cleanup
    s->pop();
}

// Test the empty function
TEST_F(ElementStackTest, Empty) {
    ASSERT_TRUE(s->empty());

    std::shared_ptr<state> initialState = std::make_shared<composite>("init_state");
    s->push(initialState, 0);
    ASSERT_FALSE(s->empty());

    // Cleanup
    s->pop();
}

// Test the pop function
TEST_F(ElementStackTest, Pop) {
    std::shared_ptr<state> initialState = std::make_shared<composite>("init_state");
    std::cout << "initialState: " << initialState->getFullName() << std::endl;
    s->push(initialState, 0);
    ASSERT_FALSE(s->empty());

    s->pop();
    ASSERT_TRUE(s->empty());
    ASSERT_EQ(s->top(), nullptr);
}

