#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <daedalux/promela/symbol/vardef/varSymNode.hpp>  // include the header, not the .cpp

class VarSymNodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Optional: Setup code to run before each test case
    }

    void TearDown() override {
        // Optional: Teardown code to run after each test case
    }
};


TEST_F(VarSymNodeTest, CreateSymbolTest) {
    // Test case 1: Create symbol of type varSymNode::Type::T_NA
    varSymNode* symbol1 = varSymNode::createSymbol(varSymNode::Type::T_NA, 1, "symbol1", 0, nullptr);
    ASSERT_NE(symbol1, nullptr);
    EXPECT_EQ(symbol1->getType(), varSymNode::Type::T_NA);
    EXPECT_EQ(symbol1->getLineNb(), 1);
    EXPECT_EQ(symbol1->getName(), "symbol1");
    EXPECT_EQ(symbol1->getBound(), 0);
    EXPECT_EQ(symbol1->getInitExpr(), nullptr);

    // Test case 2: Create symbol of type T_BIT
    varSymNode* symbol2 = varSymNode::createSymbol(varSymNode::Type::T_BIT, 2, "symbol2", 1, nullptr);
    ASSERT_NE(symbol2, nullptr);
    EXPECT_EQ(symbol2->getType(), varSymNode::Type::T_BIT);
    EXPECT_EQ(symbol2->getLineNb(), 2);
    EXPECT_EQ(symbol2->getName(), "symbol2");
    EXPECT_EQ(symbol2->getBound(), 1);
    EXPECT_EQ(symbol2->getInitExpr(), nullptr);

    //Delete the symbols
    delete symbol1;
    delete symbol2;
}

// Test the constructor of varSymNode
TEST_F(VarSymNodeTest, ConstructorTest) {
    // Test case: Create a varSymNode with valid arguments
    varSymNode* varNode = varSymNode::createSymbol(varSymNode::Type::T_NA, 1, "varName", 8, nullptr);

    EXPECT_EQ(varNode->getType(), varSymNode::Type::T_NA);
    EXPECT_EQ(varNode->getLineNb(), 1);
    EXPECT_EQ(varNode->getName(), "varName");
    EXPECT_EQ(varNode->getInitExpr(), nullptr);
    EXPECT_EQ(varNode->getBound(), 8);

    // Cleanup
    delete varNode;
}


// Test getLowerBound and getUpperBound functions
// TEST_F(VarSymNodeTest, GetBoundsTest) {
//     varSymNode* varNodeBool = varSymNode::createSymbol(varSymNode::Type::T_NA, 1, "varBool", 1, nullptr);
//     varSymNode* varNodeByte = varSymNode::createSymbol(varSymNode::Type::T_BYTE, 1, "varByte", 8, nullptr);

//     EXPECT_ANY_THROW(varNodeBool->getLowerBound());
//     EXPECT_ANY_THROW(varNodeBool->getUpperBound());
//     EXPECT_ANY_THROW(varNodeByte->getLowerBound());
//     EXPECT_ANY_THROW(varNodeByte->getUpperBound());

//     // Cleanup
//     delete varNodeBool;
//     delete varNodeByte;
// }

// Test castTo function
TEST_F(VarSymNodeTest, CastToTest) {
    varSymNode* varNodeBool = varSymNode::createSymbol(varSymNode::Type::T_NA, 1, "varBool", 1, nullptr);
    varSymNode* varNodeByte = varSymNode::createSymbol(varSymNode::Type::T_INT, 1, "varByte", 1, nullptr);

    EXPECT_TRUE(varNodeBool->castTo(varNodeBool));
    EXPECT_FALSE(varNodeBool->castTo(varNodeByte));

    // Cleanup
    delete varNodeBool;
    delete varNodeByte;
}

// Test getSizeOf function
TEST_F(VarSymNodeTest, GetSizeOfTest) {
    varSymNode* varNodeBool = varSymNode::createSymbol(varSymNode::Type::T_INT, 1, "varBool", 8, nullptr);

    EXPECT_EQ(varNodeBool->getSizeOf(), 32);

    // Cleanup
    delete varNodeBool;
}

// Test printGraphViz function
TEST_F(VarSymNodeTest, PrintGraphVizTest) {
    // Mock an ofstream for testing purposes
    class MockOfstream : public std::ofstream {
    public:
        // Override the << operator for testing purposes
        MockOfstream& operator<<(const std::string& str) {
            // Perform any necessary checks/assertions on the string
            return *this;
        }
    };

    varSymNode* varNode = varSymNode::createSymbol(varSymNode::Type::T_INT, 1, "varBool", 8, nullptr);

    MockOfstream mockOfstream;

    varNode->printGraphViz(mockOfstream);
    // You may want to add expectations/assertions for the mock ofstream
}