#!/usr/bin/env bash
# Usage: ./scripts/install-tvl-parser.sh <install-prefix>
# Example: ./scripts/install-tvl-parser.sh "$HOME/.local"

set -euo pipefail

prefix="${1:?Usage: $0 <install-prefix>}"

version=20100804
url="https://projects.info.unamur.be/tvl/files/TVLParser-${version}.zip"

# Check Java availability
if ! command -v java >/dev/null 2>&1; then
    echo "ERROR: Java 6 or higher is required to run TVL Parser but was not found in PATH." >&2
    exit 1
fi

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# Download the official TVL Parser distribution
curl --fail --location --show-error \
    --output "$work/TVLParser.zip" \
    "$url"

# Extract the distribution
unzip -q "$work/TVLParser.zip" -d "$work/src"

# Locate the parser JAR
jar=$(find "$work/src" -type f -name TVLParser.jar -print -quit)

if [[ -z "$jar" ]]; then
    echo "ERROR: TVLParser.jar not found in distribution" >&2
    exit 1
fi

jar_dir=$(dirname "$jar")

# Create a small feature model for smoke testing
cat > "$work/test.tvl" <<'MODEL'
root FD {
    group allOf {
        opt Alarm
    }
}
MODEL

# Verify TVL-to-DIMACS conversion
echo "Testing TVL Parser DIMACS conversion..."

(
    cd "$jar_dir"

    java -jar TVLParser.jar \
        -dimacs \
        "$work/mapping.txt" \
        "$work/clauses.txt" \
        "$work/test.tvl"
)

test -s "$work/mapping.txt"
test -s "$work/clauses.txt"

echo "PASS: TVL-to-DIMACS conversion"

# Install the parser distribution, preserving nearby auxiliary libraries
destination="$prefix/share/daedalux/tvl"

mkdir -p "$destination"
cp -R "$jar_dir/." "$destination/"

echo "Installed: $destination/TVLParser.jar"
