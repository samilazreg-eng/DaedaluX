# Building with the declared minimum CMake version

Investigation report for [#64](https://github.com/samilazreg-eng/DaedaluX/issues/64). It checks whether CMake 3.19, the version declared by `cmake_minimum_required`, can configure, build and test DaedaluX, and compares it with a current CMake. The build is not changed. Fixes are left to separate issues.

Findings are classified as in [build-characterization.md](build-characterization.md):

- **OBSERVED**: reproduced by a command recorded in this investigation.
- **INFERRED**: interpretation derived from observations.
- **UNKNOWN**: not established by the current evidence.

## Setup

- Commit: `e947f53` (`main`, after PR #68). The `CMakeLists.txt` line numbers below refer to this commit.
- Platform: WSL Ubuntu 26.04 (x86_64, glibc 2.43), GCC 15.2.0, Ninja 1.13.2, GNU Make, Bison 3.8.2, Flex 2.6.4.
- CMake versions: the official Kitware Linux x86_64 binaries of **3.19.0** (the exact declared minimum) and **3.19.8** (the last 3.19 patch release), both checked against Kitware's published SHA-256 files. The comparison uses the system **CMake 4.2.3**. The preset checks also use 3.20.0 and 3.21.0, from the same source. Each variant uses the `ctest` of the same CMake.
- Every variant starts from a fresh clone of `e947f53` and uses a Debug build (`-DCMAKE_BUILD_TYPE=Debug`) with the default targets. Tests run with `ctest --timeout 120 -j6`.

## Short answer

- **CMake 3.19 is a valid minimum for the plain CMake route.** With 3.19.0 and 3.19.8, a clean configure succeeds with no CMake warning, the default build succeeds (35 executables) with Ninja and with Unix Makefiles, and `ctest` discovers the same 146 tests as 4.2.3. It also gives the same result for every test: 121 pass, 25 fail (the known baseline failures).
- **The preset route needs CMake 3.21.** `CMakePresets.json` declares schema `"version": 3`. CMake 3.19 and 3.20 reject the file before reading it, although the file itself says `"cmakeMinimumRequired": 3.19.0`. The README's `cmake --preset release` / `cmake --build --preset release` instructions therefore do not work with the declared minimum.
- **`-DBUILD_TESTING=OFF` fails to configure on every CMake version tested**, not only on 3.19. `configure_package_config_file` is called without `include(CMakePackageConfigHelpers)`. It works only because GoogleTest, fetched when testing is on, includes that module.
- The remaining differences between 3.19 and 4.2.3 do not change the build result. They are the spelling of the C++ standard flag, relative or absolute paths in the Ninja commands, flag order in GoogleTest's commands, and 34 test-discovery JSON files that 4.2.3 writes into the test fixture directory.

## Test matrix

| Variant | Configure | Build | Tests (pass / total) |
|---|---|---|---|
| 3.19.0, Ninja | rc 0, 0 warnings | rc 0, 35 executables | 121 / 146 |
| 3.19.0, Unix Makefiles | rc 0, 0 warnings | rc 0, 35 executables | 121 / 146 |
| 3.19.8, Ninja | rc 0, 0 warnings | rc 0, 35 executables | 121 / 146 |
| 4.2.3, Ninja | rc 0, 0 warnings | rc 0, 35 executables | 121 / 146 |
| 4.2.3, Unix Makefiles | rc 0, 0 warnings | rc 0, 35 executables | 121 / 146 |
| 3.19.0 and 3.19.8, `cmake --preset=debug` | **rc 1**: `Unrecognized "version" field` | — | — |
| 3.20.0, `cmake --preset=debug` | **rc 1**: `Unrecognized "version" field` | — | — |
| 3.21.0, `cmake --preset=debug` + `cmake --build --preset=debug` | rc 0 | rc 0 | 121 / 146 after a fixture refresh (119 before it, see the test race below) |
| 4.2.3, `cmake --preset debug` | rc 0 | rc 0 | 121 / 146 |
| 3.19.0, Ninja, `-DBUILD_TESTING=OFF` | **rc 1**: `Unknown CMake command "configure_package_config_file"` | — | — |
| 4.2.3, Ninja, `-DBUILD_TESTING=OFF` | **rc 1**: same error | — | — |

In every row that ran the tests, `daedalux_cli --help` returns 0.

## Findings

### Configure, build and test with 3.19 (Q1–Q3)

- **OBSERVED:** CMake 3.19.0 configures a fresh clone with Ninja and with Unix Makefiles: return code 0, no `CMake Warning`, no `CMake Deprecation Warning`, no `CMake Error`. The compiler is identified as GNU 15.2.0, as with 4.2.3. The external CUDD project and the GoogleTest `FetchContent` download both work.
- **OBSERVED:** The default build (`cmake --build <dir> -j10`) returns 0 with both generators and produces `daedalux_cli` and the 34 test executables. With Ninja, the build log contains the same 75 compiler warnings as with 4.2.3 (same set after path normalization).
- **OBSERVED:** `gtest_discover_tests` works with 3.19: `ctest` lists 146 tests plus 3 disabled ones, as with 4.2.3. The pass/fail result of each test is identical across all six test runs in the matrix (3.19.0 ×2, 3.19.8, 4.2.3 ×3). 121 / 146 is also the count recorded when `baseline/2026-09` was verified.
- **OBSERVED:** 3.19.8 behaves like 3.19.0 on every point above.
- **INFERRED:** Nothing in the default configure, build and test path needs a CMake feature newer than 3.19.

### Differences from CMake 4.2.3 (Q4)

- **OBSERVED:** 3.19 spells the standard flag `-std=c++2a`, and 4.2.3 spells it `-std=c++20`. GCC 15 treats both the same.
- **OBSERVED:** With Ninja, 3.19 writes relative paths (`-I../include`) in the compile commands, and 4.2.3 writes absolute paths. After normalizing the paths and the standard flag, the 134 compile commands are identical, except for the position of `-std=c++20` in GoogleTest's own two objects.
- **OBSERVED:** CMake 4.2.3's `gtest_discover_tests` leaves 34 `cmake_test_discovery_*.json` files in the test working directory, which is the fixture copy `<build>/test_fixtures`. CMake 3.19 leaves none. The files do not affect the test results.
- **INFERRED:** None of these differences changes what is compiled or how the tests behave.

### Presets need CMake 3.21 (Q4, Q5)

- **OBSERVED:** `CMakePresets.json` declares `"version": 3` and `"cmakeMinimumRequired": { "major": 3, "minor": 19, "patch": 0 }`. It defines two configure presets and two build presets.
- **OBSERVED:** CMake 3.19.0 and 3.19.8 reject the file: `CMake Error: Could not read presets from <source>: Unrecognized "version" field`, rc 1. `cmake --list-presets` fails the same way. CMake 3.20.0 fails with the same message. CMake 3.21.0 configures and builds through the presets, and the tests give 121 / 146.
- **OBSERVED:** Two throwaway edits isolate the cause:
  - With `"version": 2` and no other change, CMake 3.20.0 configures and builds through the presets. 3.19.0 still rejects the file.
  - With `"version": 1` and the `buildPresets` removed, CMake 3.19.0 configures through `--preset=debug`.
- **INFERRED:** The file uses nothing from schema 3. Only its version number requires 3.21. Its build presets require 3.20 (schema 2). A preset file that works on 3.19 cannot have build presets at all.
- **OBSERVED:** CMake 3.19 accepts only `--preset=<name>`. With the README's spelling, `cmake --preset debug`, it treats `debug` as the source directory: `The source directory ".../debug" does not exist.` 3.20 accepts both spellings. 3.19 has no `cmake --build --preset`.
- **INFERRED:** `cmakeMinimumRequired` inside the preset file has no effect here. CMake validates the schema version first, so an older CMake stops before it reads the declared minimum.

### `BUILD_TESTING=OFF` fails on every version (not version-specific)

- **OBSERVED:** With `-DBUILD_TESTING=OFF`, configure stops at `CMakeLists.txt:297` with `Unknown CMake command "configure_package_config_file"`, on 3.19.0 and on 4.2.3.
- **OBSERVED:** `configure_package_config_file` and `write_basic_package_version_file` come from the `CMakePackageConfigHelpers` module, which `CMakeLists.txt` never includes. GoogleTest v1.14.0 includes it (`googletest/CMakeLists.txt:91`, under its default `INSTALL_GTEST=ON`), and the default build fetches GoogleTest only when `BUILD_TESTING` is on.
- **OBSERVED:** With a throwaway `include(CMakePackageConfigHelpers)` added before `include(CPack)`, CMake 3.19.0 configures and builds with `-DBUILD_TESTING=OFF`, and `daedalux_cli --help` returns 0.
- **INFERRED:** Testing is on by default, so the default build hides the missing include. It is a defect of the build file, independent of the CMake version. It also affects the package export studied in #67.

### Other version declarations

- **OBSERVED:** The README's dependency list says "CMake ≥3.16". This contradicts `cmake_minimum_required(VERSION 3.19)`: CMake 3.16 would stop at the first line.
- **OBSERVED:** `tests/CMakeLists.txt` also declares 3.19, but no `add_subdirectory` refers to it: the top-level file collects `tests/*.cpp` itself.
- **OBSERVED:** `src/libs/ltl2ba/CMakeLists.txt` declares `VERSION 3.0` and is not part of the build. Configured on its own, it is accepted by 3.19.0 and rejected by 4.2.3 (`Compatibility with CMake < 3.5 has been removed from CMake.`). This matters only if it is ever added to the build (see #22).
- **OBSERVED:** The CI workflow (`.github/workflows/cmake-multi-platform.yml`) uses the CMake that comes with `ubuntu-latest` and does not pin a version.
- **UNKNOWN:** Which CMake version CI uses, and whether CI has ever run the declared minimum. It triggers only on the `alpha` branch (#10).
- **UNKNOWN:** Whether a version older than 3.19 could build the project. It was not tested, because the first line rejects it and changing that line is out of scope.
- **UNKNOWN:** Behaviour on macOS and with Clang under 3.19. Only Linux with GCC was tested.

### Test race seen during the investigation (not version-specific)

- **OBSERVED:** One `ctest -j6` run (on the 3.21.0 preset build) gave 119 / 146: `MutantGenerationTest.GenerateMutantsFlows` and `GenerateManyMutantsFlows` failed with `syntax error, unexpected end of file`, because the fixture `test_fixtures/test_files/mutants/flows/flows.pml` had become empty. It stays empty for all later runs until the next configure copies the fixtures again. After a reconfigure restored the fixture, the same build gave 121 / 146 in a serial run, and the file stayed intact.
- **OBSERVED:** Both tests call `LTLClaimsProcessor::removeClaimFromFile` on that same file (`tests/integration/test_mutantgeneration.cpp:28`). The function reads the file, then reopens it with `trunc` and writes back what it read (`src/core/logic/ltl.cpp:95-113`). Running the two tests concurrently on the **3.19.0** build left the file empty in 1 of 40 runs.
- **INFERRED:** When the two tests overlap, one can read the file just after the other truncated it, and then write the empty content back. This is the same fixed-file pattern as #44, on a different file, and it is unrelated to the CMake version.

## Answers to the issue's questions

1. **Clean configure with 3.19:** yes, with 3.19.0 and 3.19.8, with Ninja and with Unix Makefiles, without warnings. It does not work through the presets (`cmake --preset`), which 3.19 rejects.
2. **Default targets:** all build with 3.19 (35 executables, rc 0).
3. **Tests:** discovered and run. The count (146 + 3 disabled) and each result (121 pass, 25 known failures) are identical to CMake 4.2.3.
4. **Version-specific failures:** only in `CMakePresets.json`: schema 3 needs 3.21, build presets need 3.20, and 3.19 does not accept the `--preset <name>` spelling used in the README. The `BUILD_TESTING=OFF` failure is not version-specific. The other differences (flag spelling, relative paths, discovery JSON files) do not change the result.
5. **Is 3.19 valid?** Yes, for the CMake-file route that the baseline is verified with. The repository's other declarations disagree with it: the presets need 3.21, and the README says 3.16. Each of these needs a decision (follow-up 1).

## Proposed follow-up issues

These are proposals for separate issues. Nothing here has been changed, and none has been opened.

1. **Align the presets with the declared minimum.** This needs a maintainer decision between three options:
   - (a) keep 3.19 and document that the presets need 3.21;
   - (b) set the schema to `"version": 2`, which gives 3.20 with no content change, and document that;
   - (c) raise `cmake_minimum_required` to 3.21, which changes the minimum.
   In every case, `cmakeMinimumRequired` inside the file should state the real value.
2. **Include `CMakePackageConfigHelpers` in `CMakeLists.txt`**, so that `-DBUILD_TESTING=OFF` configures. This is a confirmed defect on every CMake version, found here and related to #67.
3. **Fix the README's CMake requirement** ("≥3.16" → the declared minimum), with the preset note from item 1. This could be folded into #47.
4. **Make the two `MutantGenerationTest` Flows cases stop rewriting a shared fixture** (work on a per-test copy, or use a ctest `RESOURCE_LOCK`). This is a sibling of #44. It is active today: 1 in 40 concurrent runs, and the damaged fixture persists until the next configure.
5. **Test the declared minimum in CI** once CI runs on `main` (#10): a job with the official CMake 3.19.0 binary next to the runner's current CMake.

## Reproduction

The scripts used for this report are not committed. The key commands are:

```bash
v=3.19.0; t=cmake-$v-Linux-x86_64.tar.gz
curl -sSfLO https://github.com/Kitware/CMake/releases/download/v$v/$t
curl -sSfL https://github.com/Kitware/CMake/releases/download/v$v/cmake-$v-SHA-256.txt | grep " $t\$" | sha256sum -c -
tar xzf $t; C=$PWD/cmake-$v-Linux-x86_64/bin

git clone https://github.com/samilazreg-eng/DaedaluX.git src && git -C src checkout e947f53 && cd src
$C/cmake -S . -B _b -G Ninja -DCMAKE_BUILD_TYPE=Debug    # or -G "Unix Makefiles"
$C/cmake --build _b -j10
(cd _b && $C/ctest --timeout 120 -j6)
$C/cmake --preset=debug                                  # fails: Unrecognized "version" field
$C/cmake -S . -B _off -G Ninja -DBUILD_TESTING=OFF       # fails on every version: configure_package_config_file
```

The 3.20.0 and 3.21.0 archives are named `cmake-<v>-linux-x86_64.tar.gz` (lower case). The race in the last finding is reproduced by restoring `flows.pml` in `<build>/test_fixtures` and running `ctest -R 'MutantGenerationTest\.Generate(Many)?MutantsFlows$' -j2` in a loop.
