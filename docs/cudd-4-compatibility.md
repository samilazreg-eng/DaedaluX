# DaedaluX against CUDD 4.0.0

Investigation report for [#122](https://github.com/samilazreg-eng/DaedaluX/issues/122), the first step of [#127](https://github.com/samilazreg-eng/DaedaluX/issues/127). It checks whether DaedaluX compiles, links and behaves the same with an external CUDD 4.0.0 from [`cuddorg/cudd`](https://github.com/cuddorg/cudd), found through its CMake package, instead of the vendored CUDD 3.0.0. The build on `main` is not changed. The trial was made on a throwaway branch that is not pushed.

Findings are classified as in [build-characterization.md](build-characterization.md):

- **OBSERVED**: reproduced by a command recorded in this investigation.
- **INFERRED**: interpretation derived from observations.
- **UNKNOWN**: not established by the current evidence.

## Setup

- DaedaluX: `95c3df6` (`main`), fresh `git clone` on ext4.
- CUDD: `cuddorg/cudd`, branch `4.0.0` at `d1857bf` (2025-12-12) and tag `4.0.0-rc2` at `0e7d4d4` (2025-10-12).
- Environment: WSL Ubuntu 26.04.1, CMake 4.2.3, Ninja 1.13.2, GNU ld 2.46, GCC 15.2.0 and Clang 21.1.8.
- CUDD is configured with its default options (static library, C++ API on), built, and installed in a prefix outside the DaedaluX tree. One prefix per ref, compiler and build type.
- DaedaluX is configured with `-DCMAKE_PREFIX_PATH=<prefix>`, Ninja, and built from a fresh build directory each time.
- Tests: `ctest --timeout 120`, serial, with `--output-junit`. Results are compared test by test.
- Baseline, measured here on `95c3df6` with the vendored CUDD 3.0.0, Debug: 121 of 146 pass, with GCC and with Clang. The 25 failures are the ones CI excludes by name.

## Short answer

- **Ref to pin:** the commit `d1857bfc59f4b09d0aafbdc408221a5a0ac8995e`, the head of the upstream branch `4.0.0`. There is no `4.0.0` tag. The only 4.0 tags are release candidates, and `4.0.0-rc2` cannot be used as a package: it does not install `cuddTargets.cmake`.
- **Version argument:** `find_package(cudd 4.0.0 CONFIG REQUIRED)`. It accepts this ref and rejects CUDD 3 and CUDD 5. It cannot tell `d1857bf` from `4.0.0-rc2`: both report 4.0.0.
- **Source changes:** two, 9 files and 15 lines. The include path becomes `<cudd/cuddObj.hh>` in seven public headers and one test. `tvl.cpp` stops including `cuddInt.h` (#117). No API difference was found.
- **Compilation:** DaedaluX compiles in C++20 against the 4.0 headers with GCC and with Clang. No compiler warning names a CUDD header.
- **Link:** one static archive. `cudd::cudd` carries the include directory and the archive, and nothing else.
- **Tests:** 121 of 146 pass in the four configurations tried, and no test changes status against the baseline.
- **Output:** no difference that can be attributed to CUDD in the files the tests write.

## Findings

### Upstream refs

- **OBSERVED:** `cuddorg/cudd` has no `4.0.0` tag and no `4.0.0` release. Its tags are `cudd-2.4.0` to `cudd-3.0.0`, `3.0.0`, `4.0.0-rc1` and `4.0.0-rc2`. GitHub marks `3.0.0` as the latest release. The default branch `main` is the 2016 import of CUDD 3.0.0.
- **OBSERVED:** The 4.0 work is on the branch `4.0.0`. Its head `d1857bf` is 100 commits after `4.0.0-rc2`. The repository's last push is dated 2025-12-12.
- **OBSERVED:** Both refs declare version 4.0.0 (`src/config.h` at `d1857bf`, `project(cudd VERSION 4.0.0)` at `4.0.0-rc2`).
- **OBSERVED:** `4.0.0-rc2` installs `cuddConfig.cmake` and `cuddConfigVersion.cmake` but no `cuddTargets.cmake`: its `CMakeLists.txt` has `install(TARGETS cudd EXPORT cuddTargets)` and no `install(EXPORT ...)`. `find_package(cudd)` against that prefix fails:

  ```
  CMake Error at <prefix>/lib/cmake/cudd/cuddConfig.cmake:38 (include):
    include could not find requested file:

      <prefix>/lib/cmake/cudd/cuddTargets.cmake
  ```

- **OBSERVED:** The export was added after `rc2`, by `99ed7ee` (2025-11-07, "Fix cmake target config logic for downstream find_package usage") and `4c2a914` (2025-11-14).
- **INFERRED:** No tag names a usable 4.0 package. The dependency has to be pinned by commit until upstream tags 4.0.0. A branch name is not a pin: `4.0.0` moves.

### The installed package

- **OBSERVED:** `d1857bf` installs eight files, the same list with GCC and with Clang:

  ```
  include/cudd/cudd.h
  include/cudd/cudd.hpp
  include/cudd/cuddObj.hh
  lib/cmake/cudd/cuddConfig.cmake
  lib/cmake/cudd/cuddConfigVersion.cmake
  lib/cmake/cudd/cuddTargets.cmake
  lib/cmake/cudd/cuddTargets-release.cmake
  lib/libcudd.a
  ```

- **OBSERVED:** A Debug build installs `lib/libcudd_debug.a` and `cuddTargets-debug.cmake` instead. Both build types can be installed in one prefix.
- **OBSERVED:** The package defines one target, `cudd::cudd`, a static imported library. Its only usage requirement is `INTERFACE_INCLUDE_DIRECTORIES "<prefix>/include"`. It has no compile definition and no link dependency.
- **OBSERVED:** Configure prints `Found CUDD version 4.0.0.` and `CUDD provides the imported target 'cudd::cudd'.`
- **OBSERVED:** The version file uses `SameMajorVersion`. Against the `d1857bf` prefix:

  | `find_package(cudd <version> CONFIG REQUIRED)` | Result |
  |---|---|
  | none, `4`, `4.0`, `4.0.0`, `4.0.0 EXACT`, `4.0.0...<5` | found |
  | `4.0.1`, `4.1` | rejected: the installed version is lower |
  | `3.0.0`, `5`, `3.0.0...4.0.0` | rejected |

- **OBSERVED:** Which archive a consumer gets depends on what the prefix holds:

  | Prefix holds | Consumer Debug | Consumer Release | Consumer RelWithDebInfo |
  |---|---|---|---|
  | Release only | `libcudd.a` | `libcudd.a` | `libcudd.a` |
  | Debug only | `libcudd_debug.a` | `libcudd_debug.a` | `libcudd_debug.a` |
  | both | `libcudd_debug.a` | `libcudd.a` | `libcudd_debug.a` |

- **INFERRED:** A prefix with the Release archive alone serves every DaedaluX build type. This is the simplest provisioning for #123.

### Headers and API

- **OBSERVED:** The package installs three headers, under `include/cudd/`. `cuddInt.h`, `mtr.h`, `epd.h`, `st.h` and `util.h` stay in upstream's `src/` and are not installed. There is no `dddmp` in the 4.0 tree.
- **OBSERVED:** `cudd.h` declares the same 484 `Cudd_*` names in 3.0.0 and in 4.0. The prototypes of the four functions DaedaluX needs (`Cudd_PrintInfo`, `Cudd_PrintMinterm`, `Cudd_ReadStdout`, `Cudd_SetStdout`) are identical.
- **OBSERVED:** `cuddObj.hh` is the 3.0.0 file with two changes: the classes and `defaultError` carry an export macro (`CUDD_SYMBOL_EXPORT`), and it includes `<cudd/cudd.h>` where 3.0.0 includes `"cudd.h"`. A `diff` that ignores the macro shows the include line and nothing else.
- **OBSERVED:** `CUDD_VERSION` is not in the installed headers. In 3.0.0 it is in `cuddInt.h`. DaedaluX does not use it.
- **OBSERVED:** DaedaluX (outside `src/libs`) uses no `dddmp`, `mtr`, `epd` or `st` function, although the vendored CUDD is configured with `--enable-dddmp`.
- **INFERRED:** For DaedaluX, CUDD 4.0 at `d1857bf` is CUDD 3.0.0 behind a new build system and a new header location. The API it uses did not change.

### Source changes

The trial applies three commits on top of `95c3df6`: 11 files, 20 insertions, 100 deletions. Each stage was built with `ninja -k 0` to collect every error.

| Stage | Change | Result |
|---|---|---|
| A | `find_package(cudd 4.0.0 CONFIG REQUIRED)` replaces the content of `cmake/dependencies/CUDD.cmake`. The modules and `daedalux_lib` link `cudd::cudd`. The `CUDD_project` dependencies and the CLI's CUDD items are removed. | **OBSERVED:** configure succeeds. 54 compile steps fail, all with `fatal error: cuddObj.hh: No such file or directory`, from the seven public headers. |
| B | `"cuddObj.hh"` becomes `<cudd/cuddObj.hh>` in the seven public headers and in `tests/unit/core/automata/test_fsm.cpp`. | **OBSERVED:** one compile step fails: `src/feature/tvl.cpp: fatal error: cuddInt.h: No such file or directory`. |
| C | `tvl.cpp`: the include of `<cuddInt.h>` is removed, the three writes to `DdManager::out` become `Cudd_SetStdout`, and the three `fclose` calls read the stream back with `Cudd_ReadStdout`. | **OBSERVED:** the build succeeds. |

- **OBSERVED:** Stage B changes 8 lines in 8 files. Stage C changes 7 lines in one file.
- **INFERRED:** Stage C is the change #117 asks for. It does not depend on CUDD 4.0: the two accessors exist in 3.0.0, so #117 can be merged first, on the vendored CUDD.
- **INFERRED:** Stage B cannot be merged before the build switch. The vendored tree has `cplusplus/cuddObj.hh` and no `cudd/cuddObj.hh`.
- **OBSERVED:** In stage A, each of the seven modules links `cudd::cudd` `PUBLIC`.
- **INFERRED:** The modules need it to get the include path, because the directory-wide `include_directories` is gone. A build without these links was not tried.

### Compilation and link

- **OBSERVED:** With the three stages applied, DaedaluX builds with GCC 15.2 (Debug and Release) and with Clang 21.1.8 (Debug). In the four build logs, no warning names a CUDD header.
- **OBSERVED:** A stand-alone C++20 program that uses `Cudd`, `BDD` and `ADD` compiles against the installed headers with `-Wall -Wextra -Wpedantic` and no warning, with both compilers.
- **OBSERVED:** The link line of `daedalux_cli` has the CUDD archive once, after `libdaedalux_lib.a`:

  ```
  ... src/libdaedalux_lib.a <prefix>/lib/libcudd.a
  ```

- **OBSERVED:** No compile command refers to `src/libs/cudd` or to `ext/cudd`, and the build tree has no `ext/cudd` directory. DaedaluX no longer configures or compiles CUDD.
- **OBSERVED:** 130 of the 134 compile commands have `<prefix>/include`. The four without it are GoogleTest's and GoogleMock's own sources. With the directory-wide `include_directories`, `gtest-all.cc` had the eight CUDD paths (`docs/cudd-consumers.md`).
- **OBSERVED:** `libcudd.a` needs four `libm` functions (`exp`, `log`, `pow`, `powl`) and no `pthread` function. `cudd::cudd` does not declare `libm`. A stand-alone program links with `g++`. Its object linked with `gcc` fails until `-lm` is added.
- **INFERRED:** DaedaluX links because the C++ compiler driver adds `-lm`. A C consumer of `cudd::cudd` would have to add it.
- **OBSERVED:** CUDD 4.0 is compiled with hidden visibility by default. In `libcudd.a`, 981 global functions have default visibility (468 `Cudd_*` functions and 513 C++ symbols of the wrapper) and 290 are hidden (the internal `cudd*`, `Epd*`, `st_*`, `Mtr_*` and `util_*` functions).
- **INFERRED:** Hidden visibility does not affect a static link. It would matter only if DaedaluX became a shared library that re-exports CUDD.
- **OBSERVED:** Without a CUDD installation, configure stops before generating anything:

  ```
  CMake Error at cmake/dependencies/CUDD.cmake:1 (find_package):
    Could not find a package configuration file provided by "cudd" (requested
    version 4.0.0) with any of the following names:

      cuddConfig.cmake
      cudd-config.cmake

    Add the installation prefix of "cudd" to CMAKE_PREFIX_PATH or set
    "cudd_DIR" to a directory containing one of the above files.
  ```

### Tests

- **OBSERVED:** Serial `ctest --timeout 120`, from a fresh build directory each time:

  | DaedaluX | Compiler | CUDD | Passed | Failed | Tests whose status differs from the baseline |
  |---|---|---|---:|---:|---:|
  | `95c3df6`, Debug | GCC 15.2 | vendored 3.0.0 | 121 | 25 | baseline |
  | `95c3df6`, Debug | Clang 21.1.8 | vendored 3.0.0 | 121 | 25 | baseline |
  | trial, Debug | GCC 15.2 | 4.0 `d1857bf`, Release archive | 121 | 25 | 0 |
  | trial, Debug | GCC 15.2 | 4.0 `d1857bf`, Debug archive | 121 | 25 | 0 |
  | trial, Release | GCC 15.2 | 4.0 `d1857bf`, Release archive | 121 | 25 | 0 |
  | trial, Debug | Clang 21.1.8 | 4.0 `d1857bf`, built with Clang | 121 | 25 | 0 |

- **OBSERVED:** The comparison is made test by test on the JUnit files: the same 121 tests pass, the same 25 fail, and the same 3 are disabled.
- **OBSERVED:** The Clang baseline gives the same result as the GCC baseline. The roadmap listed the Clang test result as not established.
- **INFERRED:** The CI gate needs no change for the switch: the 25 exclusions stay the same.

### Output

The tests run in `<build>/test_fixtures` and write files there. The directory of the GCC baseline and the one of the GCC trial (Debug) were compared after the test run.

- **OBSERVED:** Both hold 1,228 files, and 1,188 are byte-identical. The 40 others are 32 `cmake_test_discovery_*.json` files (they contain the build path), `fsm_graphvis`, and 7 of the 9 `trace_report_*.trace` files.
- **OBSERVED:** `fsm_graphvis` contains object addresses. It differs between two runs of the same binaries. With the addresses masked, it is identical in every pair compared, vendored against 4.0 included.
- **OBSERVED:** The trace files also differ between two builds that use the same CUDD:

  | Pair | Trace files with the same lines, of 9 |
  |---|---:|
  | vendored 3.0.0 vs 4.0 (GCC) | 7 |
  | vendored 3.0.0 vs 4.0 (Clang) | 8 |
  | vendored 3.0.0, GCC vs Clang | 6 |
  | 4.0 Release archive vs 4.0 Debug archive (GCC) | 9, in a different order for 5 |

- **INFERRED:** The differences in the trace files are not caused by the CUDD version: two builds with the same vendored CUDD differ more than a vendored build and a 4.0 build do.
- **UNKNOWN:** What makes the trace files vary from one build to the next. It was not investigated here.
- **OBSERVED:** No file named `cudd_info.txt`, `__printbool.tmp` or `products` exists in the build tree after the test run, with the vendored CUDD or with 4.0.
- **INFERRED:** No test runs `TVL::initBoolFct`, which writes `cudd_info.txt`, nor the three `tvl.cpp` functions changed in stage C.
- **UNKNOWN:** Whether `TVL::printBool`, `TVL::toString` and `TVL::printMinterms` write the same text with CUDD 4.0. The prototypes of the functions they call are unchanged, but no test in this run compares their output.

## Answers to the issue's questions

1. **Ref and version argument.** Pin the commit `d1857bfc59f4b09d0aafbdc408221a5a0ac8995e`. Use `find_package(cudd 4.0.0 CONFIG REQUIRED)`. The version argument protects against CUDD 3 and CUDD 5. It does not protect against `4.0.0-rc2` or an older commit of the branch, which report the same version. With `rc2` the failure is still at configure time.
2. **Source changes.** The include path in 8 files, and `tvl.cpp` (#117). No API difference, no compile definition.
3. **C++20, GCC and Clang.** It compiles with both.
4. **Link requirements.** A static archive, found through `cudd::cudd`. `libm` is needed and comes from the C++ driver. No threads library. A Release-only prefix serves all build types. In Debug the archive is `libcudd_debug.a`.
5. **Tests.** 121 of 146 in four configurations, the same tests as the baseline, with GCC and with Clang.
6. **Output.** Nothing attributable to CUDD in the files the tests write. The output of the three `tvl.cpp` print functions is not covered by the tests and stays unknown.

## Input for the next issues

- **#123 (provisioning):** build `d1857bf` with CUDD's default options, Release, and install it in a prefix. Clone by commit, not by branch or tag. Pass the prefix with `CMAKE_PREFIX_PATH`.
- **#124 (switch):** stages A and B, in one PR. #117 is stage C and can go first.
- **#117:** the change is 7 lines and was built and tested here against CUDD 4.0. It was not tested against the vendored 3.0.0.
- **Upstream:** two points could be reported to `cuddorg/cudd`. There is no `4.0.0` tag, and `cudd::cudd` does not declare `libm`. Neither blocks DaedaluX.

## Not established

- **UNKNOWN:** whether upstream will tag 4.0.0, and whether the branch `4.0.0` will change again.
- **UNKNOWN:** behaviour on inputs the test suite does not cover. The comparison is the test suite and the files it writes, nothing more.
- **UNKNOWN:** macOS, Linux ARM64, a shared `libcudd`, and linkers other than GNU ld.
- **UNKNOWN:** the installed DaedaluX package. `find_package(daedalux)` was not tried against the trial (#77).

## Reproduction

The scripts used for this report are not committed. The key commands are:

```bash
# CUDD 4.0 in a prefix
git clone https://github.com/cuddorg/cudd.git && git -C cudd checkout d1857bfc59f4b09d0aafbdc408221a5a0ac8995e
cmake -S cudd -B cudd/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PWD/cudd-prefix"
cmake --build cudd/build && cmake --install cudd/build
find cudd-prefix -type f | sort

# DaedaluX with the trial changes (appendix)
cmake -S DaedaluX -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$PWD/cudd-prefix"
cmake --build build --parallel
ctest --test-dir build --timeout 120 --output-junit junit.xml
ninja -C build -t commands daedalux_cli | tail -1
```

## Appendix: the trial changes

Not for merging as they are. #124 and #117 own the real changes.

`cmake/dependencies/CUDD.cmake`, whole file:

```cmake
find_package(cudd 4.0.0 CONFIG REQUIRED)
```

`src/CMakeLists.txt`:

```diff
+foreach(module algorithm core feature formulas mutants promela visualizer)
+    target_link_libraries(daedalux_${module} PUBLIC cudd::cudd)
+endforeach()
+
-add_dependencies(daedalux_core CUDD_project)
 (and the six other add_dependencies lines)

 target_link_libraries(
     daedalux_lib
     PUBLIC
-        CUDD::obj
-        CUDD::cudd
+        cudd::cudd
 )

 target_link_libraries(
     daedalux_cli
     PRIVATE
         daedalux_lib
-        CUDD::obj
-        CUDD::cudd
 )
```

Seven public headers and `tests/unit/core/automata/test_fsm.cpp`:

```diff
-#include "cuddObj.hh"
+#include <cudd/cuddObj.hh>
```

`src/feature/tvl.cpp`, in `printBool`, `toString` and `printMinterms`:

```diff
-#include <cuddInt.h>

-		formula.manager()->out = fopen("__printbool.tmp","w");
+		Cudd_SetStdout(formula.manager(), fopen("__printbool.tmp","w"));
 		Cudd_PrintMinterm(formula.manager(), formula.getNode());
-		fclose(formula.manager()->out);
+		fclose(Cudd_ReadStdout(formula.manager()));
```
