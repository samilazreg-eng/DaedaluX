# Test build after Phase 4Bis: validation record

Validation record for [#141](https://github.com/samilazreg-eng/DaedaluX/issues/141), Phase 4Bis (test build modernization). It records how DaedaluX configures and builds with testing ON and OFF once [#144](https://github.com/samilazreg-eng/DaedaluX/pull/144) (GoogleTest, #80) and [#145](https://github.com/samilazreg-eng/DaedaluX/pull/145) (test targets, #142) are applied, and it checks the exit criteria of #141 one by one. This report changes nothing in the build.

[tests/README.md](../tests/README.md) is the contract of the test build. This report is the evidence that the build follows it.

Findings are classified as in [build-characterization.md](build-characterization.md):

- **OBSERVED**: reproduced by a command recorded in this investigation.
- **INFERRED**: interpretation derived from observations.
- **UNKNOWN**: not established by the current evidence.

## Setup

- Tree: `main` at `a827c41`, which contains #144, with the branch of #145 (`0d8893e`) merged. Its tree object is `1328a8328aca99ea9243f4f8dfd362fa3dc9e5bf`. This report is the only file added on top of it.
- Reference: `8f71c64`, the commit of `main` before the two pull requests, built the same way. It is called the reference below.
- Environment: WSL Ubuntu 26.04.1 (x86_64), Ninja 1.13.2, GNU Make 4.4.1, GNU ld 2.46, GCC 15.2.0, Clang 21.1.8, Flex 2.6.4, Bison 3.8.2.
- CMake: 4.2.3 from the distribution, and 3.21.0 from the official binary archive. 3.21 is the declared minimum.
- CI: the workflow of the two pull requests, on `ubuntu-24.04` with CMake 3.31.6, Ninja 1.13.2 and GCC 13.3.0.
- Every tree is a `git archive` export of the commit into an empty directory, configured in a fresh build directory, in Debug, and built with `cmake --build --parallel 8`.
- Evidence comes from the build's own records:
  - the build graph, from the CMake file API (`codemodel-v2`): for each target its type, sources, definitions, include directories, flags, link line, dependencies and install rule;
  - the test registration, from `ctest --show-only=json-v1`: for each test its name, command, labels, timeout and working directory;
  - the installed files, from `cmake --install` into an empty prefix, and the packaged files, from `cpack -G TGZ`;
  - the test results, from a serial `ctest --timeout 120 --output-junit`, compared test by test.
- `ctest` and `cpack` are the ones of the CMake that configured the tree.
- Runs without network use `unshare -rn`, a user namespace without network.

## Dependency providers

| Dependency | Needed by | Provider | Testing OFF |
|---|---|---|---|
| CUDD 4.0.0 | product | the environment: an installed CMake package, found through `CMAKE_PREFIX_PATH` (`cudd_DIR` is `<prefix>/lib/cmake/cudd`), one prefix per compiler | same |
| Flex 2.6.4, Bison 3.8.2 | product | the environment | same |
| GoogleTest 1.14.0 | tests | the build: `FetchContent` clones commit `f8d7d77c06936315286eb55f8de22cd23c188571` into `<build>/_deps`, or uses the directory given by `FETCHCONTENT_SOURCE_DIR_GOOGLETEST` | not acquired |

## Short answer

- **Testing OFF is a product-only graph.** Nine targets, all product. No `_deps` directory, no `tests` directory, no GoogleTest entry in the cache or in the graph. It configures without network.
- **Testing ON adds the test graph and nothing to the product.** The nine product targets have the same sources, definitions, include directories, flags and link line with testing ON and OFF, and the installed and packaged files are the same 150.
- **One function creates the test executables.** The 35 test executables come from one `add_executable` and one `gtest_discover_tests`, both in `daedalux_add_test`, and share one set of requirements.
- **The registration is preserved.** 154 tests, with the names, arguments, labels, timeouts and working directories of the reference. 40 are labelled `unit` and 114 `integration`, and all have a 120 s timeout.
- **GoogleTest is pinned, kept out of the installation, and has an offline route.** GoogleMock is not built.
- **It is reproducible in the environments tried.** The same targets, registration and test results with CMake 4.2.3 and Ninja, CMake 3.21.0 and Unix Makefiles, GCC and Clang, with network and without.
- **Test results do not change.** 126 passed, 25 failed, 3 disabled in every tree, the same named outcomes as the reference with the same compiler.

## Findings

### Configure and build

- **OBSERVED:** every configure and every build of the matrix succeeds, and no configure log contains a warning.

| Tree | CMake | Generator | Compiler | Network | Targets | CTest tests | Installed files | GoogleTest files installed |
|---|---|---|---|---|---|---|---|---|
| reference, ON | 4.2.3 | Ninja | GCC | yes | 78 | 154 | 202 | 52 |
| reference, OFF | 4.2.3 | Ninja | GCC | yes | 9 | none | 150 | 0 |
| ON | 4.2.3 | Ninja | GCC | yes | 76 | 154 | 150 | 0 |
| OFF | 4.2.3 | Ninja | GCC | yes | 9 | none | 150 | 0 |
| ON | 3.21.0 | Unix Makefiles | GCC | yes | 76 | 154 | 150 | 0 |
| OFF | 3.21.0 | Unix Makefiles | GCC | yes | 9 | none | 150 | 0 |
| ON | 4.2.3 | Ninja | Clang | yes | 76 | 154 | 150 | 0 |
| OFF | 4.2.3 | Ninja | Clang | yes | 9 | none | 150 | 0 |
| ON, local GoogleTest copy | 4.2.3 | Ninja | GCC | no | 76 | 154 | 150 | 0 |
| OFF | 4.2.3 | Ninja | GCC | no | 9 | none | 150 | 0 |

- **OBSERVED:** the 76 targets with testing ON are 36 executables (`daedalux_cli` and 35 tests), 7 object libraries, 4 static libraries (`daedalux_lib`, `daedalux_test_support`, `gtest`, `gtest_main`) and 29 utility targets (`daedalux_test_data` and the 28 dashboard targets of `include(CTest)`). The reference has two more: `gmock` and `gmock_main`.
- **OBSERVED:** the TGZ package holds the same files as the installation in every tree.
- **OBSERVED:** against `main` at `a827c41`, which already has #144, the tree differs only by #145. With testing ON, the registration, the installed and packaged files and the test results are identical, and so is the build graph apart from the names of the test targets and the order of the three sources of `daedalux_test_support`. With testing OFF, the build graph, the installed files and the packaged files are identical.
- **OBSERVED:** CI passes on #144 and on #145. It configures with testing OFF, and configures, builds and tests with testing ON.

### Testing OFF

- **OBSERVED:** the nine targets are `daedalux_cli`, `daedalux_lib` and the seven object libraries of `src/`. In the four OFF trees:
  - the build directory has no `_deps` and no `tests` directory;
  - `CMakeCache.txt` has no `FETCHCONTENT_*`, `GTest`, `BUILD_GMOCK` or `INSTALL_GTEST` entry;
  - the build graph does not mention GoogleTest.
- **OBSERVED:** a fresh OFF configure and build succeed without network.
- **OBSERVED:** the OFF graph, install list and package list are identical to those of the reference with testing OFF.
- **OBSERVED:** outside `tests/`, the only CMake line about testing is `if(BUILD_TESTING)` in the root `CMakeLists.txt`, after `include(CTest)`. No file under `src/` or `cmake/` mentions GoogleTest or `FetchContent`.
- **OBSERVED:** both presets of `CMakePresets.json` set `BUILD_TESTING` to `ON`. A product-only build needs `-DBUILD_TESTING=OFF` on the command line.
- **INFERRED:** GoogleTest is acquired and configured only by `tests/CMakeLists.txt`, which is read only when `BUILD_TESTING` is ON.

### Testing ON and the product

- **OBSERVED:** each of the nine product targets has the same sources, definitions, include directories, flags, link line and dependencies with testing ON and OFF.
- **OBSERVED:** only `daedalux_lib` and `daedalux_cli` have an install rule. The installed files and the packaged files are identical with testing ON and OFF.
- **OBSERVED:** 38 targets have a GoogleTest include directory: `gtest`, `gtest_main`, `daedalux_test_support` and the 35 test executables. No product target has one.
- **OBSERVED:** `gtest` and `gtest_main` have no CUDD include directory. This is the result of #116.
- **OBSERVED:** the two definitions of the test build, `DAEDALUX_TEST_DATA_DIR` and `DAEDALUX_TEST_RUNTIME_DIR`, are on `daedalux_test_support` only.

### Test executables

- **OBSERVED:** `tests/CMakeLists.txt` has one `add_executable` and one `gtest_discover_tests`, both in `daedalux_add_test`.
- **OBSERVED:** the 35 test executables have one source each and share one set of requirements:

  ```
  include  <source>/include
  isystem  <cudd prefix>/include
  isystem  <build>/_deps/googletest-src/googletest/include
  isystem  <build>/_deps/googletest-src/googletest
  flags    -g -std=c++20
  link     src/libdaedalux_lib.a
  link     tests/libdaedalux_test_support.a
  link     <cudd prefix>/lib/libcudd.a
  link     lib/libgtest.a
  ```

- **OBSERVED:** against the reference, each test executable keeps its source and these requirements. Its target is renamed after its path, for example `test_symbol` becomes `unit.core.symbol.test_symbol`.

### Source ownership and target names

Each probe adds one file to the tree and configures. None is committed.

| Probe | Result |
|---|---|
| `tests/unit/test_state.cpp`, next to `tests/integration/test_state.cpp` | **OBSERVED:** configures, with a second target `unit.test_state`. On the reference the configure fails in `add_executable`. |
| `tests/unit/core/helpers.cpp` | **OBSERVED:** the configure fails: `tests/unit/core/helpers.cpp is not a test source`, followed by the rule. On the reference it becomes a 36th executable with no case. |
| `tests/test_stray.cpp`, `tests/perf/test_speed.cpp` | **OBSERVED:** the configure fails with the same message. |
| `tests/unit/core.symbol/test_x.cpp`, `tests/unit/test my.cpp` | **OBSERVED:** the configure fails with the same message. |
| `tests/support/Unlisted.cpp` | **OBSERVED:** compiled by `daedalux_test_support`, and by no other target. |
| `tests/data/basic/sample.cpp` | **OBSERVED:** compiled by no target. |

- **INFERRED:** two test sources cannot get the same target. A target is the path of its source with dots for slashes, and a path that contains a dot before its extension is refused.

### CTest registration

- **OBSERVED:** 154 tests, 154 distinct names. The name, arguments, labels, timeout and working directory of each are identical to the reference. Only the path of the executable differs.
- **OBSERVED:** 40 tests have the label `unit` and 114 the label `integration`. All 154 have a timeout of 120 s. The label and the timeout are set in `daedalux_add_test`; the one entry added by hand, `TestWorkspaceTest.CasesInOneProcess`, uses the same `TEST_TIMEOUT`.
- **OBSERVED:** the registration is identical in the four ON trees.
- **OBSERVED:** `tests/CTestTestfile.cmake` includes 35 files, all written by `gtest_discover_tests`, and has one `add_test`, as on `main` at `a827c41`.
- **OBSERVED:** with a new test source `tests/unit/probe/test_symbol_table.cpp` that has a suite of its own, a plain `cmake --build` builds the new executable without a manual configure and registers its case with the label `unit` and a 120 s timeout. Once the file is removed, the next build drops its target and the registration is the recorded one again.
- **OBSERVED:** a test is named `Suite.Case`, without its executable, as on the reference. CTest accepts one name twice, and nothing in the build detects it. A check was written in #145 and removed there on the maintainer's decision: the project has no established risk of it.
- **OBSERVED:** two test sources have all their cases commented out, `test_bisimulation.cpp` and `test_temporalSymNode.cpp`. Their executables are built and register no test, on the reference as here. The 154 tests come from 33 executables.

### Test results

- **OBSERVED:** serial `ctest`: 126 passed, 25 failed, 3 disabled in the four ON trees. The named outcomes are identical to the reference with GCC for the three GCC trees, and to the reference with Clang for the Clang tree. The reference gives the same named outcomes with both compilers.

### GoogleTest

- **OBSERVED:** `<build>/_deps/googletest-src` is at `f8d7d77c06936315286eb55f8de22cd23c188571` in the three ON trees built with network.
- **OBSERVED:** without network, a fresh ON configure fails with `Could not resolve host: github.com`. With `-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=<checkout of that commit>` it configures, builds and tests, with the registration and the results of the trees built with network. `_deps` then holds only `googletest-build`.
- **OBSERVED:** no `gmock` or `gmock_main` target exists.
- **OBSERVED:** `-DINSTALL_GTEST=ON -DBUILD_GMOCK=ON` on the command line changes neither the install rules nor the targets (checked on the branch of #144, before its merge).

### Earlier results

- **OBSERVED:** #71: a configure with `BUILD_TESTING=OFF` succeeds, in the four OFF trees and in CI.
- **OBSERVED:** #88: every test has a label and a timeout, test sources are found by a `CONFIGURE_DEPENDS` glob, and `tests/CMakeLists.txt` is the only `CMakeLists.txt` under `tests/`.
- **OBSERVED:** #116: no CUDD include directory on the GoogleTest targets.

## Exit criteria of #141

| Criterion | Evidence | Status |
|---|---|---|
| #80: immutable acquisition, offline route, no GoogleMock, no install or package | [GoogleTest](#googletest), [Configure and build](#configure-and-build) | met by #144, merged |
| One creation and registration abstraction, no duplicated setup | [Test executables](#test-executables) | met by #145 |
| Source ownership documented and enforced | [Source ownership and target names](#source-ownership-and-target-names), `tests/README.md` | met by #145 |
| Same file names in different directories get distinct targets; unsupported names fail clearly | [Source ownership and target names](#source-ownership-and-target-names) | met by #145 |
| CTest inventory preserved, labels and timeouts centralized | [CTest registration](#ctest-registration) | met by #145 |
| Requirements stay on targets; the product does not depend on GoogleTest or test support | [Testing ON and the product](#testing-on-and-the-product) | met |
| Fresh ON and OFF configurations configure and build | [Configure and build](#configure-and-build) | met |
| OFF acquires no GoogleTest and has no test target | [Testing OFF](#testing-off) | met |
| Reproducible at the declared CMake baseline, without a stale cache | [Configure and build](#configure-and-build): CMake 3.21.0, fresh directories | met in the environments listed in [Setup](#setup) |
| #71, #88 and #116 intact | [Earlier results](#earlier-results) | met |

## Not established

- **UNKNOWN:** macOS, and generators other than Ninja and Unix Makefiles.
- **UNKNOWN:** a build with testing OFF on `ubuntu-24.04`. CI configures that tree and does not build it.
- **UNKNOWN:** CMake versions other than 3.21.0, 3.31.6 (CI) and 4.2.3.
- **UNKNOWN:** a GoogleTest copy at another commit. `FETCHCONTENT_SOURCE_DIR_GOOGLETEST` is used as given: CMake does not check what the directory holds.

## Observations outside this phase

Not changed here. Each is a possible follow-up.

- 28 of the 76 targets are the dashboard targets that `include(CTest)` creates (`Experimental`, `Nightly`, `Continuous` and their steps). Nothing in the repository uses them.
- `gtest_main` is built although no test links it since #143.
- Two test executables register no test (see [CTest registration](#ctest-registration)).
- CI does not build the OFF tree and does not check its content.

## Reproduction

`<cudd>` is a CUDD 4.0.0 prefix (`scripts/install-cudd.sh`). `<src>` is an export of the tree.

```bash
git archive <commit> | tar -x -C <src>

# Build graph: ask for the file API before configuring
mkdir -p <build>/.cmake/api/v1/query && touch <build>/.cmake/api/v1/query/codemodel-v2

# Testing ON, then OFF, each in a fresh directory
cmake -S <src> -B <build> -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=<cudd>
cmake -S <src> -B <build-off> -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=<cudd> -DBUILD_TESTING=OFF
cmake --build <build> --parallel 8

# Other environments
CC=clang CXX=clang++ cmake ... -DCMAKE_PREFIX_PATH=<cudd built with clang>
<cmake-3.21.0>/bin/cmake ... -G "Unix Makefiles"
unshare -rn cmake ... -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=<googletest checkout>

# Evidence
ls <build>/.cmake/api/v1/reply/                 # targets: target-*.json
(cd <build> && ctest --show-only=json-v1)       # registration
cmake --install <build> --prefix <empty dir>    # installed files
(cd <build> && cpack -G TGZ)                    # packaged files
(cd <build> && ctest --timeout 120 --output-junit results.xml)
git -C <build>/_deps/googletest-src rev-parse HEAD
```
