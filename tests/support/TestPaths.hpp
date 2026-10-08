#pragma once

#include <string>

// The test data are the files of tests/data, named by their path below that directory.
// The build keeps a copy of them, which all cases share.

// The shared copy of a file. A test must not write to it, nor next to it.
std::string sharedTestFile(const std::string & name);

// A copy of a file in the workspace of the running case, made on first use.
// For code that rewrites the file or writes next to it.
std::string privateTestFile(const std::string & name);
