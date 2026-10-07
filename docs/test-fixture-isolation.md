# Mutable test fixture isolation

`FormulaCreatorTest`, `MutantGenerationTest`, `MutantHandlerTest`, and
`TraceGeneratorTest` rewrite model inputs or generate files next to those inputs.
Each case in these suites creates a `TestWorkspace` during `SetUp()`, before
constructing `TestFilesUtils` or loading a model.

The workspace uses `mkdtemp` to reserve a unique directory and recursively copies
only `test_files/` from the configured `DAEDALUX_TEST_FIXTURES_DIR`. The shared
build-tree fixture copy remains input only for these suites. All model paths
resolve into the private copy, including the eleven fixtures identified in #86
and `mutants/flows/flows.pml` from #72. Generated `<model>_mutants/` directories
also belong to that case.

The process working directory temporarily becomes the workspace so incidental
relative outputs stay there as well. `TearDown()` restores the previous directory
and deletes the workspace; RAII also handles exceptions and early test returns.
Each subsequent case or GoogleTest repeat creates a fresh copy. The directory
change is process-wide, so background threads and child processes must finish
before the case exits. Abrupt process termination can leave a temporary directory,
but a subsequent run still reserves a different directory.

`FixtureWorkspaceTest` checks that claim rewrites preserve the shared inputs,
consecutive cases start fresh and remove their outputs, and eight concurrent
processes can repeatedly overwrite the same relative fixture and mutant filename
without seeing another process's content.

For validation, run consecutive serial and parallel selections without a
reconfigure between runs and compare named pass/fail/skip/disabled outcomes.
Hash both `examples/test_files/` and `<build>/test_fixtures/test_files/` before and
after execution. The #72 reproduction can also be stressed directly:

```sh
ctest --test-dir build -j2 --repeat until-fail:40 --output-on-failure \
  -R '^MutantGenerationTest\.Generate(Many)?MutantsFlows$'
```

This change addresses #86 and its #72 reproduction. It does not enable parallel
CI: the remaining Phase 4 work in #138 includes the other suites' fixture
lifecycle (#87), append-test isolation (#44), LTL temporary files and tool lookup
(#5/#22), and runtime dependency handling. Existing product failures remain
separate issues. CI retains its serial execution and known-failure exclusions.
