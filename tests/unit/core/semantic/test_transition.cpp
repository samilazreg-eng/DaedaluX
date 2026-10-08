#include <gtest/gtest.h>
#include <list>
#include <cstdlib>
#include <memory>

#include <daedalux/core/semantic.hpp>

#include <daedalux/formulas/formulaCreator.hpp>
#include <daedalux/algorithm/utils/stateComparer.hpp>
#include <daedalux/promela/parser/promela_loader.hpp>
#include <daedalux/promela/semantic/variable/state/initState.hpp>

#include "../../../support/TestFilesUtils.hpp"


class TransitionTest : public ::testing::Test {
protected:
    void SetUp() override {
        symTable* table = new symTable("global", nullptr);
        myFSM = std::make_unique<fsm>(table, ADD());
    }

    void TearDown() override {}
    std::unique_ptr<fsm> myFSM;
};

TEST_F(TransitionTest, SampleNonUniformEmptyList) {
    std::list<transition*> transList;
    auto result = transition::sampleNonUniform(transList);
    EXPECT_EQ(nullptr, result);
}

TEST_F(TransitionTest, SampleNonUniformSingleElement) {
    std::list<transition*> transList;
    auto s = std::make_unique<composite>("testState");
    auto t = std::make_unique<compTransition>(s.get(), transList);
    t->prob = 0.5;
    transList.push_back(t.get());
    EXPECT_EQ(1, transList.size());
    EXPECT_EQ(0.5, t->getProbability());
    // transition* result = transition::sampleNonUniform(transList);
    // EXPECT_EQ(t.get(), result);
}

TEST_F(TransitionTest, SampleNonUniformMultipleElements) {
    std::list<transition*> transList;
    auto s1 = std::make_unique<composite>("testState");
    auto t1 = std::make_unique<compTransition>(s1.get(), transList);
    t1->prob = 0.3;
    EXPECT_EQ(0.3, t1->getProbability());
    transList.push_back(t1.get());
    auto s2 = std::make_unique<composite>("testState");
    auto t2 = std::make_unique<compTransition>(s2.get(), transList);
    t2->prob = 0.6;
    EXPECT_EQ(0.6, t2->getProbability());
    transList.push_back(t2.get());
    auto s3 = std::make_unique<composite>("testState");
    auto t3 = std::make_unique<compTransition>(s3.get(), transList);
    t3->prob = 0.1;
    transList.push_back(t3.get());
    EXPECT_EQ(0.1, t3->getProbability());

    EXPECT_EQ(3, transList.size());

    transition* result = transition::sampleNonUniform(transList);
    EXPECT_TRUE(result == t1.get() || result == t2.get() || result == t3.get());
}