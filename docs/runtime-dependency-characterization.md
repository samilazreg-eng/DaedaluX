# Runtime dependencies

Identification of the external tools DaedaluX uses at run time: what they are, where the code calls them, and how it locates them today. It is an input for Phase 5 of [build-modernization-roadmap.md](build-modernization-roadmap.md). It changes nothing in the code or in the build.

- Baseline: `main` at `e341c7e`.
- A runtime dependency is needed on the machine that runs DaedaluX. It is not needed to build it.

## Inventory

| Dependency | Used for | Called from | How it is called | How it is located |
|---|---|---|---|---|
| `cpp` | preprocessing a Promela model, on every model load | `src/promela/parser/promela_loader.cpp:94-95` | `system("cpp < '<model>' > '<scratch>/__workingfile.tmp.cpp'")` | `PATH` |
| `ltl2ba` | translating an LTL formula into a never claim | `src/core/logic/ltl.cpp:13-28` | `system("<path> -f \"!(<formula>)\" > <cwd>/__formula.tmp")` | fixed path relative to the working directory: `<cwd>/../src/bin/ltl2ba`. `PATH` is not used |
| `spin` | verifying a model or a mutant with SPIN | `src/mutants/spinRunner.cpp:40` and `67-68` | `system("spin -V")` once per process, then `popen("spin -run <model>_temp")` | `PATH` |
| `gcc` | compiling the verifier that SPIN generates | not called by DaedaluX: `spin -run` calls it | SPIN runs `gcc` twice, then the `./pan` it built | `PATH`, by SPIN |
| `java` and `TVLParser.jar` | converting a TVL feature model to DIMACS | `src/feature/tvl.cpp:73` | `system("java -jar ./libs/tvl/TVLParser.jar -dimacs __mapping.tmp __clauses.tmp __workingfile.tvl")` | `java`: `PATH`. The jar: fixed path relative to the working directory, `./libs/tvl/TVLParser.jar` |
| `owl` | nothing today | `include/daedalux/formulas/formulaSimplifier.hpp:32` | `system("which owl > /dev/null 2>&1")`. The call to `owl` itself is commented out (line 22) | `PATH`, through `which` |

All calls go through `system()` or `popen()`, so each one also needs `/bin/sh`.

These are the only places where DaedaluX starts another program: four source files and one header.

## Who uses each dependency

| Dependency | Component | Reached from |
|---|---|---|
| `cpp` | PROMELA (`promela_loader`) | the CLI subcommands `gen-mutants`, `gen-traces` and `gen-single-traces`; every library function that loads a model; 71 test cases |
| `ltl2ba` | CORE (`LTLClaimsProcessor`) | the library only: `formula::neverClaim`, `appendClaimToFile`, `renewClaimOfFile`, `fsmExplorer::checkFormula`; 14 test cases. No CLI subcommand |
| `spin`, `gcc` | MUTANTS (`spinRunner`) | the library only: `spinRunner::check`, `MutantAnalyzer::killMutantsSpin`, `enhanceSpecification`; 4 test cases. No CLI subcommand |
| `java`, the jar | FEATURE (`TVL::loadFeatureModel`) | the CLI subcommand `check` only. The command is not executed today: `check` stops before it (#21) |
| `owl` | FORMULAS (`FormulaSimplifier`) | no caller |

Mutant generation needs only `cpp`. Verifying the mutants with SPIN is a separate function and is the one that needs `spin` and `gcc`.

## Where the tools come from

| Dependency | In the repository | Provided by |
|---|---|---|
| `cpp`, `gcc` | no | the environment (a GCC installation) |
| `spin` | no | the environment. The dev container and the Dockerfile build SPIN 6.5.2 |
| `java` | no | the environment |
| `ltl2ba` | `src/bin/ltl2ba`: a Linux aarch64 executable, the one the lookup points to. `src/libs/bin/ltl2ba`: a Linux x86-64 executable, which no lookup points to. `src/libs/ltl2ba/`: the sources, which the build does not compile | the tracked binaries |
| `TVLParser.jar` | `src/libs/tvl/TVLParser.jar`, with three jars it needs in `src/libs/tvl/libs/` | the tracked jars |

- The two fixed paths depend on where DaedaluX is started. `<cwd>/../src/bin/ltl2ba` exists only when the working directory is directly below the repository root, for example `<repo>/build`. `./libs/tvl/TVLParser.jar` exists only when the working directory is `<repo>/src`.
- CMake does not look for any of these tools, and `cmake --install` installs none of them.
- The README does not list them.

## Not runtime dependencies

| Dependency | What it is |
|---|---|
| Flex, Bison | code generation, on request: the default build compiles the tracked parser sources |
| CUDD | compile and link dependency, linked statically |
| GoogleTest | test dependency |
| Graphviz | not used: DaedaluX writes `.dot` files and never runs `dot` |
| Python | not used by DaedaluX. The scripts under `scripts/tools/` and `examples/models/` are separate tooling: they start the DaedaluX CLI, `spin` and `gcc`, and two of them need the packages `openai` or `Scarlet-ltl` |

## How this was established

- Every `system()` and `popen()` call in `src/` and `include/` was read, and the compiled objects were checked for references to these functions.
- The CLI subcommands and the test suite were run under a logger that records every command started. SPIN 6.5.2 and an `ltl2ba` built from `src/libs/ltl2ba` were supplied for these runs, to observe the calls that need them.
- Environment: Ubuntu 26.04 (x86-64), GCC 15.2.0, OpenJDK 25.

Related issues: #22 (the two fixed paths), #21 (`check`), #41 (tests that need SPIN).
