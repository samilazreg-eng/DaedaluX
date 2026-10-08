#pragma once

#include <filesystem>

// Every test case runs in a private workspace: a new, empty directory below <build>/tests/runtime, which is the
// working directory while the case runs. What the case writes there is removed when it ends.

// Has GoogleTest create the workspace before each case and remove it afterwards. Called by main.
void installTestEnvironment();

// The workspace of the running case.
const std::filesystem::path & testWorkspace();
