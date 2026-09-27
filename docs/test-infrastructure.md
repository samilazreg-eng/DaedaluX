# Test infrastructure and runtime assumptions

Investigation report for [#65](https://github.com/samilazreg-eng/DaedaluX/issues/65). It documents how the tests are configured, built, discovered and executed, what they need at run time, and where they share files. The build and the tests are not changed. Fixes are left to separate issues.

Findings are classified as in [build-characterization.md](build-characterization.md):

- **OBSERVED**: reproduced by a command recorded in this investigation.
- **INFERRED**: interpretation derived from observations.
- **UNKNOWN**: not established by the current evidence.

## Setup

- Commit: `46d8e82` (`main`, after PR #69). The line numbers below refer to this commit.
- Platform: WSL Ubuntu 26.04 (x86_64), GCC 15.2.0, CMake 4.2.3, Ninja 1.13.2. `cpp` and `java` are on `PATH`. `spin`, `ltl2ba` and a system GoogleTest are not installed.
- Every build starts from a fresh clone from GitHub and uses Ninja and `-DCMAKE_BUILD_TYPE=Debug`. Tests run with `ctest --timeout 120`.
- Three instruments, none of them committed:
  - a SHA-256 listing of `<build>/test_fixtures` before and after each step;
  - an `LD_PRELOAD` shim that logs every file opened for writing and every `system()` / `popen()` command. Each of the 146 enabled cases was run alone (`ctest -R '^Suite\.Case$'`), so every write can be attributed to one case;
  - `unshare -rn` (a user namespace without network) for the offline configure runs.

## Short answer

- **GoogleTest** v1.14.0 is cloned from GitHub by `FetchContent` during the **first configure** of each build directory. That configure fails without network. Later configures, the build and `ctest` work offline. An offline machine can pass a local checkout with `-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=<dir>`. `cmake --install` also installs GoogleTest and GoogleMock.
- **Test creation:** every `tests/**/*.cpp` becomes one executable (34) linked with `daedalux_lib` and `GTest::gtest_main`. After each link, `gtest_discover_tests` runs the executable to list its cases and registers **each case as its own CTest test, run in its own process**: 149 tests, 146 enabled and 3 disabled. `tests/CMakeLists.txt` and its subdirectories are not used.
- **Fixtures:** each configure deletes `<build>/test_fixtures` and copies `examples/test_files` and `examples/models` into it (1,086 files, 30 MB). The enabled tests use four directories of `test_files` and nothing from `models/` (28 MB).
- **Working directory:** CTest starts every test in `<build>/test_fixtures`, and the tests build their paths as `current_path() + "/test_files/..."`. `ltl2ba` is looked up at `<cwd>/../src/bin/ltl2ba`, which does not exist in the build tree. That accounts for **14 of the 25 baseline failures**. The copy of `ltl2ba` committed at `src/bin/ltl2ba` is an ARM64 executable.
- **Shared files:** 10 cases move to a private temporary directory: all 4 MutantOutputFolderTest cases and 6 of the 7 PromelaLoaderConcurrencyTest cases. The other 136 run in the shared fixture copy and write into it: 11 committed fixtures rewritten in place, 9 `*_mutants` folders, 9 `.trace` files, and `fsm_graphvis`. Nothing is cleaned up. Only a reconfigure resets the copy.
- **Parallel risks, reproduced:**
  - A fixture rewritten in place was **emptied under `ctest -j8`** and stayed empty until the next configure (`mutants/array_mutant.pml`, the same pattern as #72 on another file).
  - Once `ltl2ba` is found, the fixed file name `__formula.tmp` makes **every** parallel run of the 14 LTL cases fail between 5 and 11 of them. #22 and the `ltl.cpp` part of #5 are therefore coupled.
  - The full suite under `ctest -j6` gave 121 / 146 in 59 of 60 runs.
- **Other constraints:** no test has a timeout (CTest's default is 10,000,000 s), and no test has a label.

## Test pipeline

| Phase | What happens | Source |
|---|---|---|
| Configure | GoogleTest is cloned into `<build>/_deps/googletest-src` and added as a subproject | `CMakeLists.txt:247-253` |
| Configure | `<build>/test_fixtures` is deleted, then `examples/test_files` and `examples/models` are copied into it | `CMakeLists.txt:258-262` |
| Configure | `tests/*.cpp` is collected recursively | `CMakeLists.txt:263` |
| Build | One executable per source file, named after the file, linked with `daedalux_lib` and `GTest::gtest_main` | `CMakeLists.txt:264-267` |
| Build, after each link | The executable runs with `--gtest_list_tests` in `<build>/test_fixtures`, and the case list is written to `<build>/<name>[1]_tests.cmake` | `CMakeLists.txt:268-269` |
| Test | `ctest` runs each case as a separate process: `<build>/<exe> --gtest_filter=Suite.Case --gtest_also_run_disabled_tests`, in `<build>/test_fixtures` | generated |

## Findings

### GoogleTest acquisition

- **OBSERVED:** `FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.14.0)` followed by `FetchContent_MakeAvailable`. This happens only when `BUILD_TESTING` is on, which is the default. There is no `GIT_SHALLOW`, no archive URL or hash, and no `find_package` fallback.
- **OBSERVED:** The first configure makes a full clone, resolved to commit `f8d7d77c06936315286eb55f8de22cd23c188571`. The clone is 19.9 MB, 15.9 MB of which is the `.git` directory (4,279 commits). The whole configure, including the clone and the fixture copy, took 6.0 s here.
- **OBSERVED:** Offline results (`unshare -rn`):

  | Case | Result |
  |---|---|
  | Fresh build directory | rc 1. `git clone` is tried 3 times, then `Failed to clone repository` |
  | Populated build directory, reconfigure (also after touching `CMakeLists.txt`) | rc 0 |
  | Populated build directory, build `gtest_main` and a test | rc 0 |
  | Populated build directory, `cmake --fresh -DFETCHCONTENT_FULLY_DISCONNECTED=ON` | rc 0 |
  | Fresh build directory, `-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=<existing checkout>` | rc 0 |
  | Fresh build directory, `-DFETCHCONTENT_FULLY_DISCONNECTED=ON` | rc 1. FetchContent warns that the source directory is not populated, and configure then stops at `configure_package_config_file` (#71) |
  | Fresh build directory, `-DFETCHCONTENT_TRY_FIND_PACKAGE_MODE=ALWAYS` (no system GoogleTest) | rc 1, clone attempted |

- **INFERRED:** Network access is needed once per build directory, at its first configure. The build and the tests need none. The CI workflow has no cache step, so every CI run clones GoogleTest from GitHub.
- **INFERRED:** The dependency is pinned by a tag name, not by a commit hash. If the tag moved upstream, the dependency would change without any change in this repository.
- **OBSERVED:** GoogleTest's own options keep their defaults: `BUILD_GMOCK=ON` and `INSTALL_GTEST=ON`. GoogleMock is built, but no test uses it. `cmake --install <build> --prefix <p>` installs, next to DaedaluX, `include/gtest`, `include/gmock`, `libgtest.a`, `libgtest_main.a`, `libgmock.a`, `libgmock_main.a`, `lib/cmake/GTest` and pkg-config files. This is relevant to #67.
- **UNKNOWN:** Offline behaviour with CMake 3.19. Only 4.2.3 was tested offline. #64 showed that 3.19 downloads GoogleTest normally when online.

### Test executables and CTest registration

- **OBSERVED:** `file(GLOB_RECURSE ... tests/*.cpp)` finds 34 sources: 23 under `tests/integration`, 11 under `tests/unit`. Each one becomes an executable named after its file (`NAME_WE`), placed at the root of the build directory.
- **INFERRED:** Two test files with the same name in different directories would produce two targets with one name, and configure would fail. No such pair exists today.
- **OBSERVED:** `gtest_discover_tests(<name> WORKING_DIRECTORY <build>/test_fixtures)` uses the default `DISCOVERY_MODE POST_BUILD`. In `build.ninja`, the link step of each test runs `GoogleTestAddTests.cmake` with `TEST_WORKING_DIR=<build>/test_fixtures` and `TEST_DISCOVERY_TIMEOUT=5`. The generated `<name>[1]_include.cmake` is included by `<build>/CTestTestfile.cmake`.
- **OBSERVED:** `ctest -N` lists 149 tests: 146 enabled and the 3 `DISABLED_SpecificationWriterTest` cases, which ctest reports as `Disabled`. `test_bisimulation`, `test_temporalSymNode` and `test_specification_writer` register no enabled case.
- **OBSERVED:** Each case is its own CTest test and its own process. Example: `<build>/test_symbol_table "--gtest_filter=SymbolTableTest.LoadValidPromelaFile" "--gtest_also_run_disabled_tests"`. The only properties set are `WORKING_DIRECTORY`, `SKIP_REGULAR_EXPRESSION` and `DEF_SOURCE_LINE`. There is no `TIMEOUT` and no `LABELS`.
- **OBSERVED:** Running the 34 executables directly from `test_fixtures`, all cases in one process each, gives 119 passes and the same failures. The two missing passes are the other `TransitionTest` cases: they never run, because `SampleNonUniformMultipleElements` crashes the process (#35). Under CTest, a crash affects only its own case.
- **OBSERVED:** `ctest -V` prints `Test timeout computed to be: 10000000`, so there is no timeout unless `--timeout` is given. CI runs `ctest --output-on-failure --parallel` without it. **INFERRED:** A hanging case would block CI until the job's time limit.
- **OBSERVED:** The glob has no `CONFIGURE_DEPENDS`. After adding `tests/unit/test_probe65.cpp`, `cmake --build` returned 0 but built nothing new and registered 0 new tests. After a reconfigure, the probe was built and registered.
- **OBSERVED:** CMake 4.2.3 discovery leaves 34 `cmake_test_discovery_*.json` files in `test_fixtures`. CMake 3.19 leaves none (#64).
- **OBSERVED:** `tests/CMakeLists.txt`, `tests/unit/CMakeLists.txt` and `tests/integration/CMakeLists.txt` are never added with `add_subdirectory`. They describe another design: `find_package(GTest)`, `find_package(daedalux)`, `unit` / `integration` labels, and `TIMEOUT 120` for integration tests.
- **OBSERVED:** `CTestTestfile.cmake` and `DartConfiguration.tcl` at the repository root are tracked, although `.gitignore` lists both. They were generated by an in-source build in `/home/slazreg/Work/Research/Daedalux`. Running `ctest` in the source root fails with `Cannot create directory /home/slazreg/Work/Research/Daedalux/Testing/Temporary`.
- **OBSERVED:** `examples/test_files/run_tests.py` globs `./test/basic/*.pml` and runs `./deadalux`. Neither exists, and nothing calls the script. It is copied into the fixtures with the rest of `test_files`.

### Fixtures

- **OBSERVED:** Each configure, not each build, deletes `<build>/test_fixtures` and copies `examples/test_files` (2.1 MB) and `examples/models` (28 MB) into it: 1,086 files in total.
- **OBSERVED:** What the enabled tests use. Every model load runs `cpp < <file>`, so the logged commands show which files each case loads. Copied and compared files come from the test sources.

  | Source (`examples/`) | Destination (`<build>/test_fixtures/`) | Used by enabled tests |
  |---|---|---|
  | `test_files/basic` (57 files) | `test_files/basic` | `array.pml`, `flows.pml`: 8 suites |
  | `test_files/mutants` (46 files) | `test_files/mutants` | `array*.pml`, `flows/`, `minepump/`, `3Process/`, `trafficLight/`, `mutex/`: FormulaCreator, ModelAnalyzer, MutantGeneration, MutantHandler, StateComparer, TraceGenerator |
  | `test_files/ltl` (7 files) | `test_files/ltl` | `ltl.pml`, `liveness_1.pml`: LtlModelCheckerTest |
  | `test_files/appendClaimTest` (5 files) | `test_files/appendClaimTest` | `flows.pml` and the three `*_expected.pml`: LTLTransformerTest |
  | Other `test_files` entries (`atomic`, `exclusivity`, `features`, `minepump`, `mtype`, `print`, `reachability`, `variants`, `varref`, `warmingUp`, `lasso.pml`, `run_tests.py`) | same names | none |
  | `models/` (`adapro` 19 MB, `thread.zip` 8.5 MB, `windows`, `minepump`, `elevator`, `csv`) | `models/` | none. Only commented-out or disabled cases name `models/minepump` and `models/windows` |

- **OBSERVED:** The copy is not a build dependency. After an edit to `examples/test_files/basic/array.pml`, `cmake --build` left the copy unchanged. The next configure updated it.
- **OBSERVED:** One `ctest` run leaves in the copy: `fsm_graphvis`, 9 `trace_report_*.trace` files, 9 new `*_mutants` folders, and 7 committed fixtures with changed content (`mutants/array.pml`, `array_mutant.pml`, `array_never.pml`, `minepump/minepump.pml`, `3Process/threeProcess.pml`, `threeProcess_mutant.pml`, `trafficLight/trafficlight.pml`). A second run without a reconfigure changed the tree again: 4 mutant files were deleted, and 4 `.trace` files and `fsm_graphvis` got different content. The pass/fail result was the same.
- **INFERRED:** The fixtures a run sees depend on the runs before it in the same build directory. A reconfigure is the only reset. The `.trace` files differ from run to run, because the traces are generated randomly.

### Working directory and relative paths

- **OBSERVED:** Every test runs with `WORKING_DIRECTORY` set to `<build>/test_fixtures`. This was introduced by `b78cf38` (PR #38). Before that commit, tests ran in the build directory.
- **OBSERVED:** Tests turn fixture names into absolute paths with `std::filesystem::current_path() + "/test_files/..."`. They do this through `tests/TestFilesUtils.hpp` or inline. Run from `<build>`, `test_symbol_table` fails with `The fPromela file does not exist or is not readable!` (rc 1). Run from `<build>/test_fixtures`, it passes.
- **OBSERVED:** Relative outputs go to the working directory:
  - `fsm_graphvis`, on every model load (`src/promela/parser/promela_loader.cpp:121-124`, #15);
  - `trace_report_*.trace` (TraceGeneratorTest);
  - `__formula.tmp` (`src/core/logic/ltl.cpp:22`);
  - `trace/<n>.dot` (`src/visualizer/stateToGraphViz.cpp:31`). Here the open fails silently, because `trace/` does not exist.

  Mutants go next to their model, in `<dir>/<stem>_mutants` (#19).
- **OBSERVED:** `ltl2ba` is resolved as `current_path() / "../src/bin/ltl2ba"` (`src/core/logic/ltl.cpp:14`), which is `<build>/src/bin/ltl2ba`. The build does not create it. 14 cases fail with `Could not find the ltl2ba binary at <build>/test_fixtures/../src/bin/ltl2ba`: 5 FormulaTest, 3 FormulaCreatorTest and 6 LTLTransformerTest cases.
- **OBSERVED:** The repository tracks `src/bin/ltl2ba` (added in `d0d1089`, 2024-06-21). It is an ARM aarch64 Linux executable, and on x86-64 it fails with `Exec format error`. **INFERRED:** Before PR #38, tests ran in the build directory. With a build directory such as `<source>/build`, the lookup reached this binary, but it could not run on x86-64.
- **OBSERVED:** `ltl2ba` can be built from `src/libs/ltl2ba`, which is not part of the build. With CMake 4, this needs `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` (#64). With that build copied to `<build>/src/bin/ltl2ba`, the 14 cases pass serially, and the full serial run gives 135 / 146.
- **OBSERVED:** 10 cases move the process into a private `mkdtemp` directory and move it back afterwards: the 4 MutantOutputFolderTest cases and 6 of the 7 PromelaLoaderConcurrencyTest cases. The seventh, `ConcurrentLoadsInSameDirectoryDoNotMixModels`, stays in `<build>/test_fixtures`. Its forked children load inline sources and write `fsm_graphvis` there 160 times. Two PromelaLoaderConcurrencyTest cases also set `TMPDIR` inside the process. The loader's scratch directories are created as `$TMPDIR/daedalux-loader-*`. None was left in `/tmp` after the runs.

### External tools and environment

| Tool | Used by | Looked up at | Effect when missing |
|---|---|---|---|
| `cpp` | every model load (`promela_loader.cpp:95`) | `PATH` | the load exits with rc 1 |
| `ltl2ba` | LTL-to-never-claim translation (`ltl.cpp:9-45`) | `<cwd>/../src/bin/ltl2ba` | 14 cases fail (#22) |
| `spin` | `spinRunner::check` (`spinRunner.cpp:40`) | `PATH` | 2 SpinRunnerTest cases fail. `ModelIsIncorrect` passes only because `check` returns `false` (#41). `MutantHandlerTest.EnhanceSpecification_3Processes` runs `spin -V` 14 times and passes |
| `java` + `TVLParser.jar` | `tvl.cpp:74` | `./libs/tvl` | not reached by any test |

- **OBSERVED:** The only commands the enabled tests run are `cpp` and `spin -V`. No test contacts the network. The only environment variable they use is `TMPDIR`.
- **OBSERVED:** Causes of the 25 baseline failures: 14 `ltl2ba` not found, 6 mutant-analysis assertions (#39), 2 `LtlModelCheckerTest` results (#40), 2 SPIN missing (#41) and 1 segfault (#35).

### Shared files and parallel execution

- **OBSERVED:** Under `ctest -j`, cases of the same executable run concurrently. Every case starts in `<build>/test_fixtures`, and all but the 10 private-directory cases (see [Working directory and relative paths](#working-directory-and-relative-paths)) stay there. The per-case write logs show these shared paths:

  | Path in `test_fixtures` | Written by | Also used by | Status |
  |---|---|---|---|
  | `test_files/basic/array.pml` | TraceGeneratorTest (rewritten in place) | loaded by cases in 7 other suites (31 cases in total) | transient failure seen, see below |
  | `test_files/mutants/array.pml`, `array_mutant.pml`, `array_never.pml`, `3Process/threeProcess*.pml`, `minepump/minepump.pml`, `trafficLight/trafficlight*.pml` | FormulaCreator, MutantGeneration, MutantHandler, TraceGenerator (rewritten in place) | loaded by StateComparer, ModelAnalyzer and the writers themselves | `array_mutant.pml` emptied, see below |
  | `test_files/mutants/flows/flows.pml` | 2 MutantGenerationTest cases | — | emptied under `-j6` (#72) |
  | `*_mutants/` for `basic/array`, `flows`, `trafficlight`, `two_trafficlight`, `threeProcess` | 2 cases each | loaded by the same cases | overlapping writers |
  | `__formula.tmp` | 14 LTL cases | the same 14 cases | dormant until `ltl2ba` is found (#5, #22) |
  | `test_files/appendClaimTest/flows_temp.pml` | 3 LTLTransformerTest cases | — | dormant (#44) |
  | `fsm_graphvis` | 61 cases in 14 suites | nobody reads it | no effect on results (#15) |
  | `trace_report_*.trace` | one TraceGeneratorTest case per file | — | no conflict |

- **OBSERVED:** In-place rewrites (`LTLClaimsProcessor::removeClaimFromFile`, `src/core/logic/ltl.cpp:95-113`, which truncates and then rewrites):
  - Full suite, `ctest -j6` with a reconfigure before each run: 30 of 30 runs gave 121 / 146. `ctest -j16`: 20 of 20 gave 121 / 146.
  - Full suite, `ctest -j6` without a reconfigure: 29 runs gave 121 / 146, and 1 gave 120 / 146. `TraceGeneratorTest.LogTraceReportToScarletArrayWithGeneratedPredicates` failed. That case loads `basic/array.pml` and its mutants in `basic/array_mutants/`, and other TraceGeneratorTest cases rewrite both.
  - Five suites (FormulaCreator, Similarity, State, TraceGenerator, TraceReport), `ctest -j8`, 60 runs without a reconfigure:
    - runs 1-43: 41 runs gave the baseline result and 2 had transient extra failures;
    - from run 44: 4 more failures in every run, because `test_files/mutants/array_mutant.pml` had become 0 bytes.

    Every extra failure was `syntax error, unexpected end of file`. A reconfigure restored the file (204 bytes).
- **INFERRED:** This is the mechanism of #72, reaching more files. A case that reads during another case's rewrite sees an empty file, and fails once. A case that rewrites while the file is empty writes the empty content back, and every later run fails until a reconfigure. The full suite rarely hits it because the suites overlap less. CI runs `ctest --parallel`, so it is exposed.
- **OBSERVED:** `__formula.tmp` is created, read and deleted in the working directory by every translation (`ltl.cpp:22-43`). With `ltl2ba` in place, the 14 cases under `ctest -j6` failed 5 to 11 cases in each of 20 runs, and passed in 5 of 5 serial runs. The failures are `Could not open the never claim file at <build>/test_fixtures/__formula.tmp`, an empty result, or another case's never claim.
- **INFERRED:** Fixing #22 alone would replace 14 deterministic failures with non-deterministic ones under `ctest -j`. The `ltl.cpp` part of #5 has to land with it. #5 calls this race dormant, because only `check` and `checkFormula` reach it. The test suite also reaches it, as soon as `ltl2ba` is found.
- **OBSERVED:** Disk use per build directory: 30 MB of fixtures and a 20 MB GoogleTest checkout.

## Acceptance criteria

- [x] GoogleTest acquisition and its network requirements are documented: [GoogleTest acquisition](#googletest-acquisition).
- [x] Test creation and CTest registration are explained: [Test pipeline](#test-pipeline), [Test executables and CTest registration](#test-executables-and-ctest-registration).
- [x] Fixture sources and build-tree destinations are listed: [Fixtures](#fixtures).
- [x] Working-directory assumptions are documented: [Working directory and relative paths](#working-directory-and-relative-paths), [External tools and environment](#external-tools-and-environment).
- [x] Shared-file or parallel-test risks are identified: [Shared files and parallel execution](#shared-files-and-parallel-execution).

## Proposed follow-ups

None of these has been opened. They wait for the maintainer's decision.

Severity uses the project's S0–S4 scale, which rates the risk to a buildable, testable and reproducible baseline, not how ugly the code is:

- **S0:** blocks building, running, testing or reproducing.
- **S1:** critical correctness or reliability problem: crash, undefined behaviour, data race, wrong results or data loss.
- **S2:** major technical debt.
- **S3:** improvement opportunity.
- **S4:** cosmetic.

| Kind | Target | Severity | Subject |
|---|---|---|---|
| Comment | #22 | S2 | The committed `src/bin/ltl2ba` is ARM64. The lookup accounts for 14 of the 25 baseline failures, and a local build of `ltl2ba` gives 135 / 146 serially |
| Comment | #5 | S2 | The tests reach `__formula.tmp` once `ltl2ba` is found: 20 of 20 parallel runs failed. Land it with #22 |
| Comment | #15 | S3 | 61 cases write `fsm_graphvis` into the shared fixture directory |
| New issue | — | S2 | Tests rewrite 11 committed fixtures in place (4 suites). `mutants/array_mutant.pml` was emptied under `ctest -j8` and stayed empty. This generalizes #72 |
| New issue | — | S3 | Fixture lifecycle: the copy is refreshed only by a configure, test output accumulates across runs, and 28 MB of `models/` are copied but unused |
| New issue | — | S3 | Test registration: no timeout (CI included), no labels, and a glob without `CONFIGURE_DEPENDS`. Remove or replace the unused `tests/**/CMakeLists.txt` |
| New issue | — | S3 | GoogleTest acquisition: network at the first configure, full clone, tag pin, installed with DaedaluX, GoogleMock built but unused. Document the offline route. Related to #67 and #71 |
| New issue | — | S4 | Remove the tracked root `CTestTestfile.cmake` and `DartConfiguration.tcl`, and the stale `examples/test_files/run_tests.py` |

## Reproduction

The scripts used for this report are not committed. The key commands are:

```bash
git clone https://github.com/samilazreg-eng/DaedaluX.git src && git -C src checkout 46d8e82
cmake -S src -B b -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build b -j10
(cd b && ctest --timeout 120 -j6)
ctest --test-dir b -N -V -R '^SymbolTableTest' | grep -E 'Test command|Working Directory'
unshare -rn cmake -S src -B offline -G Ninja                  # fails: GoogleTest clone
unshare -rn cmake -S src -B offline2 -G Ninja -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=$PWD/b/_deps/googletest-src
file src/src/bin/ltl2ba                                       # ARM aarch64
cmake -S src/src/libs/ltl2ba -B l2b -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && cmake --build l2b
mkdir -p b/src/bin && cp l2b/ltl2ba b/src/bin/                # then the 14 LTL cases pass serially
(cd b && ctest -R '^(FormulaTest\.neverClaim|FormulaCreatorTest\.formulaStringToNeverClaim|LTLTransformerTest\.)' -j6)
```

The fixture damage is reproduced by looping `ctest -j8 -R '^(TraceGeneratorTest|SimilarityTest|FormulaCreatorTest|StateTest|TraceReportTest)\.'` without a reconfigure until `find b/test_fixtures -name array_mutant.pml -empty` prints the file. `cmake -S src -B b` restores it.
