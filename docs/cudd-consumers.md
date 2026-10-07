# CUDD consumers and the duplicate `libcudd.a` linkage

Investigation report for [#63](https://github.com/samilazreg-eng/DaedaluX/issues/63). It maps who uses CUDD and explains why the same archive appears twice on every link line. The build is not changed. Fixes are left to separate issues.

The last section, [After the devendoring](#after-the-devendoring), records the same map once CUDD is an external dependency.

Findings are classified as in [build-characterization.md](build-characterization.md):

- **OBSERVED**: reproduced by a command recorded in this investigation.
- **INFERRED**: interpretation derived from observations.
- **UNKNOWN**: not established by the current evidence.

## Setup

- Commit: `1740647` (`main`; the CUDD rules are unchanged since `baseline/2026-09`). The `CMakeLists.txt` line numbers below refer to this commit.
- Environment: WSL Ubuntu 26.04, CMake 4.2.3, Ninja 1.13.2, GNU ld 2.46, GCC 15.2 and Clang 21.1.8
- Fresh `git clone`, configured out of source with Ninja in Debug, full default-target build: `184/184`, rc 0, with each compiler.
- The analysis uses the build's own records: `ninja -t deps` (headers each object actually included), `nm` (symbols each object leaves undefined, compared with the symbols `libcudd.a` defines), and `ninja -t commands` (link lines), re-run by hand with changes.

## Short answer

- CUDD is used by **13 of the 95 library objects**, in five of the seven modules, plus six test sources. It is used almost only through its C++ wrapper (`ADD`, `BDD`, `Cudd`). One source, `tvl.cpp`, also includes CUDD's **internal** header `cuddInt.h`.
- CUDD is part of the **public API**: 7 public headers include `cuddObj.hh`, and 30 of the 144 public headers reach it, including the umbrella `daedalux.hpp` and `core/automata/fsm.hpp`.
- `CUDD::obj` and `CUDD::cudd` point to the same file because the vendored CUDD used to be built **without** `--enable-obj`, which produces a separate `libobj.a` for the C++ wrapper. Since `3fd17b2` (#4, PR #34) CUDD is configured **with** `--enable-obj`, which puts the wrapper into `libcudd.a`. Both targets were kept and pointed to that archive.
- CMake removes duplicate link items by **target**, not by file, so two targets give two copies. They come from `daedalux_lib`'s `PUBLIC` link interface.
- The duplicate is **not needed** with GNU ld and GCC or Clang: all 35 executables link with a single copy and produce **byte-identical** outputs. The **position** of the archive matters, but the number of copies does not. macOS is untested.

## CUDD in the build

| Item | Location | What it does |
|---|---|---|
| `ExternalProject_Add(CUDD_project)` | `CMakeLists.txt` 82–100 | runs `configure --enable-silent-rules --enable-obj --enable-dddmp --disable-shared` in `<build>/ext/cudd/build`, then `make -j4`. The only declared output is `cudd/.libs/libcudd.a` |
| `CUDD::cudd` | 102–109 | imported static library → `libcudd.a`, usage requirement `src/libs/cudd/cudd` |
| `CUDD::obj` | 111–119 | imported static library → the **same** `libcudd.a`, usage requirement `src/libs/cudd/cplusplus` |
| `include_directories(...)` | 70–80 | directory-scope include path: `cplusplus`, the CUDD root, `cudd`, `st`, `mtr`, `epd`, `dddmp` and the generated `config.h` directory |
| `add_dependencies(<module> CUDD_project)` | 203–211 | makes `daedalux_lib` and all seven object libraries wait for the CUDD build |
| `target_link_libraries(daedalux_lib PUBLIC CUDD::obj CUDD::cudd)` | 212–215 | the only place where CUDD becomes a link dependency of the library |
| `target_link_libraries(daedalux_cli PRIVATE daedalux_lib CUDD::obj CUDD::cudd)` | 238–242 | the CLI lists CUDD again |
| `add_custom_target(bootstrap_cudd)` | 56–68 | runs `./configure` **in the source tree**. Nothing depends on it |

## Findings

### Direct consumers (Q1)

- **OBSERVED:** Only two source files include a CUDD header themselves: `src/feature/tvl.cpp` (`<cuddInt.h>`) and `tests/unit/core/automata/test_fsm.cpp` (`"cuddObj.hh"`). Every other use goes through the public headers below.
- **OBSERVED:** `libcudd.a` defines 1,343 global symbols. The objects that reference at least one of them are:

| Module | Objects | Include a CUDD header | Reference CUDD symbols | Direct consumers |
|---|---:|---:|---:|---|
| core | 24 | 8 | 3 | `automata/astToFsm`, `automata/fsm`, `automata/fsmEdge` |
| algorithm | 10 | 8 | 2 | `reachabilityRelation`, `cli/modelchecking_subcommand` (only `ABDD::~ABDD`) |
| feature | 5 | 5 | 5 | `tvl`, `expToADD`, `ADDutils`, `featured`, `featuredTransition` |
| formulas | 2 | 2 | 0 | — |
| promela | 47 | 6 | 2 | `parser/promela_loader` (only the `ADD` copy constructor), `semantic/.../initState` |
| mutants | 4 | 3 | 0 | — |
| visualizer | 3 | 1 | 1 | `stateToGraphViz` |
| **library** | **95** | **33** | **13** | |
| CLI (`main_cli.cpp`) | 1 | 1 | 0 | — |
| tests | 34 | 20 | 6 | `test_ADDutils`, `test_fsm`, `test_fsmEdge`, `test_fsmNode`, `test_stateToGraphViz`, `test_transition` |

- **OBSERVED:** The referenced symbols are the C++ wrapper classes (`ADD`, `BDD`, `ABDD`, `DD`, `Cudd`) and its `defaultError` handler. Only `tvl.cpp` calls the C API directly (`Cudd_PrintInfo`, `Cudd_PrintMinterm`). `tvl.cpp` is also the only object that creates a `Cudd` manager (`TVL::mgr`), apart from `test_ADDutils`.
- **OBSERVED:** `tvl.cpp` needs `cuddInt.h` because it writes the manager's internal output stream: `formula.manager()->out = fopen(...)` then `fclose(formula.manager()->out)`, in `TVL::printBool(const ADD&)`, `TVL::toString(const ADD&)` and `TVL::printMinterms`. Compiled without that include, it fails with `invalid use of incomplete type 'DdManager'`. `cudd.h` declares public accessors for this field (`Cudd_ReadStdout`, `Cudd_SetStdout`).
- **OBSERVED:** `tvl.cpp` is the only object that includes the generated `<build>/ext/cudd/build/config.h`, through `cuddInt.h`. The other 53 CUDD-including objects use only `cuddObj.hh` and `cudd.h`, which do not need it.
- **INFERRED:** Only `daedalux_feature` needs CUDD's configure step to finish before it compiles. The six other object-library dependencies on `CUDD_project` (lines 205–211) serialize compilation behind the CUDD build without being needed for it. The link still needs `libcudd.a`, and it gets it through the dependencies of the imported targets.

### Transitive consumers (Q2)

- **OBSERVED:** 20 library objects, `main_cli.cpp` and 14 test sources include `cuddObj.hh` without referencing any CUDD symbol. They get it through the public headers below, so they recompile when CUDD's headers change.
- **OBSERVED:** At the target level, the 34 test executables get CUDD only through `daedalux_lib`, which links CUDD `PUBLIC`. The CLI gets it the same way and also lists `CUDD::obj` and `CUDD::cudd` itself (`CMakeLists.txt` 238–242). Those direct items add nothing to its link line (next section).
- **OBSERVED:** Linking each executable **without** `libcudd.a` (GCC build) shows which ones really need it. 25 fail (the CLI with 202 `undefined reference` lines), because the library objects they pull in reference CUDD. 10 link without it: `test_bisimulation`, `test_bitSymNode`, `test_intSymNode`, `test_ltl_creator`, `test_specification_writer`, `test_spinRunner`, `test_symTable`, `test_symbol`, `test_temporalSymNode` and `test_varSymNode`.
- **OBSERVED:** Because `include_directories` is directory-scoped, every target in the project receives the CUDD include paths, including the GoogleTest targets fetched by `FetchContent` (`gtest-all.cc` is compiled with all eight `-I…cudd…` flags).
- **INFERRED:** The build does not say who needs CUDD. The include path is global, the configure-step dependency is applied to every module, and the link dependency is attached to the aggregate library. The actual consumers can only be recovered from the compiled objects, as done here.

### Exposure through public headers

- **OBSERVED:** Seven public headers include `cuddObj.hh` directly: `core/automata/fsm.hpp`, `core/automata/astToFsm.hpp`, `core/automata/fsmEdge.hpp`, `feature/tvl.hpp`, `feature/ADDutils.hpp`, `feature/semantic/variable/state/featured.hpp` and `feature/semantic/variable/transition/featuredTransition.hpp`.
- **OBSERVED:** 30 of the 144 headers under `include/` reach `cuddObj.hh` (`g++ -MM`). They include the umbrella headers `daedalux.hpp`, `core.hpp`, `core/automata.hpp`, `algorithm.hpp`, `feature.hpp` and `visualizer.hpp`, plus `promela/parser/promela_loader.hpp` and `mutants/cli/mutant_subcommand.hpp`.
- **OBSERVED:** CUDD types appear in the signatures and data members of these headers, not only in implementation details. For example `fsm(const symTable*, const ADD& fd)` and `ADD fd` in `fsm.hpp`, `static Cudd* mgr` and `std::vector<BDD> vars` in `tvl.hpp`, `ADD features` in `fsmEdge.hpp`, and `ADD` members in `reachabilityRelation.hpp`, `stateToGraphViz.hpp` and `expToADD.hpp`. The last three get `ADD` through `tvl.hpp` or `core/automata.hpp` rather than including `cuddObj.hh` themselves.
- **INFERRED:** CUDD cannot be made a private dependency of `daedalux_lib` without changing these headers. Even `core`, the base module, exposes CUDD through `fsm.hpp`.
- **OBSERVED:** `cmake --install` succeeds but installs neither CUDD's headers nor `libcudd.a`. The exported `daedaluxTargets.cmake` sets `INTERFACE_LINK_LIBRARIES "CUDD::obj;CUDD::cudd"`, and `cmake/daedaluxConfig.cmake.in` defines no CUDD target (its `find_dependency` line is commented out). A minimal downstream project that calls `find_package(daedalux)` fails at configure time: `The link interface of target "daedalux::daedalux_lib" contains: CUDD::obj but the target was not found.`
- **INFERRED:** The installed package is not usable, and CUDD's exposure in the public headers is one of the reasons.

### Why two imported targets reference one archive (Q3)

- **OBSERVED:** The vendored CUDD is 3.0.0 (`configure.ac`). Its `cplusplus/Included.am` compiles the C++ wrapper (`cuddObj.cc`) **into `libcudd.la` when `--enable-obj` is given**, and otherwise into a separate, uninstalled convenience library `cplusplus/libobj.la`.
- **OBSERVED:** The history of the CUDD link rules:

| Commit | CUDD configured with | Linked as |
|---|---|---|
| `fb2aa4d` (2022-10-15, first commit) | plain `./configure` (committed `config.log`, `OBJ_FALSE=''`) | `target_link_libraries(deadalux libobj.a libcudd.a)` (the target's spelling at the time) from the committed `cplusplus/.libs` and `cudd/.libs`. The committed `libobj.a` contains only `cuddObj.o`, and the committed `libcudd.a` (76 members) contains no wrapper |
| `ec82629` (2025-06-27, alpha refactor) | `configure` without flags, then `make <src> check` | introduces `CUDD::obj` → `cplusplus/.libs/libobj.a` and `CUDD::cudd` → `cudd/.libs/libcudd.a`, both linked by `daedalux_lib` and the CLI |
| `f3e2875` (2025-06-27) | no configure step (`CONFIGURE_COMMAND ""`). The flags `--enable-obj …` are written only into the new, unused `bootstrap_cudd` target | unchanged |
| `3fd17b2` (2026-09-24, #4 / PR #34) | the `bootstrap_cudd` flags, including `--enable-obj`, out of source | `libobj.a` is no longer produced. Both targets now point to `libcudd.a` (90 members, including `cudd_libcudd_la-cuddObj.o`) |

- **INFERRED:** The two targets mirror the original **two-archive** layout, wrapper plus core. The pair lost its meaning when the build adopted `--enable-obj`. Pointing both targets at `libcudd.a` in `3fd17b2` fixed the clean-checkout build without changing any other part of the CMake file, and it left the duplicate behind.
- **OBSERVED:** Today the only difference between the targets is their include directory: `cplusplus` for `CUDD::obj`, `cudd` for `CUDD::cudd`. Both directories are also on the global include path.
- **UNKNOWN:** Why `ec82629` chose to configure without `--enable-obj` while `f3e2875` wrote `--enable-obj` into `bootstrap_cudd`. Neither commit message says so.

### Why the archive appears twice (Q4)

- **OBSERVED:** All 35 executables are linked with `libcudd.a` exactly twice, right after `libdaedalux_lib.a` (CLI) or after the GoogleTest archives (tests). For example, the CLI's line is `… libdaedalux_lib.a …/libcudd.a …/libcudd.a`.
- **OBSERVED:** The copy count in configure-only variants of `CMakeLists.txt`:

| Variant | CLI | `test_fsm` |
|---|---:|---:|
| as committed | 2 | 2 |
| `-DCMAKE_POLICY_DEFAULT_CMP0156=NEW` | 2 | 2 |
| CLI links only `daedalux_lib` (its `CUDD::obj CUDD::cudd` removed) | 2 | 2 |
| `CUDD::obj` removed everywhere | 1 | 1 |
| both changes | 1 | 1 |

- **INFERRED:** CMake removes duplicate link items by target identity. `CUDD::obj` and `CUDD::cudd` are two targets, so both are emitted even though they name the same file. The pair comes from `daedalux_lib`'s `PUBLIC` interface. The CLI's own `CUDD::obj CUDD::cudd` are merged with it and add nothing.
- **OBSERVED:** `cmake_minimum_required(VERSION 3.19)` leaves CMP0156 (CMake 3.29) unset. According to its documentation, even `NEW` removes static library duplicates only for linkers that need no repetition (Apple, Windows, LLD). On Linux with GNU ld, the variant above keeps both copies.

### Is the duplicate required? (Q5)

- **OBSERVED:** Each of the 35 link commands was re-run by hand three ways, with GCC and with Clang:
  - **Two copies** (as generated): all link.
  - **One copy** (the second removed): all link, and each output is **byte-identical** to the two-copy output (`cmp`), with both compilers.
  - **No copy:** the 25 executables listed above fail.
  - **One copy placed before `libdaedalux_lib.a`:** the CLI fails with the same 202 (GCC) or 201 (Clang) `undefined reference` lines as with no copy.
- **INFERRED:** GNU ld searches an archive repeatedly until it resolves no new symbol, so references from the wrapper (`cuddObj.o`) to the C core in the same archive are resolved within one copy. A second copy of the same archive at the same position is a no-op. What the link does need is `libcudd.a` **after** `libdaedalux_lib.a`, because nothing in CUDD references DaedaluX.
- **OBSERVED:** The documented platforms are Linux and macOS with GCC ≥ 10 or Clang ≥ 11 (README). CI builds `ubuntu-latest` with `gcc` and `clang`. Both Linux configurations are covered above. Both compilers use the system GNU ld here. LLD, gold and mold are not installed and were not tested.
- **INFERRED:** Linkers that record every archive symbol (LLD, Apple's linker) cannot need the repetition either, according to the CMP0156 documentation quoted above.
- **UNKNOWN:** The behaviour on macOS, which was not tested. Windows is not a documented platform (see also #14).

## Answers to the issue's questions

1. **Direct users:** in the library, 13 objects in `core`, `algorithm`, `feature`, `promela` and `visualizer`; `formulas` and `mutants` use none. Two sources include CUDD themselves (`tvl.cpp`, `test_fsm.cpp`). Seven public headers include `cuddObj.hh`. In the tests, six test sources reference CUDD symbols. The CLI executable uses CUDD only through the library.
2. **Transitive only:** 20 library objects, `main_cli.cpp` and 14 test sources see CUDD's headers without using its symbols. At the target level, every test gets CUDD only through `daedalux_lib`'s `PUBLIC` link. The CLI also lists the two CUDD targets directly, without effect on its link line. 10 test executables need no CUDD code at all.
3. **Two targets, one archive:** a leftover of CUDD's two-archive layout (`libobj.a` + `libcudd.a`, built without `--enable-obj`). Since `3fd17b2` configures with `--enable-obj`, the wrapper is inside `libcudd.a`, and both targets were pointed there.
4. **Twice on the link line:** CMake keeps one occurrence per target, and `daedalux_lib` links two targets that name the same file.
5. **Required?** No, with GNU ld on Linux, with GCC and with Clang: one copy after `libdaedalux_lib.a` gives byte-identical executables. macOS is **UNKNOWN**.

## Proposed follow-up issues

These are proposals for separate implementation issues. Nothing here has been changed.

1. **Replace the two imported targets with one** (for example `CUDD::cudd` carrying both the `cudd` and the `cplusplus` include directories), and drop the CLI's redundant CUDD items. Expected effect: one copy per link line, byte-identical outputs.
2. **Scope CUDD's include paths to targets**: move them from the directory-scope `include_directories` to the imported target's usage requirements, so that GoogleTest and CUDD-free targets stop receiving them. Keep the internal directories (`st`, `mtr`, `epd`, `dddmp`, the CUDD root, generated `config.h`) for the one source that needs them.
3. **Stop using CUDD internals in `tvl.cpp`**: replace the writes to `DdManager::out` with `Cudd_ReadStdout`/`Cudd_SetStdout`, and remove `#include <cuddInt.h>`. The object libraries would then no longer need CUDD's generated `config.h`, and the seven compile-order dependencies on `CUDD_project` could be reduced to the link.
4. **Decide CUDD's place in the public API and fix the package export**: either install and export CUDD (headers, archive, a `CUDD::` target found by `daedaluxConfig.cmake`), or stop exposing it in the public headers. Today `find_package(daedalux)` fails.
5. **Remove the unused `bootstrap_cudd` target**, which would configure CUDD inside the source tree if anyone ran it.
6. **Verify the link on macOS** (Apple's linker, and CMP0156 `NEW` there).

## Reproduction

The scripts used for this report are not committed. The key commands are:

```bash
git clone https://github.com/samilazreg-eng/DaedaluX.git src && git -C src checkout 1740647
cmake -S src -B src/build -G Ninja -DCMAKE_BUILD_TYPE=Debug && ninja -C src/build
cd src/build
ninja -t deps | less                                  # headers each object included (look for src/libs/cudd/)
nm -g --defined-only ext/cudd/build/cudd/.libs/libcudd.a   # symbols CUDD defines
nm -u CMakeFiles/daedalux_core.dir/src/core/automata/fsm.cpp.o | c++filt   # symbols an object needs
ninja -t commands daedalux_cli | tail -1              # the link line: libcudd.a twice
```

For the multiplicity test, the last command is re-run with the second `libcudd.a` removed and a different `-o`, and the two outputs are compared with `cmp`.

## After the devendoring

Verification for [#126](https://github.com/samilazreg-eng/DaedaluX/issues/126). The sections above describe the build when DaedaluX compiled a vendored CUDD 3.0.0. This one records the same map once DaedaluX consumes an external CUDD 4.0 ([#127](https://github.com/samilazreg-eng/DaedaluX/issues/127)).

- Measured on the branch `build/remove-vendored-cudd` (#132), which contains #129, #130 and #131. Its `README.md` and a comment of `scripts/install-cudd.sh` were reworded after the measurement.
- CUDD 4.0 at `d1857bf`, installed by `scripts/install-cudd.sh`, one prefix per compiler.
- Same environment as above: GCC 15.2 and Clang 21.1.8, CMake 4.2.3, Ninja 1.13.2, GNU ld 2.46. Debug, fresh build directories.

| | Before | After |
|---|---|---|
| Who provides CUDD | the DaedaluX build: `ExternalProject`, vendored 3.0.0 | the environment: `find_package(cudd 4.0.0 CONFIG REQUIRED)` |
| CUDD targets | `CUDD::obj` and `CUDD::cudd`, written by hand, same archive | `cudd::cudd`, from CUDD's package |
| Include paths | eight, directory-wide, on every target | one, from `cudd::cudd` |
| Build order | all seven modules wait for the CUDD build | no CUDD build |
| Link line | `libcudd.a` twice | `libcudd.a` once |
| CUDD files in the repository | 266 files and one archive | none |

### Dependency scope

- **OBSERVED:** In the target graph (`cmake --graphviz`), eight targets have a direct edge to `cudd::cudd`, all `PUBLIC`: the seven modules and `daedalux_lib`. The CLI and the 34 test executables have none. They get CUDD through `daedalux_lib`. No GoogleTest target has one.
- **OBSERVED:** 130 of the 134 compile commands have the CUDD include path: the 95 library objects, the CLI's source and the 34 test sources. The four without it are GoogleTest's and GoogleMock's sources. The result is the same with GCC and with Clang.
- **INFERRED:** The path reaches more objects than the ones that use CUDD: at `1740647`, 33 of the 95 library objects and 20 of the 34 test sources included a CUDD header. This is not a leak of the build. CUDD types are in DaedaluX's public headers, so the requirement is `PUBLIC`. Narrowing it means removing CUDD from those headers, which is an API change.
- **OBSERVED:** CUDD adds no compile definition. The only `-D` on any compile command is GoogleTest's own `GTEST_HAS_PTHREAD=1`.
- **OBSERVED:** No compile command names `src/libs/cudd` or `ext/cudd`. The build tree has no `ext/` directory and no CUDD object.
- **OBSERVED:** Each of the 35 executables has the CUDD archive once on its link line.
- **OBSERVED:** In a fresh build directory, `ninja daedalux_formulas` and `ninja daedalux_visualizer` each succeed alone. Before, every module waited for `CUDD_project`.
- **OBSERVED:** Configure, build and test from fresh build directories: 121 of 146 with GCC and with Clang. No test differs from the baselines measured with the vendored CUDD (`docs/cudd-4-compatibility.md`).
- **OBSERVED:** `git ls-files src/libs/cudd src/libs/cudd-release.zip` returns nothing. Outside `docs/`, CUDD is named in the three CMake files that find and link it, in `ci.yml`, `README.md` and `scripts/install-cudd.sh`, in the sources and headers that use it, and in `docker/Dockerfile`.

### Install and package

- **OBSERVED:** `cmake --install` installs no CUDD file. The export says `INTERFACE_LINK_LIBRARIES "cudd::cudd"`, and `daedaluxConfig.cmake` declares no dependency.
- **OBSERVED:** A consumer that calls only `find_package(daedalux)` fails at configure time: `The link interface of target "daedalux::daedalux_lib" contains: cudd::cudd but the target was not found.`
- **OBSERVED:** A consumer that calls `find_package(cudd 4.0.0 CONFIG REQUIRED)` first configures, builds and links, with `libdaedalux_lib.a` then `libcudd.a` on its link line. The consumer is a one-line `main` that links `daedalux::daedalux_lib` and includes no DaedaluX header.
- **INFERRED:** #77 is reduced to one missing `find_dependency(cudd 4.0.0)` in `cmake/daedaluxConfig.cmake.in`. A consumer of the installed DaedaluX needs CUDD 4.0.0 installed too.

### Leaks

None was found in the build. Two known points remain, and each already has an issue:

- the package export does not declare CUDD (#77);
- `docker/Dockerfile` still downloads and builds CUDD 3.0.0 from `ivmai/cudd` (#84).

### Commands

```bash
scripts/install-cudd.sh "$PWD/cudd"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$PWD/cudd" --graphviz=build/deps.dot
cmake --build build --parallel
ctest --test-dir build --timeout 120 --output-junit junit.xml
grep -c "$PWD/cudd/include" build/compile_commands.json       # commands with the CUDD path
grep -c -E 'src/libs/cudd|ext/cudd' build/compile_commands.json
ninja -C build -t commands daedalux_cli | tail -1             # link line
cmake --install build --prefix "$PWD/install"
grep -n cudd install/lib/cmake/daedalux/daedaluxTargets.cmake
```
