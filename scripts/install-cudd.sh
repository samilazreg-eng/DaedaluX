#!/usr/bin/env bash
# Build CUDD 4.0.0 and install it into a prefix, as a CMake package.
#
# DaedaluX requires CUDD but does not provide it. Run this once, then pass the
# prefix to DaedaluX with -DCMAKE_PREFIX_PATH=<prefix>.
#
# Usage: scripts/install-cudd.sh <prefix>
# Needs: git, cmake, ninja, and a C and C++ compiler (CC and CXX are honoured).
set -euo pipefail

# Upstream has no 4.0.0 tag: this is the validated head of its 4.0.0 branch
# (docs/cudd-4-compatibility.md).
CUDD_REPOSITORY=https://github.com/cuddorg/cudd.git
CUDD_COMMIT=d1857bfc59f4b09d0aafbdc408221a5a0ac8995e

prefix=${1:?usage: $0 <prefix>}

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

git init --quiet "$work/src"
git -C "$work/src" fetch --quiet --depth 1 "$CUDD_REPOSITORY" "$CUDD_COMMIT"
git -C "$work/src" checkout --quiet FETCH_HEAD

cmake -S "$work/src" -B "$work/build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$prefix"
cmake --build "$work/build"
cmake --install "$work/build"
