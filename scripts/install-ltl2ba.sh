#!/usr/bin/env bash
# Usage: ./scripts/install-ltl2ba.sh <install-prefix>
# Example: ./scripts/install-ltl2ba.sh "$HOME/.local"

set -euo pipefail

prefix="${1:?Usage: $0 <install-prefix>}"

version=1.3
url="https://lsv.ens-paris-saclay.fr/~gastin/ltl2ba/ltl2ba-${version}.tar.gz"
sha256="912877cb2929cddeadfd545a467135a2c61c507bbd5ae0edb695f8b5af7ce9af"

# Create temporary workspace
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# Download and verify the official source archive
curl --fail --location \
    --output "$work/ltl2ba.tar.gz" \
    "$url"

echo "${sha256}  $work/ltl2ba.tar.gz" | sha256sum --check -

# Extract sources
mkdir -p "$work/src"

tar -xzf "$work/ltl2ba.tar.gz" \
    --strip-components=1 \
    -C "$work/src"

# Build using the upstream Makefile 
make -C "$work/src" \
    CC=${CC:-cc} \
    CFLAGS="-O2 -DNXT" \
    ltl2ba

# Verify executable
echo "Verifying ltl2ba executable..."

# Smoke tests for LTL2BA
formulas=(
    # Atomic propositions and constants
    "a"
    "true"
    "false"

    # Boolean operators
    "! a"
    "a && b"
    "a || b"
    "a -> b"
    "a <-> b"
    "!(a && b)"

    # Temporal operators
    "X a"
    "[] a"
    "<> a"
    "a U b"
    "a V b"

    # Nested temporal operators
    "X (a U b)"
    "[] <> a"
    "<> [] a"
    "[] (a -> X b)"
    "<> (a && X b)"

    # Typical LTL properties
    "[] (request -> <> response)"
    "[] !(critical && error)"
    "[] (request -> (waiting U response))"
    "[] (a -> <> (b && c))"
    "[] (a -> X (b U c))"
    "(a U b) && (c U d)"
)

for formula in "${formulas[@]}"; do

    if ! "$work/src/ltl2ba" -f "$formula" > /dev/null; then
        echo "FAIL: $formula" >&2
        exit 1
    fi
done

echo "ltl2ba executable verified successfully."
    
# Install to user-provided prefix
install -Dm755 \
    "$work/src/ltl2ba" \
    "$prefix/bin/ltl2ba"

echo "Installed: $prefix/bin/ltl2ba"