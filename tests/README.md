# Tests

The tests are built with DaedaluX when `BUILD_TESTING` is `ON`, which is the default.
[CMakeLists.txt](CMakeLists.txt) describes the whole test build.

## Layout

The directory of a `.cpp` file decides what it is.

| Directory | Content | Built as |
|---|---|---|
| `unit/`, `integration/` | test sources, named `test_<name>.cpp` | one executable per source |
| `support/` | code shared by the tests: `main`, the workspace of a case, the paths of the test data | the library `daedalux_test_support`, linked into every test executable |
| `data/` | test data | not compiled; copied to `<build>/tests/data` |

A `.cpp` file anywhere else below `tests/`, or under another name in `unit/` or `integration/`, stops the configure with a message that names it.

## Adding a test

Create `test_<name>.cpp` below `tests/unit/` or `tests/integration/`, in any subdirectory. Directory names and `<name>` use letters, digits, `_` and `-`.
Nothing has to be listed: the next build finds the file.

`daedalux_add_test`, in `CMakeLists.txt`, creates the executable and registers its cases. It is the only place that does so.

- **Target.** The path below `tests/`, with dots for slashes and without the extension: `tests/unit/core/symbol/test_symbol.cpp` is the target `unit.core.symbol.test_symbol`, built as `<build>/tests/unit.core.symbol.test_symbol`. Two sources with the same file name in two directories are two targets.
- **Link.** `daedalux_lib` and `daedalux_test_support`. The source does not define `main`.
- **CTest.** Each case is one test, named `<Suite>.<Case>`, with the label `unit` or `integration` of its first directory and a 120 s timeout.

## Adding shared code

Put the `.cpp` file below `tests/support/`. It is compiled into `daedalux_test_support`, and never as a test executable.

## Running

```bash
ctest --test-dir build                               # all tests
ctest --test-dir build -L unit                       # one label
ctest --test-dir build -R '^SymbolTableTest\.'       # one suite
cmake --build build --target unit.core.symbol.test_symbol
```
