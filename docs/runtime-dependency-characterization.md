# Runtime dependencies: characterization for Phase 5

Characterization for Phase 5 (runtime contract, D7 and D10) of [build-modernization-roadmap.md](build-modernization-roadmap.md). It records what DaedaluX runs, reads and writes outside its own process at run time, how it finds each tool, what happens when a tool is missing or fails, and what is installed. It changes no production code and no CMake file, and it selects no solution.

## 1. Scope and methodology

- **Baseline:** `main` at `e341c7e93e7fd013f0c43034fc97db6d620f6a2e` (2026-10-09, merge of #146). CI is green on it. Line numbers refer to this commit.
- **Environment:** WSL Ubuntu 26.04.1 (x86-64), GCC and `cpp` 15.2.0, CMake 4.2.3, Ninja 1.13.2, OpenJDK 25.0.4.1, Python 3.14.4, CUDD 4.0.0 from `scripts/install-cudd.sh`. Not installed: `spin`, `ltl2ba`, `owl`, `dot`, and the Python packages `openai` and `Scarlet`.
- **Tree:** a `git archive` export of the baseline, a fresh build directory, Debug, Ninja. A Release build of the CLI was added for section 7.
- **Tools added for the experiments**, outside the repository and without installation:
  - SPIN 6.5.2, the Ubuntu package `spin 6.5.2+dfsg-2build1`, extracted into a private directory and reached through `PATH`. It is the version that `docker/Dockerfile` and `.devcontainer/Dockerfile` build.
  - `ltl2ba`, built from a copy of `src/libs/ltl2ba`.
- **Instruments**, none committed:
  1. An `LD_PRELOAD` logger: every `system()` command and its return value, every `popen()` and `pclose()`, every `execve()`, every file opened for writing, and every file opened through a relative path. Child processes inherit it, so it also shows what `sh` and `spin` run. The named test outcomes are the same with and without it.
  2. A probe linked against `libdaedalux_lib.a`. It calls one public entry point inside `try`/`catch` and reports whether the call returned, threw, or ended the process.
  3. `nm -u` on the 95 objects of `daedalux_lib` and on the test objects, to list the compiled code that references each entry point.
  4. A `PATH` reduced to chosen directories, to remove or add one tool.
- **Classification.** A finding marked *run* is OBSERVED in an executed experiment, named by its ID (section 8.3). A finding marked *code* is OBSERVED by reading the source at the cited line. **INFERRED** and **UNKNOWN** are written out.
- **Earlier reports.** [test-infrastructure.md](test-infrastructure.md) and [install-package-portability.md](install-package-portability.md) describe `46d8e82`. Three things differ today and matter here: each test case runs in a private workspace `<build>/tests/runtime/<Suite>.<Case>.XXXXXX` (#143), CUDD is an external package (#127), and the default build compiles the tracked parser sources without Flex or Bison.

## 2. Executive summary

1. **Five external commands are written in the code:** `cpp`, `ltl2ba`, `spin`, `java -jar TVLParser.jar` and `which owl`. All go through `system()` or `popen()`, so `/bin/sh` is a sixth. `spin -run` itself runs `gcc` twice and then the `pan` it compiled.
2. **Only `cpp` is reachable from the CLI.** `gen-mutants`, `gen-traces` and `gen-single-traces` run `cpp` and nothing else. `check` stops before any tool (#21). `ltl2ba` and `spin` are reached only through the library API, which today means the tests. The Java call cannot be reached. The `owl` check has no caller.
3. **Three discovery mechanisms coexist.** `PATH` for `cpp`, `spin`, `gcc` and `java`. The working directory for `ltl2ba` (`<cwd>/../src/bin/ltl2ba`) and the TVL jar (`./libs/tvl/TVLParser.jar`). CMake knows none of them: no CMake file and no configure log names a runtime tool.
4. **Four failure behaviours coexist.** A failing `cpp` ends the process with `exit(1)` inside the library. A failing `ltl2ba` throws `std::runtime_error`, with one message for three causes. A missing `spin` returns `false`, the value that also means "property violated". A `spin` that starts and then fails (no `gcc`, a model that does not parse, a read-only working directory) returns `true`, the value that means "property holds".
5. **Nothing the runtime needs is installed.** The install is the CLI, the static library and the headers (150 files). The repository tracks two `ltl2ba` binaries, an aarch64 one where the lookup can find it and an x86-64 one that nothing references, plus `ltl2ba` sources that the build ignores and the TVL jar with three jars it needs beside it.
6. **Test coverage is uneven.** 71 of the 151 enabled cases run `cpp`. 14 reach the `ltl2ba` lookup and fail there. 4 reach `spin`. None reaches Java or `owl`, and no test runs the CLI executable. With `ltl2ba` and SPIN supplied, 142 cases pass instead of 126, serially and under `ctest -j8`.
7. **Python is used around DaedaluX, not by it.** DaedaluX starts no Python process. 17 tracked scripts do the reverse: some start the CLI under the name `daedalux`, which the build does not produce, read what it prints, or read the trace files it writes. Two need `openai` or `Scarlet`. None is installed, tested or run by CI.
8. **The Phase 5 exit criterion is not met.** The three working CLI subcommands and the tests do run from any working directory. LTL translation and TVL loading do not, and `#include` in a model resolves against the working directory.

## 3. Dependency inventory

### 3.1 Runtime dependencies

| Dependency | Command as executed | Source | Reached today from | Transitive |
|---|---|---|---|---|
| `/bin/sh` | every `system()` and `popen()` below | — | wherever a tool is | — |
| `cpp` | `cpp < '<model>' > '<scratch>/__workingfile.tmp.cpp'` | `promela_loader.cpp:94-95` | CLI (3 subcommands), library, 71 test cases | GCC's `cc1` (**INFERRED** from `cpp` being the GCC driver) |
| `ltl2ba` | `<cwd>/../src/bin/ltl2ba -f "!(<formula>)" > <cwd>/__formula.tmp` | `ltl.cpp:27-28` | library API only; 14 test cases | libc only (*run*: `ldd`) |
| `spin` | `spin -V`, then `spin -run <model>_temp` | `spinRunner.cpp:40`, `67-68` | library API only; 4 test cases | `gcc`, `./pan` |
| `gcc`, by that name | run by SPIN: `gcc -std=gnu99 -E -x c "<model>_temp" > "pan.pre"`, then `gcc -std=gnu99 -O -DNOFAIR -o pan pan.c` | SPIN 6.5.2 (*run* SPIN-1) | with `spin -run` | — |
| `java` and `TVLParser.jar` | `java -jar ./libs/tvl/TVLParser.jar -dimacs __mapping.tmp __clauses.tmp __workingfile.tvl` | `tvl.cpp:73` | nowhere (section 5.4) | three jars named by the jar's manifest |
| `which` and `owl` | `which owl > /dev/null 2>&1`. The `owl` call itself is commented out | `formulaSimplifier.hpp:32`, `22` | no caller | — |

- *run* (STATIC): four objects of `daedalux_lib` reference `system` or `popen`: `ltl.cpp.o`, `tvl.cpp.o`, `spinRunner.cpp.o` and `promela_loader.cpp.o`. No object references `fork`, an `exec` function or `posix_spawn`. The fifth call site is in a header, `formulaSimplifier.hpp`, and no object contains it.
- *run* (SPIN-1, SPIN-5b): the roadmap marks the runtime C compiler as INFERRED. It is now observed: SPIN preprocesses with `gcc`, compiles with `gcc`, and fails when only `cc` is on `PATH`.
- **Not runtime dependencies.** Graphviz: DaedaluX writes `.dot` text and no log shows `dot` being run. Threads: the only `std::thread` use is commented out (`mutantAnalyzer.cpp:185-206`). Network: none. `dlopen`: none.
- **Environment.** *code*: no DaedaluX source calls `getenv`, and no CLI option uses CLI11's `envname`. **INFERRED:** the `getenv` reference in `main_cli.cpp.o` comes from the CLI11 header. `PATH` acts through `sh`, and `TMPDIR` through `std::filesystem::temp_directory_path()` (`promela_loader.cpp:51`, `fsmExplorer.cpp:168`).
- **Optional input from the working directory.** `mdp.sched` is opened by relative name (`explore.cpp:125`). Only `check` reaches it.

### 3.2 Dependencies of other classes

Listed so that they are not counted as runtime dependencies.

| Dependency | Class | Evidence |
|---|---|---|
| C++20 compiler, CMake ≥ 3.21, Ninja or Make | build | `CMakeLists.txt:1-22` |
| Bison 3.8.2, Flex 2.6.4 | code generation, on request | `DAEDALUX_REGENERATE_PROMELA_PARSER` is `OFF` by default (`src/promela/parser/CMakeLists.txt:5-20`). *run*: the default `build.ninja` names neither tool. The README and CI still list both as prerequisites |
| CUDD 4.0.0 | compile/link, `PUBLIC` | `cmake/dependencies/CUDD.cmake:3`. Linked statically: *run*, `ldd daedalux_cli` lists only `libstdc++`, `libm`, `libgcc_s` and `libc` |
| `libstdc++`, `libm`, `libgcc_s`, `libc` | runtime, shared libraries | same `ldd` |
| CLI11 | compile, vendored header | `include/daedalux/CLI11.hpp` |
| GoogleTest 1.14.0, `tests/data` | test | `tests/CMakeLists.txt` |
| CPack (TGZ, ZIP) | package | `cmake/Packaging.cmake` |
| Python 3, `openai`, `Scarlet-ltl` | tooling around the product | section 3.3 |

### 3.3 Python tooling around DaedaluX

DaedaluX never starts Python. *run*: no log of the CLI, of the probe or of the test suite shows a Python process. *code*: no C++ source, no CMake file and no CI step names Python, and no `.py` file is installed. The dependency runs the other way: the repository tracks 17 Python files, and some of them start DaedaluX or read what it writes. None has a shebang, and no requirements file lists their packages.

| Files | Purpose | Commands they run | Needs beyond the standard library | Link with DaedaluX | State |
|---|---|---|---|---|---|
| `scripts/tools/GPTExperiment/*.py` (9) | specification mining with a language model | `./daedalux gen-mutants`, `./daedalux gen-single-traces`, `spin -a`, `gcc`, `./pan` | `openai`, an API key, network access | start the CLI from the working directory, and take the mutant and trace file names from its standard output | *run* PY-1: fails, because the build produces `daedalux_cli` and not `daedalux`. *run* PY-2: with a link named `daedalux` in the working directory, both calls work with the current CLI. *code*: the model paths in `query_chatgpt.py` are `../test_files/...`; the test data is now in `tests/data` |
| `scripts/tools/mutation_testing.py` | mutation testing of a model against its properties | `../daedalux -f ... -n ... -p ... mutants`, `spin -a`, `gcc`, `./pan`, `rm` | none | starts the CLI with an older command line | *run* PY-3: the current CLI rejects that command line (status 109) |
| `scripts/tools/ltl_learner.py` | learns an LTL formula from traces | none | `Scarlet` (the package `Scarlet-ltl`) | reads `trace_report_mutex_scarlet.trace`, in the format that `MutantAnalyzer::generateScarletFile` and `generatePredicatesForScarletFile` write | *run* PY-6: the tests write nine `.trace` files, none of that name. Not run: `Scarlet` is not installed here |
| `scripts/tools/run_tests.py` | an earlier test runner | `./deadalux` | none | none | *run* PY-4: neither `./deadalux` nor `./test` exists |
| `examples/models/{adapro,elevator,windows}/mutants/script.py` (3) | kill the mutants of one example with SPIN | `spin -a`, `gcc`, `./pan`, `cp`, `rm` | none | read mutant files | not run |
| `examples/models/csv/csvtodtrace.py` | CSV to Daikon `.dtrace` and `.decls` | none | none | reads CSV traces. Daikon, a Java tool, is the next step and is not in the repository | not run; #28 to #32 |
| `tests/data/warmingUp/gen_warmingUp.py` | generates test models | none | none | none | no test and no CMake rule runs it |

- **What this tooling relies on in DaedaluX** (*code*, and *run* PY-2): the name and the place of the executable (`./daedalux`, `../daedalux`), the names of the subcommands and of their options, the standard output of `gen-mutants` and `gen-single-traces` (one file name per line), and the two file formats, CSV and Scarlet traces.
- **SPIN again, by another route.** These scripts run `spin -a`, `gcc` and `./pan` themselves. They do not go through `spinRunner::check`, which runs `spin -run`.
- **Class.** A machine that runs `daedalux_cli` needs no Python. A user of the mutation-testing and specification-mining workflows does, with `spin`, `gcc`, and for two scripts `openai` or `Scarlet-ltl`.
- *run*: the 17 files parse under Python 3.14. **UNKNOWN:** the Python and package versions they need, and whether the workflows run end to end.

## 4. Component → capability → dependency mapping

The seven `OBJECT` libraries are merged into one static archive, `daedalux_lib`. Only `daedalux_lib` and `daedalux_cli` have an install rule. A component is therefore not something a user can install or leave out, and a runtime dependency follows a call path, not a component. `daedalux_cli` has one source, `main_cli.cpp`: the subcommands are compiled into `daedalux_algorithm` and `daedalux_mutants`.

### 4.1 Owners of the tool calls

| Component (target) | Owning code | Tool | Direct callers in compiled objects (*run*: `nm -u`) |
|---|---|---|---|
| PROMELA / PARSER (`daedalux_promela`, sources listed by `promela_parser`) | `promela_loader::promela_loader`, `promela_loader.cpp:71-125` | `cpp` | `daedalux_algorithm`: `cli/modelchecking_subcommand`, `ltlModelChecker`. `daedalux_mutants`: `cli/mutant_subcommand`, `modelAnalyzer`, `mutantAnalyzer`. Header: `traceGenerator.hpp:18,21` |
| CORE (`daedalux_core`) | `LTLClaimsProcessor::transformLTLStringToNeverClaim`, `ltl.cpp:9-46` | `ltl2ba` | In `ltl.cpp`: `appendClaimToFile`, `appendClaim`, `renewClaimOfFile`. `daedalux_algorithm`: `fsmExplorer` (`checkFormula`, lines 192 and 194). Header: `formula::neverClaim`, `formula.hpp:63-67` |
| MUTANTS (`daedalux_mutants`) | `spinRunner::check`, `spinRunner.cpp:32-95` | `spin`, then `gcc` and `pan` | `mutantAnalyzer` (`killMutantsSpin`, line 128), which `enhanceSpecification` calls (line 83) |
| FEATURE / TVL (`daedalux_feature`) | `TVL::loadFeatureModel`, `tvl.cpp:62-92` | `java`, the jar | `daedalux_algorithm`: `cli/modelchecking_subcommand` (lines 77 and 87) |
| FORMULAS (header only) | `FormulaSimplifier::simplify`, `formulaSimplifier.hpp:11-38` | `which` | none: no product object, no test object, no other source names the class |
| ALGORITHM, VISUALIZER, CLI (`main_cli.cpp`) | — | none of their own | they reach the tools through the owners above |

### 4.2 Capabilities and what they need

"Mandatory" answers two different questions: whether DaedaluX needs the capability, and whether the capability needs the tool.

| Capability | Entry points | `cpp` | `ltl2ba` | `spin` + `gcc` | `java` + jar | Is the capability needed today? | Does it need the tool? |
|---|---|---|---|---|---|---|---|
| Load a Promela model | `promela_loader` | yes | — | — | — | Yes: every working CLI subcommand and every analysis starts with a load | Yes. `cpp` runs on every load, with no fallback (*code*) |
| Mutant generation | `MutantAnalyzer::createMutants`, CLI `gen-mutants` | yes (one load) | — | — | — | Yes, CLI feature | only through the load |
| Trace generation | `TraceGenerator`, CLI `gen-traces`, `gen-single-traces` | yes | — | — | — | Yes, CLI feature | only through the load |
| Trace export for Scarlet | `MutantAnalyzer::generateScarletFile`, `generatePredicatesForScarletFile` | yes (it loads the model and its mutants) | — | — | — | Library and tests only. No CLI command writes this format | only through the loads. The file is meant for Scarlet, a Python package that DaedaluX does not run (section 3.3) |
| LTL model checking by DaedaluX | `ltlModelChecker::check` | yes for the file overload | — | — | — | Library and tests only. The never claim must already be in the model | only through the load |
| LTL formula to never claim | `formula::neverClaim`, `appendClaimToFile`, `appendClaim`, `renewClaimOfFile` | — | yes | — | — | No production caller: `checkFormula` has none, the only `appendClaim` call is commented out (`main_cli.cpp:63`), and tests call the other two. The CLI never uses `--ltl` beyond checking that it is set (`modelchecking_subcommand.cpp:158-166`) | Yes |
| Formula check on a model and its mutant | `fsmExplorer::checkFormula` | yes | yes | — | — | No caller in `src/`, `include/` or `tests/` | Yes, both |
| SPIN-based verification | `spinRunner::check`, `killMutantsSpin`, `enhanceSpecification` | only `enhanceSpecification`, which creates mutants first | — | yes | — | Library and tests only. No CLI path | Yes, and `gcc` with it |
| Feature model from TVL | `TVL::loadFeatureModel` | — | — | — | by design | Only `check` asks for it, and `check` cannot run (#21) | By design yes. Today the parser is never run (section 5.4) |
| Feature model from DIMACS | `TVL::loadFeatureModelDimacs` | — | — | — | — | Called only by `loadFeatureModel` | no tool |
| Formula simplification | `FormulaSimplifier::simplify` | — | — | — | — | No caller. Returns its argument on both branches (*run* OWL-1) | `owl` is never run |
| Removing a claim from a model file | `removeClaimFromFile` | — | — | — | — | Used by `spinRunner::check` and by four test suites | no tool, although it lives in `ltl.cpp` |

### 4.3 Mutant generation and SPIN-based verification

They are two capabilities with different needs, in one class.

- *run* (CLI-1): `gen-mutants -n 2` runs one `cpp` and writes `<model>_mutants/` next to the model. It runs no other command.
- *run* (SPIN-10): `createMutants(2)` followed by `killMutantsSpin(nullptr)` runs one `cpp`, one `spin -V`, three `spin -run` (the original and two mutants), six `gcc` and three `./pan`.
- *code*: `MutantAnalyzer::killMutants` (`mutantAnalyzer.cpp:135-139`) is the variant that uses DaedaluX's own checker instead of SPIN. It has no caller.

## 5. Current discovery and invocation mechanisms

### 5.1 `cpp`

- **Discovery:** `PATH`, through `sh`. No path is configured or recorded.
- **Invocation:** `system()`. The model path is quoted for the shell (`promela_loader.cpp:61-67`). The model is passed on standard input.
- **Working directory.** *run* (CPP-7): because `cpp` reads standard input, `#include "defs.h"` resolves against the working directory, not against the directory of the model. A model that includes a file next to itself loads when run from its own directory, and fails from any other. The code states this (`promela_loader.cpp:90-92`).
- **Files:** a private directory `$TMPDIR/daedalux-loader-XXXXXX` per load, removed by the destructor or at exit. *run*: none is left in `/tmp` after all the experiments, including the `exit(1)` paths.
- **Debug output:** `fsm_graphvis` in the working directory after every load (`promela_loader.cpp:121-124`). *run* (CPP-2): in a read-only working directory the open fails silently and the load succeeds.
- **Other outputs of the CLI** go next to the model, named after it without its extension: `<name>_mutants/`, `<name>_mutants.txt`, `<name>_trace_<n>.csv`, and `<mutant>_positive.csv` and `<mutant>_negative.csv` (*run* CLI-1, CLI-2, CPP-1). **INFERRED:** the directory of the model must be writable.

### 5.2 `ltl2ba`

- **Discovery:** `std::filesystem::current_path() / "../src/bin/ltl2ba"` (`ltl.cpp:13-14`), then `fs::exists` (line 17). *run* (LTL-2): `PATH` is not consulted. With `ltl2ba` on `PATH` and not at that place, the call fails.
- **Invocation:** `system()` on a concatenated string (`ltl.cpp:27`).
  - The path is not quoted. *run* (LTL-5): with a space in the working directory, `sh` runs the first word and the call fails.
  - The formula is placed between double quotes. *run* (LTL-8): `sh` expands it, so `[]($(echo expanded_by_sh))` reaches `ltl2ba` as `[](expanded_by_sh)`.
- **Files:** the never claim is redirected to `<cwd>/__formula.tmp`, read back, and removed on success only (`ltl.cpp:22-43`). The working directory must be writable (*run* LTL-7). The fixed name is #5.

### 5.3 `spin` and its C compiler

- **Discovery:** `PATH` for `spin`. SPIN finds `gcc` on `PATH` too.
- **Availability check:** `std::system("spin -V")` before the first use (`spinRunner.cpp:40`). It is the only runtime availability check in DaedaluX. *code*: `spinInstalledFlag` is a process-wide static, so the check is not repeated after its first success. `spin -V` prints its version on the standard output of the calling process.
- **Invocation:** `popen("spin -run <model>_temp")` (`spinRunner.cpp:67-68`), path not quoted. Standard output is captured. Standard error is not, so the compiler's warnings about `pan.c` appear on the caller's terminal (*run* SPIN-1). The exit status returned by `pclose` is discarded.
- **Files** (*run* SPIN-1, SPIN-2):
  - next to the model: `<model>_temp`, removed at the end. The directory of the model must be writable (SPIN-8);
  - in the working directory, by SPIN: `pan.pre`, `pan.c`, `pan.h`, `pan.m`, `pan.b`, `pan.p`, `pan.t`, `_spin_nvr.tmp`, `pan`;
  - left after the call: `pan`, and `<model>_temp.trail` when SPIN finds a violation.

### 5.4 `java` and the TVL jar

- **Discovery:** `PATH` for `java`. The jar is `./libs/tvl/TVLParser.jar`, relative to the working directory.
- **The command cannot be reached.** `tvl.cpp:69` tests `std::filesystem::copy_file(...) == 0`. `copy_file` returns `true` when it copies and throws when the destination exists, so the block that holds the `java` call never runs.
  - *run* (TVL-1): a first `loadFeatureModel` returns `false`, runs no command, and leaves `__workingfile.tvl` in the working directory.
  - *run* (TVL-2): a second call in the same directory throws `std::filesystem::filesystem_error` ("File exists"). The function's own `catch` only handles `std::ifstream::failure`.
  - *run*: no object and no test constructs a `TVL`. The CLI calls `loadFeatureModel` on a null `std::unique_ptr<TVL>` (`modelchecking_subcommand.cpp:73,77`).
- **The jar itself works** (*run* TVL-3): run by hand under OpenJDK 25, it writes the mapping and the clauses of `examples/example.tvl`. Its main class is compiled for Java 6 (class version 50).
- **The jar is not self-contained.** Its manifest has `Class-Path: libs/java-cup-11a-runtime.jar libs/org.sat4j.core.jar libs/JFlex.jar`, relative to the jar. *run* (TVL-3): copied alone, it fails with `NoClassDefFoundError: org/sat4j/specs/TimeoutException`.
- **Files, if the command ran:** `__workingfile.tvl`, `__mapping.tmp` and `__clauses.tmp`, fixed names in the working directory (#5).

### 5.5 `owl`

`FormulaSimplifier::simplify` runs `which owl` and returns its argument whatever the answer: the line that would run `owl` is a comment (`formulaSimplifier.hpp:20-26`). *run* (OWL-1): called directly, it runs `which owl` and returns the formula. The string `which owl` is in no object of the library, in no test executable and not in the CLI. The header is public and installed, and `formulas.hpp` includes it.

### 5.6 Where the two working-directory lookups can succeed

| Working directory | `<cwd>/../src/bin/ltl2ba` | `./libs/tvl/TVLParser.jar` |
|---|---|---|
| `<repo>/src` | the tracked aarch64 binary | found |
| `<repo>/build` (the `release` preset), or any directory directly below the repository root | the tracked aarch64 binary | not found |
| a test workspace under `ctest` | `<build>/tests/runtime/src/bin/ltl2ba`, which the build does not create | not found |
| an install prefix, or any other directory | not found | not found |

*run*: the tracked `src/libs/bin/ltl2ba` is x86-64 and runs here. No lookup leads to it.

### 5.7 Existing and usable

| Check today | What it establishes | What it does not (*run*) |
|---|---|---|
| `fs::exists(ltl2baPath)` | a file is at the path | LTL-3: the aarch64 binary passes the check, then `sh` reports `Exec format error` (status 126) |
| `spin -V` returns 0 | `spin` starts | SPIN-5: verification still cannot run without `gcc` |
| none for `cpp` | — | CPP-6: a `cpp` that exits 0 and writes nothing is accepted. The failure appears later as a syntax error |
| none for the jar | — | TVL-3: the jar without its `libs/` directory does not start |
| CMake configure | nothing about runtime tools | *run*: the configure log names none of them |

## 6. Runtime failure behavior

### 6.1 By tool and condition

| Tool | Condition | What the caller gets | Evidence |
|---|---|---|---|
| `cpp` | not on `PATH` | `Could not run the c preprocessor (cpp).` on standard error, then `exit(1)` inside the constructor. The caller's `catch` does not run. `atexit` handlers and static destructors do | *run* CPP-3a (CLI), CPP-3b (consumer); `promela_loader.cpp:95-98` |
| `cpp` | fails on the model, for example a missing `#include` | the same message and `exit(1)` | *run* CPP-4, CPP-7 |
| `cpp` | exits 0 without output | `Syntax error on line 0`, `exit(1)` | *run* CPP-6 |
| loader | `TMPDIR` names a directory that does not exist | `std::filesystem::filesystem_error`, which the caller can catch | *run* CPP-8 |
| loader | `TMPDIR` is not writable | message, `exit(1)` | *run* CPP-8b; `promela_loader.cpp:52-55` |
| loader | model missing; syntax error | message, `exit(1)` | *run* CPP-10, CPP-9 |
| `ltl2ba` | not at `<cwd>/../src/bin/ltl2ba` | `std::runtime_error`: `Could not find the ltl2ba binary at <path>`. No command is run | *run* LTL-1, LTL-2 |
| `ltl2ba` | present, not executable on this machine | `std::runtime_error`: `Could not transform the LTL formula into a never claim using ltl2ba.` An empty `__formula.tmp` is left | *run* LTL-3 |
| `ltl2ba` | malformed formula | the same exception. The diagnostic of `ltl2ba` is in `__formula.tmp`, which is left | *run* LTL-6 |
| `ltl2ba` | working directory read-only, or a space in its path | the same exception | *run* LTL-7, LTL-5 |
| `spin` | not on `PATH` | `Error: Spin is not installed.` on standard error, return value `false` | *run* SPIN-4 |
| `spin` | model file missing | message, `false` | *run* SPIN-9 |
| `spin` | `gcc` not on `PATH` | `true`, for a property that is violated | *run* SPIN-5, SPIN-5b |
| `spin` | model does not parse | `true` | *run* SPIN-6 |
| `spin` | working directory read-only | `true`, for a property that is violated | *run* SPIN-7 |
| `spin` | directory of the model read-only | `std::filesystem::filesystem_error` | *run* SPIN-8 |
| `spin` | `popen` fails | `std::runtime_error("popen() failed!")` | *code* `spinRunner.cpp:19-21`; not run |
| `java` / jar | any | not reachable. *code*: a non-zero status would return `false`, and `check` would print `Could not load the specified feature model file.` and `exit(1)` | `tvl.cpp:73-75`, `modelchecking_subcommand.cpp:77-80` |
| TVL | `check -f x.tvl`, first run | `Could not load the specified feature model file.`, exit status 1 | *run* TVL-4 |
| TVL | the same command again in the same directory | uncaught `filesystem_error`, abort, exit status 134 | *run* TVL-4 |
| `owl` | absent or present | the formula, unchanged | *run* OWL-1; *code* |

### 6.2 Tool failure and negative verification result

`spinRunner::check` returns one `bool` for every outcome (*run*, SPIN-1 to SPIN-9):

| Situation | Return value |
|---|---|
| the property holds | `true` |
| the property is violated | `false` |
| `spin` is not on `PATH` | `false` |
| the model file does not exist | `false` |
| `gcc` is not on `PATH` | `true` |
| the model does not parse | `true` |
| the working directory is read-only | `true` |

- *code* (`spinRunner.cpp:71-88`): the result starts at `true` and becomes `false` only when the standard output of SPIN contains one of four fixed texts. The exit status is not read. *run*: `pclose` returned 256 in SPIN-5 and SPIN-6.
- **INFERRED:** any other way for `spin -run` to stop early gives `true` as well, because the four texts are then absent.
- *code*: `MutantAnalyzer::checkMutants` (`mutantAnalyzer.cpp:222-255`) turns that value into "surviving" or "killed". By the same reading, a missing compiler makes every mutant survive and a missing `spin` kills every mutant. *run* (T-1d) agrees: `EnhanceSpecification_3Processes` passes without `spin` and with it, and fails with `spin` and no `gcc`.

For `cpp`, one message and one exit status cover a missing tool (status 127 from `sh`) and an error in the model (status 1 from `cpp`). The code tests `!= 0` only.

For `ltl2ba`, the exception text is the same for a binary of the wrong architecture (126), a malformed formula (1), an unwritable working directory (2) and a path with a space (126).

### 6.3 Where the library ends the process

- `promela_loader.cpp`: five `exit(1)`, at lines 54, 85, 97, 104 and 110. Line 97 is the only one about a tool.
- The parser and the lexer: `promela.y:39` and `309`, `promela.l:145` and `228`.
- The subcommands, which are compiled into the library: `modelchecking_subcommand.cpp:70`, `79`, `89` and `94`.

## 7. Distribution and installation assumptions

- *run* (INSTALL): `cmake --install` writes 150 files: `bin/daedalux_cli`, `lib/libdaedalux_lib.a`, `lib/cmake/daedalux/`, and the headers, with `formulas/formulaSimplifier.hpp` and `CLI11.hpp` among them. The TGZ package holds the same 150 files. No entry is an `ltl2ba`, a jar or anything from `src/libs` or `src/bin`.
- *run*: no CMake file mentions `ltl2ba`, `src/libs`, `src/bin`, Java or SPIN, and the build graph does not either.
- **Tracked tool files:**

  | Path | What it is | Used by |
  |---|---|---|
  | `src/bin/ltl2ba` | ELF, aarch64, added in `d0d1089` (2024-06-21). *run*: status 126 on x86-64 | the lookup, from the directories of section 5.6 |
  | `src/libs/bin/ltl2ba` | ELF, x86-64, added in `ec82629` (2025-06-27). *run*: works here | nothing |
  | `src/libs/ltl2ba/` | C sources with their own `CMakeLists.txt` and a GPL `LICENSE` file | nothing: no `add_subdirectory`. *run*: its configure fails under CMake 4.2.3 unless `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` is given, and it writes `config.h` into its own source directory |
  | `src/libs/tvl/TVLParser.jar`, `libs/*.jar` | 976 KB in four jars, added in `99c48c2` (2023-11-28). No licence file | the unreachable command of `tvl.cpp:73` |

- **The installed CLI** (*run* INSTALL-1, T-4): `gen-single-traces` works from an empty directory with a model given by absolute path. The Release binary contains no path of the build or source tree. The Debug binary contains 83 such strings, all names of source files: DaedaluX sources and headers, CUDD sources, and one stale `/home/slazreg/.../promela.y`. **INFERRED:** they are the file names of `assert` messages and of the generated parser, and none is used to locate anything.
- **Where the tools come from in the environments the repository describes** (*code*, not run):

  | Environment | `cpp`, `gcc` | `ltl2ba` | `spin` | `java` |
  |---|---|---|---|---|
  | CI (`ci.yml`, `ubuntu-24.04`) | the runner's GCC (**INFERRED**: no step installs it, and the cases that load models pass) | none | none: two cases are excluded as "SPIN is not installed" | not used |
  | `.devcontainer/Dockerfile` | the base C++ image | none | 6.5.2 built from source into `/usr/local/bin` | **UNKNOWN** |
  | `docker/Dockerfile`, final stage | `gcc-c++` | none | 6.5.2 copied from a build stage | `java-11-openjdk-devel` |
  | `README.md` | names none of the runtime tools | | | |

  **UNKNOWN:** whether either image builds today (#84 reports broken paths in the Docker scripts).
- **Python** (*code*): `.devcontainer/Dockerfile` installs `python3`, `pip`, a virtual environment, `openai` and `Scarlet-ltl`. `docker/Dockerfile` installs `python3` and `pip`, and copies a `test_scripts` directory that does not exist. CI installs and runs none of it, and the README does not mention it.
- **Build machine and machine of use.** Nothing records, at configure or install time, which tools the build machine had. A binary carries only the command strings.
- **Platform.** `system`, `popen`, `mkdtemp`, `getpid` and `<unistd.h>` are POSIX (#14).
- **Licences.** DaedaluX is MIT. `src/libs/ltl2ba/LICENSE` is the GNU GPL, and its README says version 2 or later. **UNKNOWN:** the terms of `TVLParser.jar`, which the repository does not record.

## 8. Test coverage and validation evidence

### 8.1 Capabilities exercised by the test suite

151 cases are enabled and 3 are disabled. The baseline is 126 passed and 25 failed (*run* T-0), as in [test-build-validation.md](test-build-validation.md).

| Capability | Cases that reach it | Today, without `ltl2ba` and SPIN | With the tool supplied |
|---|---|---|---|
| Model load, `cpp` | 71: 362 `cpp` runs in a serial pass, all returning 0 | 63 pass. 8 fail for other reasons (#39, #40) | — |
| Model load without `cpp` | the same 71 | *run* T-2: the 63 passing cases fail, with exit status 1. CTest reports `Failed`, as for a failed assertion | — |
| `ltl2ba` | 14: FormulaTest 5, FormulaCreatorTest 3, LTLTransformerTest 6 | 14 fail on the lookup, before any command | *run* T-1a: 14 pass. T-1a′, with the tracked aarch64 binary at the same place: 14 fail |
| `spin` | 4: SpinRunnerTest 3, MutantHandlerTest.EnhanceSpecification_3Processes | 17 `spin -V`, all status 127. Two cases fail (#41). `ModelIsIncorrect` passes because `check` returns `false` for a missing tool. `EnhanceSpecification_3Processes` passes | *run* T-1b: 4 pass |
| `spin` without `gcc` | the same 4 | — | *run* T-1d: `ModelIsIncorrect` and `EnhanceSpecification_3Processes` fail. The two "correct" cases pass because `check` returns `true` |
| `java`, TVL loading | 0 | — | — |
| `owl` | 0 | — | — |
| `fsm_graphvis` | 361 writes in a serial pass, into the working directory of the case: its workspace, or a temporary directory it moves to | — | — |

**Not exercised by any test:** the CLI executable (*run*: no test starts `daedalux_cli`), `fsmExplorer::checkFormula`, `renewClaimOfFile`, `appendClaim`, `MutantAnalyzer::killMutants`, `TVL::loadFeatureModel` and `loadFeatureModelDimacs`, the TVL printing functions (#133), `launchExecutionMarkovChain`, `FormulaSimplifier`, every Python script of section 3.3, and every failure path of section 6 except the missing `ltl2ba` and the missing `spin`. Nine TraceGeneratorTest cases write trace files, seven of them through the two Scarlet functions; nothing checks that Scarlet accepts them.

### 8.2 Suite totals

| Run | Passed | Failed | Disabled |
|---|---|---|---|
| T-0: baseline, `PATH=/usr/bin:/bin` | 126 | 25 | 3 |
| T-1a: `ltl2ba` (x86-64) at `<build>/tests/runtime/src/bin/ltl2ba` | 140 | 11 | 3 |
| T-1b: SPIN 6.5.2 on `PATH` | 128 | 23 | 3 |
| T-1c: both | 142 | 9 | 3 |
| T-1c′: both, `ctest -j8`, three runs | 142 | 9 | 3 |
| T-2: `cpp` not on `PATH` | 63 | 88 | 3 |

- The 9 remaining failures of T-1c are the six cases of #39, the two of #40 and the segfault of #35.
- The three `-j8` runs have the named outcomes of the serial run. The product still uses the fixed name `__formula.tmp` (#5); each case now has its own working directory (#143).
- *run* (T-3): `ctest --test-dir <build>` from `/`, and a test executable started from an empty directory, pass and leave nothing behind. The tests find their data through two absolute paths compiled into `daedalux_test_support`: `<build>/tests/data` and `<build>/tests/runtime`.

### 8.3 Experiments

Each experiment ran in a new, empty directory, under the logger.

| ID | Setup | Result |
|---|---|---|
| CPP-1 | CLI `gen-single-traces`, empty working directory, model elsewhere | status 0; one `cpp`; `fsm_graphvis` in the working directory; the trace next to the model |
| CPP-2 | the same, working directory read-only | status 0; opening `fsm_graphvis` fails silently |
| CPP-3a, 3b | `cpp` not on `PATH`: CLI, then the probe | status 1; `sh` returns 127; the probe's `catch` is not reached |
| CPP-4 | the model includes a file that does not exist | the same message as CPP-3, status 1 |
| CPP-6 | a `cpp` that is `/bin/true` | `Syntax error on line 0`, status 1 |
| CPP-7 | the model includes `defs.h`, which is next to it | fails from another directory, loads from the directory of the model |
| CPP-8, 8b | `TMPDIR` missing; `TMPDIR` read-only | `filesystem_error` caught by the probe; `exit(1)` |
| CPP-9, 10 | syntax error; missing model | `exit(1)` |
| LTL-1 | probe, empty working directory | `runtime_error`, "Could not find" |
| LTL-2 | `ltl2ba` on `PATH` only | the same error |
| LTL-3 | the tracked aarch64 binary at `<cwd>/../src/bin` | `Exec format error`, `runtime_error`, `__formula.tmp` left |
| LTL-4 | the x86-64 build at that place | the never claim; nothing left |
| LTL-5 | the same, with a space in the path | `runtime_error` |
| LTL-6 | malformed formula | `runtime_error`, `__formula.tmp` left |
| LTL-7 | working directory read-only | `runtime_error` |
| LTL-8 | a formula containing `$(echo ...)` | expanded by `sh` |
| SPIN-1 | property that holds | `true`; `spin -V`, `spin -run`, two `gcc`, `./pan`; `pan` left |
| SPIN-2 | property that is violated | `false`; `pan` and a `.trail` file left |
| SPIN-3 | no formula | `true` |
| SPIN-4 | `spin` not on `PATH` | `false` |
| SPIN-5, 5b | `spin` on `PATH`, no `gcc`; then `cc` and `cpp` but no `gcc` | `true` for the violated property |
| SPIN-6 | a model that does not parse | `true` |
| SPIN-7, 8 | working directory read-only; directory of the model read-only | `true`; `filesystem_error` |
| SPIN-9 | missing model | `false` |
| SPIN-10 | `createMutants(2)`, then `killMutantsSpin` | command counts of section 4.3 |
| TVL-1, 2 | `loadFeatureModel` once; twice | `false` and no command; `filesystem_error` |
| TVL-3 | the jar by hand; the jar alone; the command of `tvl.cpp:73` from a copy of `src/` | works; `NoClassDefFoundError`; works |
| TVL-4 | CLI `check`: without `-f`, with `-f`, and again | status 1, 1 and 134 |
| OWL-1 | `FormulaSimplifier::simplify` from the probe | runs `which owl`; returns the formula |
| CLI-1, 2, 3 | `gen-mutants`, `gen-traces`, a relative model path | `cpp` only; outputs next to the model |
| PY-1, PY-2 | `DeaduluxRunner` of `GPTExperiment`, imported from an empty directory: without, then with a link named `daedalux` | `CalledProcessError` and no trace; then two mutants and one trace, found through the CLI's standard output |
| PY-3 | the command line of `mutation_testing.py`, given to the current CLI | status 109, "The following arguments were not expected" |
| PY-4 | what `run_tests.py` looks for | `./deadalux` and `./test` do not exist |
| PY-6 | `.trace` files written during a test pass | nine files, none named `trace_report_mutex_scarlet.trace` |
| INSTALL-1 | the installed CLI from an empty directory | as CPP-1 |
| T-0 to T-3 | the test suite, section 8.2 | — |
| T-4 | Release build, `BUILD_TESTING=OFF` | no build path in the binary; behaves as the Debug CLI in CPP-1 and CPP-3a |
| STATIC | `nm -u` on the objects, `ldd`, `build.ninja`, the configure log | sections 3 and 4 |

### 8.4 Not established

- **UNKNOWN:** SPIN versions other than 6.5.2. The four texts that `spinRunner::check` looks for, and the name of the compiler SPIN runs, belong to SPIN.
- **UNKNOWN:** a preprocessor other than GCC's `cpp`; a JDK other than OpenJDK 25.
- **UNKNOWN:** macOS, Linux ARM64, and a Clang build. The aarch64 `src/bin/ltl2ba` was not run on an ARM machine.
- **UNKNOWN:** the tools actually present on the CI runner and in the two container images. Section 7 reads their definitions.
- **UNKNOWN:** the behaviour of the Java path inside DaedaluX, since no input reaches it. TVL-3 runs the same command by hand.
- **UNKNOWN:** whether the Python workflows of section 3.3 run end to end. They need `openai` with an API key, or `Scarlet`, and neither is installed here. Only `DeaduluxRunner` was run (PY-1, PY-2).
- **Not run:** the test suite in Release; `launchExecutionMarkovChain` and `mdp.sched`.

### 8.5 Reproduction

The logger, the probe and the scripts are not committed. The key commands:

```bash
git clone https://github.com/samilazreg-eng/DaedaluX.git src && git -C src checkout e341c7e
cmake -S src -B b -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=<cudd>
cmake --build b --parallel
(cd b && ctest --timeout 120)                                         # T-0: 126 / 25 / 3

# cpp
mkdir empty && cd empty
PATH=/nonexistent ../b/src/daedalux_cli gen-single-traces -f <model.pml> -l 5 -n 1   # CPP-3a: status 1

# ltl2ba: build the vendored sources in a copy, then put the binary where the tests look
cp -r src/src/libs/ltl2ba l2b-src
cmake -S l2b-src -B l2b -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && cmake --build l2b
mkdir -p b/tests/runtime/src/bin && cp l2b/ltl2ba b/tests/runtime/src/bin/
(cd b && ctest --timeout 120)                                         # T-1a: 140 / 11 / 3
file src/src/bin/ltl2ba src/src/libs/bin/ltl2ba

# SPIN without installing it
apt-get download spin && dpkg -x spin_*.deb spin
(cd b && PATH=$PWD/../spin/usr/bin:/usr/bin:/bin ctest --timeout 120)  # T-1c with the ltl2ba above: 142 / 9 / 3

# SPIN without gcc: a directory holding only spin and a cpp wrapper
(cd b && PATH=<that directory> ctest -R '^(SpinRunnerTest|MutantHandlerTest)\.')    # T-1d

# TVL jar
java -jar src/src/libs/tvl/TVLParser.jar -dimacs map.tmp clauses.tmp src/examples/example.tvl
unzip -p src/src/libs/tvl/TVLParser.jar META-INF/MANIFEST.MF

# static evidence
for o in $(find b/src -name '*.o'); do nm -u $o | grep -qwE 'system|popen' && echo $o; done
ldd b/src/daedalux_cli
cmake --install b --prefix prefix && find prefix -type f | wc -l       # 150
```

## 9. Gap analysis against Phase 5

"Validation needed" names what would have to be observed to call the gap closed. It does not name a design.

### 9.1 Requirements

| # | Requirement | Current behavior | Evidence | Consequence | Existing issue | Validation needed |
|---|---|---|---|---|---|---|
| 1a | Locate `ltl2ba` independently of the working directory | `<cwd>/../src/bin/ltl2ba`. `PATH` is ignored. The path is not quoted. The only tracked binary the lookup can reach is aarch64. Nothing builds or installs an `ltl2ba` | 5.2, 5.6, 7; LTL-1 to LTL-5; T-1a, T-1a′ | LTL translation works from no directory on x86-64 without manual setup. 14 test cases fail. An installed DaedaluX cannot translate a formula | #22. Test side: #139, and workstream D of #138 | The 14 cases, and a consumer of the installed package, translate a formula from the build tree, from an install prefix and from an unrelated directory, one of them with a space in its path, with no `src/bin` near the working directory. A binary of another architecture is reported as such |
| 1b | Locate the TVL jar independently of the working directory | `./libs/tvl/TVLParser.jar`, found only from `<repo>/src`. The jar needs its `libs/` directory beside it. The command is unreachable (`copy_file(...) == 0`). The jar is not installed | 5.4, 5.6; TVL-1 to TVL-4 | No way to load a TVL feature model. Fixing the path alone changes nothing observable | #22 for the path. #21 for `check` and the `copy_file` test. #5 for the fixed file names. The `libs/` requirement and the missing install are in no issue | A TVL model is loaded through DaedaluX from an unrelated directory and from an install prefix, and the result is asserted by a test. Today no test and no command can show it |
| 2a | A missing `cpp` is an error the caller can handle | `exit(1)` inside the constructor. The same for an error in the model. Four more `exit(1)` in the loader and four in the parser | 6.1, 6.3; CPP-3 to CPP-10 | A process that uses the library ends when a model fails to load. A missing tool cannot be told from a wrong model. Under `ctest` it reads as a failed assertion (T-2) | None. The roadmap proposes one (section 7, "New in this roadmap"); it was not opened | The probe of this report: with `cpp` absent, the caller receives an error it can inspect, and the process continues. The error separates "tool unavailable" from "the model is wrong" |
| 2b | A missing `spin` is an error the caller can handle | Returns `false`, the value of a violated property. With `spin` present and `gcc` absent, or any early stop of SPIN, returns `true` | 6.1, 6.2; SPIN-4 to SPIN-7; T-1d | A missing tool changes a verification verdict in both directions. `checkMutants` then reports every mutant killed, or every mutant surviving | #41 for the tests. #139 for the test contract. The return value of the product and the `true` on failure are in no issue | The seven situations of 6.2 give distinguishable outcomes. A run without `gcc` reports no verdict |
| 2c | A missing `ltl2ba` is an error the caller can handle | Already an exception. One text for four causes. `__formula.tmp` is left on failure | 6.1; LTL-3, LTL-5, LTL-6, LTL-7 | The caller can handle it, and cannot tell a bad formula from an unusable tool | #22 ("give a clear error when a tool is missing"). #5 for the file | A malformed formula and an unusable binary give different errors. No file is left |
| 2d | A missing `java` is an error the caller can handle | Unreachable. By the code: `false`, then `check` prints a message that does not mention Java | 6.1 | Cannot be observed today | #21, #22 | Depends on 1b |
| 3 | Remove or justify the `owl` check | `which owl`, in a public header. No caller. No effect on the result | 5.5; OWL-1 | No runtime effect today. The installed API carries a `system()` call that does nothing | None. Proposed in the roadmap, not opened | Either no `owl` text is left in the installed headers, or a document states what the check is for and a test runs it |
| 4 | No implicit debug output in the working directory | `fsm_graphvis` on every load. Also `trace/<n>.dot` when `./trace` exists (`stateToGraphViz.cpp:31`), `sym_table_graphviz.dot` (`check`), `cudd_info.txt` (the first `TVL` construction, `tvl.cpp:27-28` and `230`), `products` (`tvl.cpp:440`). Tool leftovers: `pan`, `*.trail`, `__formula.tmp` on failure, `__workingfile.tvl` | 5.1, 5.3, 5.4; CPP-1, CPP-2, SPIN-1, SPIN-2, LTL-3, TVL-1 | Every CLI run leaves `fsm_graphvis` where it was started. 361 writes per test pass, now inside per-case workspaces | #15 for `fsm_graphvis`. #134 for the unchecked `fopen` calls. #5 for the temporary names. The other debug files and the SPIN leftovers are in no issue | After each CLI subcommand and after a test pass under the logger, no file is written to the working directory unless the caller asked for it |
| 5 | No build option unless D10 makes a capability optional | No option concerns a runtime capability. The options are `BUILD_TESTING` and `DAEDALUX_REGENERATE_PROMELA_PARSER` | 3.2; `CMakeCache.txt` | No gap today | — | The option list is unchanged after Phase 5, or each new option cites its D10 decision |

### 9.2 Exit criterion

> The CLI and tests run from arbitrary working directories without knowledge of the repository layout.

| Part | Status | Evidence |
|---|---|---|
| CLI: `gen-mutants`, `gen-traces`, `gen-single-traces` | **Met, with two reservations.** They run from an empty directory, from a read-only one, installed or not, with an absolute or a relative model path. Reservations: `fsm_graphvis` is written to the working directory, and `#include "..."` in a model resolves against the working directory | CPP-1, CPP-2, CPP-7, CLI-1 to CLI-3, INSTALL-1, T-4 |
| CLI: `check` | **Not met.** It cannot run from any directory (#21). A second run in the same directory aborts | TVL-4 |
| Library: LTL translation, TVL loading | **Not met.** Both depend on the working directory being at a given place in the source tree | LTL-1, LTL-2, 5.6 |
| Tests: location | **Met.** `ctest --test-dir` from `/` and a test executable from an empty directory pass. The tests know the build tree through two compiled-in absolute paths, not through the working directory | T-3 |
| Tests: tools | **Not met.** The 14 LTL cases pass only when an `ltl2ba` is at `<build>/tests/runtime/src/bin/ltl2ba`, a path that mirrors `src/bin` of the repository | T-1a |

**Overall: not met.**

### 9.3 Outside the five requirements

No Phase 5 requirement names the Python tooling of section 3.3, and the exit criterion speaks of the CLI and the tests. The tooling is nevertheless bound to what Phase 5 touches: it starts the CLI by a relative path, under a name the build does not produce, and it reads the CLI's standard output. Related issues: #47 (the binary name in the README), #84 (the Docker image copies a `test_scripts` directory that does not exist), #28 to #32 (`csvtodtrace.py`). No issue covers the command line of `mutation_testing.py` or `scripts/tools/run_tests.py`.

## 10. Open questions and decisions required before implementation

Each item is a decision for the maintainer. The facts under it come from the sections above.

1. **D10, per capability.** The roadmap lists D10 as a decision to take; this report finds no record that it was taken.
   - No CLI command reaches LTL translation, SPIN verification or TVL loading today. Whether each one is required or optional is therefore a statement about the library API, or about commands that do not exist yet.
   - TVL loading cannot work in the current code, whatever the tools.
2. **D7, the origin of `ltl2ba`.** Facts that bear on it: the vendored `CMakeLists.txt` declares CMake 3.0 and fails to configure under CMake 4.2.3 without a policy override; its configure writes `config.h` into its source directory; two binaries of different architectures are tracked; the sources are under the GPL in an MIT project, and the program is run as a separate process.
3. **What "located" has to cover for the TVL jar:** the jar and the `libs/` directory its manifest names. Its licence is not recorded.
4. **Order of the TVL work.** Requirement 1b has no observable effect while `loadFeatureModel` never runs the parser and `check` cannot start (#21). Does Phase 5 include making that path work, or only stating where the jar is?
5. **Scope of "caller-handlable".** Does it cover only the missing tool (`promela_loader.cpp:97`), or the nine places where the loader and the parser end the process? The first leaves a library that still exits on a syntax error.
6. **Telling a missing tool from a failing input.** Today the only signal is the exit status of the shell. Is the distinction part of the contract? If it is, for `cpp` only, or for every tool?
7. **The result type of `spinRunner::check`.** A `bool` cannot carry "tool unavailable" or "tool failed". Changing it changes a public signature and `MutantAnalyzer::checkMutants`.
8. **Is the C compiler part of the SPIN capability?** SPIN 6.5.2 needs a `gcc` on `PATH`. A check of `spin` alone does not establish that verification can run.
9. **`owl`:** remove the check, or state its purpose. It has no caller and no effect, and it is in a public header.
10. **Which files requirement 4 covers:** `fsm_graphvis` only (#15), or also `trace/<n>.dot`, `sym_table_graphviz.dot`, `cudd_info.txt` and `products`? Are the files that SPIN leaves (`pan`, `*.trail`) in scope?
11. **`#include` in a model.** Is its resolution against the working directory part of "runs from arbitrary working directories", or the documented behaviour?
12. **Tests and the build tree.** The tests locate their data through absolute build-tree paths. Does the exit criterion accept that, or does it ask for tests that can run against an installed DaedaluX?
13. **Where availability is reported.** The roadmap gives configure a reporting role (#95) and asks for a check at the time of use. #139 owns the test side. Which tools does the product check at run time: all of them, or those of the capabilities D10 declares optional?
14. **Supported tool versions.** Only SPIN 6.5.2, GCC's `cpp` 15.2 and OpenJDK 25 were observed. Does the runtime contract name versions?
15. **The Python tooling.** Is it inside the runtime contract, or separate tooling with its own owner? DaedaluX runs no Python. The scripts depend on the name `daedalux` in the working directory, on the subcommand options and on the CLI's standard output. One uses a command line the CLI no longer accepts, one looks for an executable and a directory that do not exist, and two need packages that only the dev container installs.
16. **Coordination with open issues.** #22 owns both lookups, and its LTL part is also scheduled in Phase 4 (#138). #5 owns the fixed names next to them. #21 blocks any end-to-end check of TVL. #41 and #139 own the test side of SPIN. Which of these does Phase 5 take, and which stay where they are?
