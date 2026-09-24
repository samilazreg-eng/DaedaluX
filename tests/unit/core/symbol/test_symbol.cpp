#include <daedalux/core/symbol/symTable.hpp>
#include <gtest/gtest.h>
#include <daedalux/core/symbol/symNode.hpp>
#include <daedalux/promela/symbol/vardef/intSymNode.hpp>
#include <daedalux/promela/symbol/vardef/boolSymNode.hpp>

class SymbolTestFixture : public ::testing::Test {
protected:
    // SetUp() is called before each test case
    void SetUp() override {
        // Optional: Setup code to run before each test case
    }

    // TearDown() is called after each test case
    void TearDown() override {
        // Optional: Teardown code to run after each test case
    }
};

// Test fixture for the constructor of the symbol class
TEST_F(SymbolTestFixture, ConstructorTest) {
    // Test case 1: Create a symbol with type and name
    auto s1 = new intSymNode(0, "symbol1");
    EXPECT_EQ(s1->getType(), symbol::Type::T_INT);
    EXPECT_EQ(s1->getName(), "symbol1");
    EXPECT_EQ(s1->getLineNb(), 0);

    // Test case 2: Create a symbol with type, line number, and string value
    auto s2 = new boolSymNode(1, "symbol2");
    EXPECT_EQ(s2->getType(), symbol::Type::T_BOOL);
    EXPECT_EQ(s2->getLineNb(), 1);
    EXPECT_EQ(s2->getName(), "symbol2");

    delete s1;
    delete s2;
}

// Test fixture for the parent-related functionality of the symbol class
TEST_F(SymbolTestFixture, ParentTest) {
    // Test case 1: Create a symbol and set its parent
    symTable* parent = new symTable("parent", nullptr);

    auto s = new intSymNode(0, "symbol1");
    s->setSymTable(parent);
    EXPECT_EQ(s->getSymTable(), parent);

    // Test case 2: Detach the symbol from its parent
    s->detach();
    EXPECT_EQ(s->getSymTable(), nullptr);

    delete s;
    delete parent;
}