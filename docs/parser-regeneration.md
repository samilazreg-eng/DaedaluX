# Why a clean checkout regenerates the tracked parser files

Investigation report for [#61](https://github.com/samilazreg-eng/DaedaluX/issues/61). It explains the current behaviour and changes nothing in the build. Fixes are left to separate issues.

Findings are classified as in [build-characterization.md](build-characterization.md):

- **OBSERVED**: reproduced by a command recorded in this investigation.
- **INFERRED**: interpretation derived from observations.
- **UNKNOWN**: not established by the current evidence.

## Setup

- Commit: `410b514` (`main`; the parser rules are unchanged since `baseline/2026-09`)
- Environment: WSL Ubuntu 26.04 on ext4, CMake 4.2.3, Ninja 1.13.2, GNU Make, Bison 3.8.2 (invoked as `yacc`), Flex 2.6.4, Git 2.53.0
- Every experiment starts from a fresh `git clone`, configured out of source (`cmake -S <clone> -B <clone>/build`).
- Ninja runs use `-d explain`, which prints the reason why each edge is considered dirty.

## Short answer

The build writes its generator outputs into the source tree, onto the tracked files, and both CMake generators decide that those files are stale on a fresh build directory. The reason differs per generator:

- **Ninja** (the generator the README documents) regenerates `lex.yy.cpp`, `y.tab.cpp` and `y.tab.hpp` on the first build of **every new build directory**, whatever the file timestamps, because a new build directory has no `.ninja_log` entry for these outputs.
- **Unix Makefiles** regenerates only `lex.yy.cpp`, because Git checks out `lex.yy.cpp` just before `promela.l` and the output is therefore a little older than its input.

The regenerated files differ from the tracked ones **only** in absolute paths. Their generated code is identical to the tracked copies.

## Parser inputs and outputs

| File | Role | Produced by | Declared to CMake | Written where |
|---|---|---|---|---|
| `src/promela/parser/promela.y` | input | hand-written | `DEPENDS` of the Bison command | source tree |
| `src/promela/parser/promela.l` | input | hand-written | `DEPENDS` of the Flex command | source tree |
| `src/promela/parser/y.tab.cpp` | output | `yacc -y -d -o <abs>/y.tab.cpp <abs>/promela.y` | `OUTPUT` | source tree, overwriting the tracked copy |
| `src/promela/parser/y.tab.hpp` | output | the same Bison command (`-d`) | **no** | source tree, overwriting the tracked copy |
| `src/promela/parser/lex.yy.cpp` | output | `flex -o <abs>/lex.yy.cpp <abs>/promela.l` | `OUTPUT` | source tree, overwriting the tracked copy |
| `src/promela/parser/lexer.h` | output of `%option header-file="lexer.h"` in `promela.l` | the same Flex command | **no** | **build directory** (Flex writes the relative header name into its working directory, which is `${CMAKE_BINARY_DIR}`) |

All four generated files are tracked, and `.gitignore` excludes none of them.

## Findings

### Dependency chain (Q1)

- **OBSERVED:** The chain is `daedalux_lib` → object library `daedalux_promela` → `add_dependencies(daedalux_promela promela_parser)` → custom target `promela_parser` → the two `add_custom_command` rules (`CMakeLists.txt` lines 18–44, 164–166). `y.tab.cpp` and `lex.yy.cpp` are also sources of `daedalux_promela`, so every build of the library, the CLI or a test runs the two rules.
- **OBSERVED:** Configuring alone (`cmake -S … -B …`), reconfiguring, or regenerating the build system after touching `CMakeLists.txt` leaves the checkout clean. Regeneration happens only during the build step.
- **OBSERVED:** A full default-target Ninja build of a fresh clone modifies exactly `lex.yy.cpp`, `y.tab.cpp` and `y.tab.hpp`. The build completes (`184/184`), and the lexer and parser objects are then recompiled from the regenerated sources.

### Why Ninja regenerates (Q1, Q3)

- **OBSERVED:** On a fresh Ninja build directory, `-d explain` reports `command line not found in log` for both outputs. When the checkout's timestamps also make `lex.yy.cpp` older than `promela.l` (see the next section), Ninja reports that reason first for the lexer.
- **OBSERVED:** After touching the four tracked outputs so they are newer than both inputs, a fresh Ninja build directory still regenerates both, again with `command line not found in log`. Timestamps therefore do not prevent the regeneration.
- **INFERRED:** Ninja rebuilds any output of a non-generator edge that has no entry in its build log. A new build directory starts with an empty log, so with Ninja the tracked copies are always overwritten on the first build, from any checkout and at any timestamp.

### Why Unix Makefiles regenerates only the lexer (Q1, Q3)

- **OBSERVED:** Git checks out the parser directory in index order: `lex.yy.cpp`, `lexer.h`, `promela.l`, `promela.y`, `promela_loader.cpp`, `y.tab.cpp`, `y.tab.hpp`.
- **OBSERVED:** In 30 of 30 fresh clones, `lex.yy.cpp` was older than `promela.l` (by about 0.5 ms), and `y.tab.cpp` was never older than `promela.y` (both usually share the same timestamp).
- **OBSERVED:** With Unix Makefiles, a fresh clone regenerates `lex.yy.cpp` only. With outputs touched newer than their inputs, it regenerates nothing.
- **INFERRED:** Make compares timestamps only. The lexer is regenerated because its output happens to be written before its input, and the parser is not because its output is written after. The outcome depends on checkout order and timestamp resolution, not on content.
- **UNKNOWN:** Why the lexer output and its input always receive different timestamps here while the parser files share one. The outcome on other filesystems, on Windows and on macOS has not been tested.

### What changes, and why (Q3)

- **OBSERVED:** The Ninja build changes 274 lines of `lex.yy.cpp`, 920 lines of `y.tab.cpp` and 10 lines of `y.tab.hpp` (added plus removed). Every changed line is either a `#line N "<absolute path>"` directive or, in `y.tab.hpp`, the include guard that Bison derives from the absolute output path:

  ```diff
  -#ifndef YY_YY_HOME_SLAZREG_WORK_RESEARCH_DAEDALUX_SRC_PROMELA_PARSER_Y_TAB_HPP_INCLUDED
  +#ifndef YY_YY_HOME_SAMIL_DX_I61_NINJA_A_SRC_SRC_PROMELA_PARSER_Y_TAB_HPP_INCLUDED
  -#line 71 "/home/slazreg/Work/Research/daedalux/src/promela/parser/promela.y"
  +#line 71 "/home/samil/dx/i61/ninja/a/src/src/promela/parser/promela.y"
  ```

- **OBSERVED:** After both absolute source prefixes are replaced by a placeholder, the regenerated `lex.yy.cpp` and `y.tab.cpp` are byte-identical to the tracked copies. `y.tab.hpp` differs only by the path-derived include guard.
- **INFERRED:** The paths are absolute because CMake passes absolute paths to `flex` and `yacc`, and both tools copy those paths into `#line` directives (Flex also names its own output file). The tracked copies were generated by this same rule on the original author's machine, from the current `promela.y` and `promela.l`, with the same tool versions (Bison 3.8.2, Flex 2.6.4). The regeneration is therefore not caused by stale content.
- **UNKNOWN:** Whether other Flex or Bison versions would also change the generated code. Only the versions above were tested.

### Incrementality and reproducibility (Q4)

- **OBSERVED:** A second build without changes does no work, both for `promela_parser` and for the full default target, with both generators.
- **OBSERVED:** Touching `promela.y` regenerates `y.tab.cpp` and `y.tab.hpp` only. Touching `promela.l` regenerates `lex.yy.cpp` only.
- **OBSERVED:** Deleting `y.tab.hpp` or the tracked `lexer.h`, then rebuilding, reports `no work to do` and does not restore the file. Neither file is a declared output, so the build cannot recreate it.
- **OBSERVED:** Clones at two different paths (`…/a/src` and `…/b/deeper/src`) produce outputs that are identical after path normalization, except for the path-derived include guard. The generated files are therefore deterministic for a given path and tool versions, but they are **not path-independent**.
- **OBSERVED:** The Ninja and Makefiles builds produce identical `lex.yy.cpp` files, again after path normalization.

### `lexer.h` is stale and never rebuilt in place (Q2)

- **OBSERVED:** The Flex command runs in the build directory (`cd <build> && flex …`), so `%option header-file="lexer.h"` writes a new `lexer.h` into the build directory. The tracked `src/promela/parser/lexer.h` is never modified by the build.
- **OBSERVED:** `promela_loader.cpp` includes `"lexer.h"` from its own directory, which is the tracked copy. After a full build, Ninja's dependency records (`ninja -t deps`) show that `promela_loader.cpp.o` depends on `src/promela/parser/lexer.h`, and that no object depends on the `lexer.h` in the build directory.
- **OBSERVED:** The tracked `lexer.h` is byte-identical to what Flex 2.6.4 generates, from within the parser directory, from `promela.l` as of `32f2b6e` (2024-02-08). `promela.l` changed in `ec82629` (2025-06-27) without `lexer.h` being regenerated. Compared with a header generated from the current `promela.l`, the only difference is one directive: `#line 227 "promela.l"` versus `#line 230 "…/promela.l"`.
- **INFERRED:** The tracked `lexer.h` is stale, but only in one line-number directive, so it still declares the same interface as the current lexer.
- **OBSERVED:** The repository's first commit (`fb2aa4d`) already contained a second `lexer.h` at the repository root, next to `parser/lexer.h`.
- **INFERRED:** This is consistent with Flex having written the header into whatever directory a build ran in, as it still does today.

### Role of the tracked generated files (Q5)

- **OBSERVED:** The generated files have been tracked since the first commit (`fb2aa4d`, 2022-10-15), which already contained the same generation rules writing into the source tree. They were updated together with their inputs in `32f2b6e` and `ec82629`.
- **OBSERVED:** The README lists Flex and Bison as build prerequisites and documents a Ninja build.
- **OBSERVED:** Without `flex`, `bison` and `yacc` on `PATH`, a fresh Ninja build fails (`FAILED: [code=127]` on `lex.yy.cpp`), even when the tracked outputs are newer than their inputs. A fresh Makefiles build also fails on `lex.yy.cpp`. It succeeds only if the tracked outputs are first made newer than their inputs.
- **INFERRED:** In the build as it is, the tracked `y.tab.cpp`, `y.tab.hpp` and `lex.yy.cpp` are **cached outputs** that happen to live where the build writes. They are not usable as a fallback, since the documented Ninja build always regenerates them and requires the tools. The tracked `lexer.h` and, when the build does not regenerate it, `y.tab.hpp` are real **build inputs**: the build compiles against them but cannot recreate them.
- **INFERRED:** The committed copies carry the author's absolute paths, which indicates that they were committed after a local build rather than generated on purpose for distribution.
- **UNKNOWN:** Whether the tracked copies were ever meant to let users build without Flex and Bison. No document or commit message states an intent.

## Answers to the issue's questions

1. **Trigger:** the first build of the `promela_parser` target, reached by any build of `daedalux_promela` and therefore of the library, the CLI and the tests. With Ninja, the missing build-log entries of a new build directory trigger it. With Makefiles, the checkout timestamp order does.
2. **Declarations:** `y.tab.cpp` and `lex.yy.cpp` are declared. `y.tab.hpp` (Bison `-d`) and `lexer.h` (Flex `header-file`) are not. `lexer.h` is even written to a different directory from the tracked copy.
3. **Differences:** only absolute paths in `#line` directives and the path-derived include guard. The generated code is identical.
4. **Incrementality and reproducibility:** a second unchanged build does no work. Outputs are reproducible across checkouts only after path normalization.
5. **Role:** cached outputs in practice (never used as-is by a Ninja build). `lexer.h` and `y.tab.hpp` also act as undeclared inputs. The original intent is unknown.

## Proposed follow-up issues

These are proposals for separate implementation issues. Nothing here has been changed.

1. **Decide the role of the generated files**: stop tracking them and generate into the build tree, or keep them as an explicit fallback. This extends #33.
2. **Declare every generated output**: `y.tab.hpp` and `lexer.h` (for example as `BYPRODUCTS` or additional `OUTPUT`s), so that deleting them or depending on them works.
3. **Make the Flex header land beside the lexer, or stop tracking it**: today the build writes `lexer.h` into the build directory and compiles against a stale tracked copy.
4. **Remove absolute paths from generated sources** (for example relative `#line` names, or generation in the build tree), so outputs are path-independent.
5. **Investigate** whether other Flex and Bison versions change the generated code, and how the Makefiles timestamp race behaves on other filesystems and platforms.

## Reproduction

The scripts used for this report are not committed. The key commands are:

```bash
git clone https://github.com/samilazreg-eng/DaedaluX.git src && git -C src checkout 410b514
cmake -S src -B src/build -G Ninja -DCMAKE_BUILD_TYPE=Debug
ninja -C src/build -d explain promela_parser   # why each parser edge runs
git -C src status --porcelain                  # which tracked files changed
ninja -C src/build -d explain promela_parser   # second build: no work to do
stat -c '%.9Y %n' src/src/promela/parser/*     # checkout timestamps
```
