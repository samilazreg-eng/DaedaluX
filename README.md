# daedalux

A high-performance C++ model checker for feature-based product lines, written in C++20.

---

## Table of Contents

1. [Install](#install)  
2. [Build & Test](#build--test)  
3. [Usage](#usage)  
4. [Example](#example)  
5. [Dependencies](#dependencies)  
6. [Contributing](#contributing)  
7. [Future Work](#future-work)

---

## Install

### From Source (Linux & macOS)

```bash
git clone https://github.com/yourorg/daedalux.git
cd daedalux
./scripts/ci/bootstrap.sh
```

That single command will:  
1. Install prerequisites (`cmake`, `ninja`) via `apt` or Homebrew.  
2. Configure, build, test, and install `daedalux` into `$HOME/.local/bin`.

After completion, verify with:

```bash
daedalux --help
```

### Prebuilt Archives

Download the latest `.tar.gz` or `.zip` from:

https://github.com/yourorg/daedalux/releases

```bash
# Unpack and run directly without installation
tar xzf daedalux-<version>-Linux.tar.gz   # or .zip
cd daedalux-<version>/build
./daedalux --help
```

### Docker

Build and run via Docker (no local dependencies required):

```bash
./scripts/docker/build_image.sh
```

Run Tests in Container
```bash
./scripts/docker/run_tests_in_container.sh
```

Run model checker inside container
```bash
docker run --rm -it \
  -v "$(pwd)/examples":/data \
  -w /data \
  daedalux:latest \
  check -ltl '[](danger-><>stopped)' example.pml
```

---

## Build & Test

If you ever need manual control over build steps:

1. **Configure** (out-of-source):

   ```bash
   mkdir -p build && cd build
   cmake .. -G Ninja      -DCMAKE_BUILD_TYPE=Release      -DCMAKE_INSTALL_PREFIX="$HOME/.local"      -DBUILD_TESTING=ON
   ```

2. **Build & Test**:

   ```bash
   cmake --build .
   ctest --output-on-failure
   ```

3. **Install**:

   ```bash
   cmake --install . --prefix "$HOME/.local"
   ```

Use the provided [CMakePresets.json](/CMakePresets.json) for one-command builds:

```bash
cmake --preset release
cmake --build --preset release
cmake --install build --prefix "$HOME/.local"
```

---

## Usage

```bash
# Basic invocation
daedalux <mode> [options] <path-to-promela-file>
```

### Modes & Options

#### Model Checking Modes
- `check`        : Stop at first property violation.
- `exhaustive`   : Explore all traces to classify valid products.
- `sample <n>`   : Randomly explore _n_ traces to find a valid product.
- `ksteps <n>`   : Bounded search exploring traces of length _n_.

#### LTL Properties
- `-ltl <expr>`         : Inline LTL formula to verify.
- `-ltlFile <path>`     : File containing one or more LTL properties.
- `-multiLtl <expr>`    : Multi-property LTL query.
- `-multiLtlFile <path>`: File with multiple multiLTL properties.

#### Feature Model
- `-fm <path>` : Path to the feature model (TVL). Omit if name matches Promela file.

#### Output Control
- `-nt` : Do not print the execution trace to the terminal.
- `-st` : Only show states when no variables have changed.

#### Debugging
- `-exec` : Execute the model without printing intermediate states.
- `-l <n>` : Stop after exploring _n_ states.
- `-s`     : Print static information (symbols, FSMs, MTypes).
- `-t`     : Retain generated temporary files.

---

## Example

Given the following product line in `example.pml`:

```promela
typedef features {
    bool Alarm;
}

bool safe = true;
bool danger, stopped = false;

active proctype Motor() {
  do
  :: safe ->
      if :: skip;
         :: safe = false; danger = true
      fi;
  :: danger ->
      gd :: Alarm -> danger = false; stopped = true;
         :: else -> skip; dg;
  :: stopped ->
      if :: skip;
         :: stopped = false; safe = true;
      fi;
  od
}
```

Run the checker:

```bash
./daedalux check -ltl '[](danger-><>stopped)' example.pml
```

By default, the tool writes its trace and results to `example.pml.out` in the current directory.

---

## Dependencies

Before building, install:

- A C++20-compatible compiler (GCC ≥10 or Clang ≥11)  
- CMake ≥3.21 (required by the project and its CMake presets)
- Ninja build system  
- Flex & Bison (for Promela parsing)  
- [CUDD](https://github.com/ivmai/cudd) (Binary Decision Diagrams library)

On Ubuntu:

```bash
sudo apt-get update && sudo apt-get install   build-essential cmake ninja-build flex bison libgmp-dev   libboost-all-dev
# CUDD (if packaged) or build from source
```

On macOS (Homebrew):

```bash
brew update && brew install cmake ninja flex bison boost gmp
# Install CUDD via Homebrew or from source
```

---

## Contributing

Please see [CONTRIBUTING.md](/CONTRIBUTING.md) for guidelines on code style, testing, and running CI.

### Code of Conduct

This project follows a [Code of Conduct](/CODE_OF_CONDUCT.md); please ensure you respect it when contributing.

---

## Future Work

- [ ] Support Timed Automata  
  - [ ] Extend input language and parser  
  - [ ] Integrate TA model-checking algorithms  
- [ ] CI pipeline (GitHub Actions)  
- [ ] Clean up repository (ignore or remove temp files)  
  - [ ] Consolidate `tests/` and `test_scripts/`  
  - [ ] Add traces and tmp files to `.gitignore`  
