# Installation, package export, portability and reproducibility

Investigation report for [#67](https://github.com/samilazreg-eng/DaedaluX/issues/67). It installs DaedaluX into a clean prefix, tests the installed package with a minimal consumer, and records the platform and reproducibility assumptions of the build and of the tools at run time. The build is not changed. Fixes are left to separate issues.

Findings are classified as in [build-characterization.md](build-characterization.md):

- **OBSERVED**: reproduced by a command recorded in this investigation.
- **INFERRED**: interpretation derived from observations.
- **UNKNOWN**: not established by the current evidence.

## Setup

- Commit: `46d8e82` (`main`, after PR #69). The `CMakeLists.txt` line numbers below refer to this commit.
- Platform: WSL Ubuntu 26.04 (x86_64, glibc 2.43), GCC 15.2.0, Clang 21.1.8, CMake 4.2.3, Ninja 1.13.2, GNU Make 4.4.1, Bison 3.8.2, Flex 2.6.4, OpenJDK 25. SPIN, Graphviz and Docker are not installed.
- Each experiment starts from a fresh clone. The install and consumer tests use a Debug build (`-DCMAKE_BUILD_TYPE=Debug`, Ninja, default targets), installed with `cmake --install _b --prefix <prefix>`. The reproducibility tests use Release builds of the `daedalux_cli` target.
- Tests run with `ctest --timeout 120`, with `-j6` or serially as stated.

## Short answer

- **Install works, but the installed package cannot be used as is.** `cmake --install` succeeds. A consumer that calls `find_package(daedalux)` fails at configure time, because the exported target links to `CUDD::obj` and `CUDD::cudd`, which the package neither installs nor defines. This confirms the result of [cudd-consumers.md](cudd-consumers.md).
- **With CUDD supplied by the consumer, the package works.** A consumer that defines the two CUDD targets itself configures, builds and loads a Promela model, also after the prefix is moved. The exported target does not state that its headers need C++20.
- **The install and the CPack archives also ship GoogleTest.** 52 of the 202 installed files are GoogleTest's headers, libraries and package files. CPack ignores the project's own settings (generators, contact), because they are set after `include(CPack)`.
- **The umbrella header `daedalux.hpp` does not compile on Linux.** It includes `daedalux/Visualizer.hpp`, but the file is `visualizer.hpp`. It compiles on a case-insensitive file system.
- **The committed `src/bin/ltl2ba` is an ARM64 (aarch64) Linux executable.** It is also looked up relative to the working directory (#22). On the baseline, 14 of the 25 known test failures come from this lookup. With the committed x86-64 copy (`src/libs/bin/ltl2ba`) at the expected path, a serial run passes 135 / 146 tests instead of 121.
- **Builds are reproducible in time but not across paths.** Two Release builds at the same path, one minute apart, are byte-identical. A different checkout path changes `daedalux_cli` and `libcudd.a`, because CUDD is always compiled with `-g` and embeds its source paths. Configuring needs network access for GoogleTest.
- **Only Linux x86-64 has been verified.** The README also promises macOS. Several assumptions exclude Windows, and some are unverified on macOS and on ARM64 Linux.

## Installation contents

`cmake --install` of the Debug build (rc 0) writes 202 files:

| Path | Files | Content |
|---|---|---|
| `bin/daedalux_cli` | 1 | the CLI, 25.8 MB (Debug) |
| `lib/libdaedalux_lib.a` | 1 | the library, 68 MB (Debug) |
| `include/daedalux.hpp`, `include/daedalux/**` | 144 | all public headers, including the vendored `CLI11.hpp` |
| `lib/cmake/daedalux/` | 4 | `daedaluxConfig.cmake`, `daedaluxConfigVersion.cmake`, `daedaluxTargets.cmake`, `daedaluxTargets-debug.cmake` |
| `include/gtest/**`, `include/gmock/**` | 40 | GoogleTest and GoogleMock headers |
| `lib/libgtest*.a`, `lib/libgmock*.a` | 4 | GoogleTest and GoogleMock libraries |
| `lib/cmake/GTest/`, `lib/pkgconfig/` | 8 | GoogleTest's CMake package and four `.pc` files |

Not installed: CUDD's headers and `libcudd.a`, the test executables, the runtime tools (`ltl2ba`, `TVLParser.jar`), the examples and the documentation. There is no `daedalux.pc` for consumers that do not use CMake.

## Findings

### Package-consumer workflow

The consumer used for these tests is a `CMakeLists.txt` that calls `find_package(daedalux 1.0 REQUIRED)` and links `daedalux::daedalux_lib`, plus a `main.cpp` that loads a model through `promela_loader` and checks that an automaton was built. Its contents are in [Reproduction](#reproduction).

| Variant | Configure | Build | Run |
|---|---|---|---|
| Plain consumer | **rc 1**: `The link interface of target "daedalux::daedalux_lib" contains: CUDD::obj but the target was not found.` | — | — |
| Consumer defines `CUDD::cudd` and `CUDD::obj` from a DaedaluX build tree, default C++ standard | rc 0 | rc 0 | rc 0 |
| Same, with `CMAKE_CXX_STANDARD 20` | rc 0 | rc 0 | rc 0 |
| Same, prefix moved to another directory before configuring | rc 0 | rc 0 | rc 0 |
| Consumer built, then the prefix removed | — | — | rc 0 |
| Consumer run without `cpp` in `PATH` | — | — | **rc 1**: `Could not run the c preprocessor (cpp).` |

- **OBSERVED:** The exported `daedalux::daedalux_lib` has `INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"` and `INTERFACE_LINK_LIBRARIES "CUDD::obj;CUDD::cudd"`. `cmake/daedaluxConfig.cmake.in` defines no CUDD target, and its only `find_dependency` line is commented out.
- **OBSERVED:** The consumer works when it defines the two targets itself: `CUDD::cudd` pointing to `<build>/ext/cudd/build/cudd/.libs/libcudd.a` with `src/libs/cudd/cudd` as include directory, and `CUDD::obj` adding `src/libs/cudd/cplusplus`. CUDD's `config.h` is not needed. Nothing else has to be linked: `libdaedalux_lib.a` and `libcudd.a` are enough with GCC 15.
- **OBSERVED:** The package is relocatable. `daedaluxTargets.cmake` computes the prefix from its own location, and a consumer configured against a moved prefix builds and runs. The consumer is linked statically. `ldd` lists only `libstdc++`, `libm`, `libgcc_s` and `libc`, and it still runs after the prefix is deleted.
- **OBSERVED:** The package version file uses `AnyNewerVersion` with version 1.0.0. The export contains only the configuration that was installed (`daedaluxTargets-debug.cmake` for a Debug install).
- **OBSERVED:** At run time, `promela_loader` calls `cpp` through `system()`. Without `cpp` in `PATH`, the library prints an error and calls `exit(1)`, so the consumer process ends.
- **INFERRED:** Today the only working consumer workflow needs the DaedaluX build tree for CUDD. An installed prefix alone is not enough.

### Exported targets and public headers

- **OBSERVED:** The export set contains `daedalux::daedalux_lib` (static) and `daedalux::daedalux_cli` (executable). The seven object libraries are not exported. Their objects are inside `libdaedalux_lib.a`.
- **OBSERVED:** Each of the 144 installed headers was compiled on its own against the prefix (`g++ -fsyntax-only -I<prefix>/include`):

  | Include path and standard | Compile | Fail | First error |
  |---|---|---|---|
  | prefix only, `-std=c++20` | 114 | 30 | 30 × `cuddObj.hh: No such file or directory` |
  | prefix + CUDD source directories, `-std=c++20` | 143 | 1 | `daedalux.hpp`: `daedalux/Visualizer.hpp: No such file or directory` |
  | prefix only, `-std=c++17` | 100 | 44 | the 30 above, 11 × `'concept' does not name a type`, 2 × `uint64_t` undeclared, 1 × `runtime_error` not in `std` |

- **OBSERVED:** The exported target has no `INTERFACE_COMPILE_FEATURES`. The project sets `CMAKE_CXX_STANDARD 20`, which applies only to its own targets. A consumer that does not choose C++20 compiles with the compiler's default (C++17 for GCC 15), and 11 public headers then fail on `concept`.
- **OBSERVED:** `include/daedalux/visualizer/trace.hpp` uses `uint64_t` without `<cstdint>`, and `include/daedalux/algorithm/elementStack.hpp` uses `std::runtime_error` without `<stdexcept>`. Both compile with C++20 only because other standard headers include them.
- **OBSERVED:** `include/daedalux.hpp` includes `daedalux/Visualizer.hpp`, but the file is `include/daedalux/visualizer.hpp`. On ext4 the include fails. The same headers, copied to NTFS (case-insensitive) and compiled from there, compile. No source file or test includes `daedalux.hpp`, so the build never checks it. The line dates from `ec82629`.
- **OBSERVED:** `include/daedalux/CLI11.hpp` (the vendored CLI11 library) is installed as a public header under the `daedalux/` directory.
- **INFERRED:** Missing or incorrect exported dependencies: CUDD (the archive, its headers and a CUDD target are all missing) and the C++20 requirement. The standard-header gaps are latent: another standard library version may expose them.

### GoogleTest in the install and in the archives

- **OBSERVED:** The default build fetches GoogleTest v1.14.0 with `FetchContent` (`CMakeLists.txt:247-253`). GoogleTest's `INSTALL_GTEST` option defaults to `ON`, so `cmake --install` also installs GoogleTest's 52 files (see the table above).
- **OBSERVED:** `-DBUILD_TESTING=OFF` would avoid the download, but configure then fails (#71). A package without GoogleTest therefore cannot be built today without editing the build.

### CPack

- **OBSERVED:** `CMakeLists.txt:284` calls `include(CPack)` before it sets `CPACK_PACKAGE_NAME`, the version variables, `CPACK_GENERATOR "TGZ;ZIP"` and `CPACK_PACKAGE_CONTACT` (`CMakeLists.txt:285-290`). The generated `CPackConfig.cmake` contains `CPACK_GENERATOR "STGZ;TGZ;TZ"` (CMake's Linux default) and no `CPACK_PACKAGE_CONTACT`. The name and version still come out right, because CPack derives them from `project()`.
- **OBSERVED:** `cpack` (rc 0) produces `daedalux-1.0.0-Linux.sh`, `.tar.gz` and `.tar.Z`, not the `.tar.gz` and `.zip` the file asks for. Each archive contains the same 202 files as the install, including GoogleTest, built in whatever configuration the build directory uses.
- **OBSERVED:** The archive's layout is `daedalux-1.0.0-Linux/bin/daedalux_cli`. The README's "Prebuilt Archives" section describes `daedalux-<version>/build/daedalux` and a `.zip` (see #47).
- **INFERRED:** CPack reads its variables when `include(CPack)` runs, so the settings written after it have no effect.

### Portability assumptions

#### Build time

| Assumption | Where | Evidence |
|---|---|---|
| `yacc` and `flex` found in `PATH` by those exact names | `CMakeLists.txt:27-38` | **OBSERVED:** configure does not look for them. Without them, configure returns 0 and the first build fails (`flex: not found`, rc 127). Ninja always regenerates the parser (see [parser-regeneration.md](parser-regeneration.md)), so the tracked generated files do not help. Here `yacc` resolves to `/usr/bin/bison.yacc` (Bison 3.8.2). |
| A POSIX shell, `touch`, `make` and a C compiler for CUDD | `CMakeLists.txt:56-100` | **OBSERVED:** CUDD is built by its `configure` script and `make -j4`, also with the Ninja generator. |
| CUDD compiled by the compiler its `configure` finds, not the project's | `CMakeLists.txt:91-95` | **OBSERVED:** with `-DCMAKE_CXX_COMPILER=clang++`, CUDD's `Makefile` still uses `CC = gcc` and `CXX = g++`, with CUDD's own flags `-g -O3`. |
| Network access at configure time | `CMakeLists.txt:248-253` | **OBSERVED:** configure in a network namespace without network fails (`Could not resolve host: github.com`). It succeeds offline with `-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=<local copy>`. |
| A case-insensitive file system for the umbrella header | `include/daedalux.hpp:7` | **OBSERVED**, see above. |
| Short paths on Windows | repository | **OBSERVED** in #45: a Windows checkout fails on paths over 260 characters. |

#### Run time

| Tool or assumption | Used by | Evidence |
|---|---|---|
| `cpp` in `PATH` | every model load (`promela_loader.cpp:95`) | **OBSERVED:** without it the process exits with rc 1 (see above). |
| `ltl2ba` at `<working directory>/../src/bin/ltl2ba` | LTL-to-never-claim (`ltl.cpp:14`) | **OBSERVED:** the committed `src/bin/ltl2ba` is an `ELF 64-bit LSB pie executable, ARM aarch64` (added in `d0d1089`, 2024-06-21). On x86-64 it fails with `Exec format error` (rc 126). The x86-64 copy is `src/libs/bin/ltl2ba` (added in `ec82629`), and nothing refers to it. The path depends on the working directory (#22). |
| `java` and `./libs/tvl/TVLParser.jar` relative to the working directory | feature models (`tvl.cpp:74`) | **OBSERVED** statically. The jar is in `src/libs/tvl/` (#22). It could not be reached through the CLI: `check` exits before it (#21). |
| `spin` in `PATH` | `spinRunner.cpp:40, 67`, mutant analysis | **OBSERVED** in #41: the SpinRunner tests fail without SPIN. |
| `owl` in `PATH` | `formulaSimplifier.hpp:32` | **OBSERVED** statically: only a `which owl` check. The call itself is commented out. |
| POSIX APIs and `sh` | `mkdtemp`, `fork`/`getpid`, `popen`, `<unistd.h>`, `system()` | **OBSERVED** statically (#14). |
| LP64 data model | `main_cli.cpp:15-43` | **OBSERVED** statically: the CLI exits unless `short` is 2 bytes, `int` is 4, and `unsigned long`, `double` and pointers are 8. A Windows (LLP64) or 32-bit build would stop there. |
| Unaligned typed access to the state vector | `payload.hpp:62-90`, `payload.cpp:173-191` | **OBSERVED** statically: values are read and written through `reinterpret_cast<T*>(ptr + offset)` at arbitrary byte offsets. **INFERRED:** this relies on the hardware accepting unaligned loads (x86-64 and ARM64 do), and it breaks C++ aliasing and alignment rules. |

#### Effect of the `ltl2ba` lookup on the test suite

- **OBSERVED:** On the baseline (`ctest -j6 --output-on-failure`), 14 failing tests throw `Could not find the ltl2ba binary at <build>/test_fixtures/../src/bin/ltl2ba`. The tests run in `<build>/test_fixtures`, so the path points into the build directory, where no `ltl2ba` exists.
- **OBSERVED:** With a throwaway symlink `<build>/src/bin/ltl2ba` to the committed aarch64 binary, the result does not change (121 / 146): the binary cannot run on x86-64.
- **OBSERVED:** With the symlink pointing to the x86-64 `src/libs/bin/ltl2ba`, a serial run gives **135 / 146**. The 14 tests that now pass are in `FormulaTest` (5), `FormulaCreatorTest` (3), `LTLTransformerTest` (6). Under `-j6` the same build gives 128 / 146. `LTLTransformerTest.formulaStringToNeverClaim_Finally`, which fails under `-j6`, passes when run alone.
- **INFERRED:** Under `-j6` the LTL tests collide on files in the shared working directory: `__formula.tmp` (#5) and `appendClaimTest/flows_temp.pml` (#44).
- **OBSERVED:** The 11 remaining failures do not involve `ltl2ba`: `LtlModelCheckerTest` (2, #40), `SpinRunnerTest` (2, SPIN missing, #41), the six mutant-analysis cases of #39 (`FormulaCreatorTest` 3, `TraceGeneratorTest`, `SimilarityTest`, `StateComparerTest`), and the `TransitionTest` segfault (#35).

### Reproducibility

- **OBSERVED:** Two Release builds of a fresh clone at the **same path**, the second started 61 s after the first, give byte-identical `libdaedalux_lib.a`, `daedalux_cli`, `libcudd.a` and generated parser files. No source file uses `__DATE__`, `__TIME__` or `__TIMESTAMP__`, in DaedaluX or in the vendored CUDD.
- **OBSERVED:** A Release build at a **different path** gives an identical `libdaedalux_lib.a`, but a different `daedalux_cli`, `libcudd.a`, `y.tab.cpp` and `lex.yy.cpp`. `libcudd.a` contains the checkout path 552 times, and `daedalux_cli` contains it 14 times, all as CUDD source file names (for example `<src>/src/libs/cudd/cudd/cuddTable.c`). The parser files differ in their `#line` paths ([parser-regeneration.md](parser-regeneration.md)).
- **INFERRED:** CUDD is always built with `-g` and with absolute source paths, whatever `CMAKE_BUILD_TYPE` is. That makes the CLI depend on the checkout path. A Debug build of DaedaluX itself also embeds paths through its debug information (not measured).
- **OBSERVED:** Configure downloads GoogleTest by the tag `v1.14.0`, not by a commit hash, with no checksum. Tags can move, so the input is not pinned.
- **OBSERVED:** Several places depend on the working directory: the `ltl2ba` and TVL lookups (#22), the files written next to it (`__formula.tmp`, `__workingfile.tvl`, `fsm_graphvis`, `trace/*.dot`, #5 and #15), and the mutants folder next to the model. The loader's scratch files go to `std::filesystem::temp_directory_path()` (`$TMPDIR`).
- **OBSERVED:** Two runs of `gen-single-traces` (and of `gen-mutants`) on the same model give identical traces (and mutants), but a different `fsm_graphvis`: node IDs are memory addresses (`fsmNode::getID` and `symbol::getID` return `(unsigned long)this`). The IDs are used only in Graphviz output.
- **OBSERVED:** The repository commits `CTestTestfile.cmake` and `DartConfiguration.tcl` at its root. They are CMake-generated files that name the original author's machine and paths (`/home/slazreg/Work/Research/Daedalux`). The build does not use them.

Tool versions: the tracked parser files were made by Bison 3.8.2 and Flex 2.6.4 ([parser-regeneration.md](parser-regeneration.md)), and this investigation used the same versions. CUDD is vendored (3.0.0), GoogleTest is 1.14.0, and the declared CMake minimum is 3.19 ([cmake-minimum-version.md](cmake-minimum-version.md)). The C++ compiler must support C++20 concepts. The README says GCC ≥ 10 or Clang ≥ 11, which was not tested here.

### Platform declarations

- **OBSERVED:** The README describes "From Source (Linux & macOS)", with Homebrew instructions for macOS. It also lists Boost and GMP as prerequisites, which `CMakeLists.txt` does not use.
- **OBSERVED:** CI (`.github/workflows/cmake-multi-platform.yml`) runs only `ubuntu-latest`, Debug, and only on `alpha` (#10). Its matrix declares `compiler: [gcc, clang]`, but no step uses `matrix.compiler`, so both jobs build with the runner's default compiler.
- **OBSERVED:** `scripts/docker/build_image.sh` runs `docker build --file Dockerfile`, but the repository has no `Dockerfile` at its root (the file is `docker/Dockerfile`). `docker/Dockerfile` copies `./models`, `./test_files` and `test_scripts`, which do not exist at the root (the first two are under `examples/`). It builds a second CUDD 3.0.0 from a downloaded tarball, and SPIN 6.5.2. It uses `gcc:latest` and `fedora:latest`. `.devcontainer/Dockerfile` installs `byacc`, not `bison`.
- **UNKNOWN:** Whether the Docker image or the dev container builds. Docker is not available here. Given the missing paths, the image build would fail at the first missing `COPY`, but that was not run.
- **UNKNOWN:** What `yacc -y -d -o` does when `yacc` is `byacc`, as in the dev container.

## Confirmed defects and open decisions

### Confirmed defects

| Defect | Evidence |
|---|---|
| `find_package(daedalux)` fails: CUDD is neither installed nor defined by the package | consumer, rc 1 |
| The exported target does not require C++20 | 11 headers fail under the default C++17 |
| `daedalux.hpp` includes `Visualizer.hpp` (wrong case) | fails on ext4, compiles on NTFS |
| The install and the CPack archives contain GoogleTest | 52 of 202 files |
| CPack settings are ignored (set after `include(CPack)`) | `CPACK_GENERATOR "STGZ;TGZ;TZ"` in `CPackConfig.cmake` |
| The `ltl2ba` found by the tests is ARM64-only, and the x86-64 copy is never used | `file`; 121 → 135 serial with the x86-64 copy |
| Configure does not check for `yacc` and `flex` | configure rc 0, build rc 127 |
| CUDD ignores the chosen compiler and build type | `CC = gcc`, `-g -O3` under Clang and Release |
| The Docker scripts refer to paths that do not exist | static reading |
| Two public headers depend on transitive standard includes | C++17 compile |

### Open decisions (for the maintainer)

1. **Is the installed library a supported deliverable,** or only the CLI? Only the CLI works from an install today.
2. **CUDD in the public API:** install and export CUDD with the package, or stop exposing it in the public headers (already raised in [cudd-consumers.md](cudd-consumers.md)).
3. **Supported platforms:** only Linux x86-64 with GCC is verified. Decide whether macOS (declared in the README), Linux ARM64 (the committed `ltl2ba` targets it) and Clang are supported, and what CI must cover. Windows is excluded by #14 and the LP64 check.
4. **Runtime tools:** decide whether `ltl2ba` is built from the vendored `src/libs/ltl2ba`, shipped as a binary per platform, or required in `PATH`, and the same for SPIN, Java with `TVLParser.jar`, and `cpp`. Decide whether they are installed with the package.
5. **Package format and configuration:** which archives CPack should produce, in which build type, and whether they include the headers and the library or only the CLI.
6. **Reproducibility target:** decide whether path-independent binaries (for example `-ffile-prefix-map`, which CUDD would need too) and offline builds (a pinned or local GoogleTest) are goals.

## Answers to the acceptance criteria

1. **Installation contents and consumer workflow:** documented in [Installation contents](#installation-contents) and [Package-consumer workflow](#package-consumer-workflow). The working workflow today needs the consumer to define the CUDD targets from a DaedaluX build tree and to use C++20.
2. **Missing or incorrect exported dependencies:** CUDD (targets, archive, headers) and the C++20 requirement. GoogleTest is installed when it should not be.
3. **Portability assumptions:** listed with evidence under [Portability assumptions](#portability-assumptions).
4. **Reproducibility risks and tool versions:** under [Reproducibility](#reproducibility).
5. **Supported platforms and package guarantees:** Linux x86-64 with GCC is validated. The rest is recorded as open decisions above.
6. **Fixes deferred:** this report changes nothing. The follow-ups are listed below.

## Follow-up issues

These are proposals. They have not been opened yet, and they need the maintainer's review.

| Proposal | Severity | Subject |
|---|---|---|
| New issue | S2 | Make `find_package(daedalux)` work: install and export CUDD, or remove it from the public interface (decision 2) |
| New issue | S3 | Export the C++20 requirement (`target_compile_features(daedalux_lib PUBLIC cxx_std_20)`) and add the missing `<cstdint>` and `<stdexcept>` includes |
| New issue | S3 | Fix the case of `daedalux/visualizer.hpp` in `daedalux.hpp`, and compile the umbrella header in the build so that it stays valid |
| New issue | S3 | Keep GoogleTest out of the install and the archives (`INSTALL_GTEST OFF`) |
| New issue | S3 | Set the `CPACK_*` variables before `include(CPack)` (decision 5) |
| New issue | S3 | Check for Bison and Flex at configure time (`find_package(BISON)`, `find_package(FLEX)`) |
| New issue | S3 | Pass the project's compiler and build type to CUDD's `configure` |
| New issue | S3 | Allow offline configure: pin GoogleTest by commit hash, or use an installed GoogleTest |
| New issue | S3 | Fix or remove the Docker scripts (`build_image.sh`, `docker/Dockerfile`) |
| New issue | S4 | Remove the committed `CTestTestfile.cmake` and `DartConfiguration.tcl` |
| Comment on #22 | — | Add the evidence: the committed `src/bin/ltl2ba` is ARM64-only, and 14 of the 25 baseline failures come from the lookup |
| Comment on #10 | — | The CI matrix declares `gcc` and `clang`, but no step uses the compiler |

## Reproduction

The scripts used for this report are not committed. The key commands are:

```bash
git clone https://github.com/samilazreg-eng/DaedaluX.git src && git -C src checkout 46d8e82 && cd src
cmake -S . -B _b -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build _b -j10
cmake --install _b --prefix "$PWD/../prefix"
(cd _b && cpack)                                        # .sh, .tar.gz and .tar.Z, all with GoogleTest

# Each header on its own
cd ../prefix/include && for h in $(find . -path ./gtest -prune -o -path ./gmock -prune -o -type f -print); do
  echo "#include \"${h#./}\"" | g++ -std=c++20 -fsyntax-only -x c++ -I. - || echo "FAIL $h"; done

# ltl2ba at the path the tests expect (throwaway, inside the build directory)
file ../../src/src/bin/ltl2ba ../../src/src/libs/bin/ltl2ba
cd ../../src/_b && mkdir -p src/bin && ln -s "$PWD/../src/libs/bin/ltl2ba" src/bin/ltl2ba && ctest --timeout 120

# Offline configure
unshare -rn cmake -S . -B _off -G Ninja                  # fails: Could not resolve host
```

The minimal consumer:

```cmake
cmake_minimum_required(VERSION 3.19)
project(daedalux_consumer LANGUAGES CXX)
option(PROVIDE_CUDD "Define CUDD::obj/CUDD::cudd from a DaedaluX build tree" OFF)
option(USE_CXX20 "Compile the consumer as C++20" OFF)
if(PROVIDE_CUDD)
  add_library(CUDD::cudd STATIC IMPORTED)
  set_target_properties(CUDD::cudd PROPERTIES IMPORTED_LOCATION "${DX_BUILD}/ext/cudd/build/cudd/.libs/libcudd.a"
    INTERFACE_INCLUDE_DIRECTORIES "${DX_SRC}/src/libs/cudd/cudd")
  add_library(CUDD::obj INTERFACE IMPORTED)
  set_target_properties(CUDD::obj PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${DX_SRC}/src/libs/cudd/cplusplus")
endif()
if(USE_CXX20)
  set(CMAKE_CXX_STANDARD 20)
endif()
find_package(daedalux 1.0 REQUIRED)
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE daedalux::daedalux_lib)
```

```cpp
#include <daedalux/promela/parser/promela_loader.hpp>
#include <iostream>

int main(int argc, char **argv) {
  if (argc != 2) return 2;
  promela_loader loader(argv[1]);
  auto fsm = loader.getAutomata();
  std::cout << "loaded " << argv[1] << ": " << (fsm ? "automaton built" : "no automaton") << "\n";
  return fsm ? 0 : 1;
}
```

It is configured with `-DCMAKE_PREFIX_PATH=<prefix>` (plain), then with `-DPROVIDE_CUDD=ON -DDX_BUILD=<src>/_b -DDX_SRC=<src>` (with and without `-DUSE_CXX20=ON`), and run on `examples/test_files/basic/struct_simple.pml`. The reproducibility comparison clones the same commit into two directories of different lengths, builds `daedalux_cli` in Release in each, and compares the outputs with `cmp`.
