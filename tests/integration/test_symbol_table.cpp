#include <daedalux/promela/parser/promela_loader.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <string>

// The model is passed inline, so the test does not depend on the working directory.
static const std::string arrayModel = "int array[4];\n"
                                      "int i = 0;\n"
                                      "active proctype test(){\n"
                                      "  do\n"
                                      "  :: i < 4; array[i] = i; i++;\n"
                                      "  :: else; break;\n"
                                      "  od;\n"
                                      "}\n";

class SymbolTableTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    loader = std::make_unique<promela_loader>(arrayModel, nullptr);
    globals = loader->getSymTable()->getSubSymTab("global");
    ASSERT_NE(globals, nullptr);
  }

  std::string printedProgram() const { return stmnt::string(loader->getProgram()); }

  std::unique_ptr<promela_loader> loader;
  symTable * globals = nullptr;
};

TEST_F(SymbolTableTest, RenameUpdatesTableAndPrintedProgram)
{
  auto proc = globals->lookup("test");
  auto array = globals->lookup("array");
  ASSERT_NE(proc, nullptr);
  ASSERT_NE(array, nullptr);

  proc->setName("software");
  array->setName("array2");

  EXPECT_EQ(globals->lookup("software"), proc);
  EXPECT_EQ(globals->lookup("array2"), array);
  EXPECT_EQ(globals->lookup("test"), nullptr);
  EXPECT_EQ(globals->lookup("array"), nullptr);

  auto program = printedProgram();
  EXPECT_NE(program.find("proctype software("), std::string::npos) << program;
  EXPECT_NE(program.find("int array2[4]"), std::string::npos) << program;
  EXPECT_NE(program.find("array2[i] = i"), std::string::npos) << program;
  EXPECT_EQ(program.find("proctype test("), std::string::npos) << program;
}

TEST_F(SymbolTableTest, QualifiedLookupThroughProctype)
{
  auto pid = globals->lookup("test._pid");
  ASSERT_NE(pid, nullptr);
  EXPECT_EQ(pid->getName(), "_pid");
}

TEST_F(SymbolTableTest, QualifiedLookupMissesReturnNull)
{
  EXPECT_EQ(globals->lookup("nosuch._pid"), nullptr);  // unknown prefix
  EXPECT_EQ(globals->lookup("i._pid"), nullptr);       // prefix is not a complex symbol
  EXPECT_EQ(globals->lookup("test.nosuch"), nullptr);  // unknown member
}

TEST_F(SymbolTableTest, FullNameIsDotQualified)
{
  auto i = globals->lookup("i");
  ASSERT_NE(i, nullptr);
  EXPECT_EQ(i->getFullName(), globals->getFullNameSpace() + ".i");
}
