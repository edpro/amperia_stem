#!/usr/bin/env bash
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SKETCH="$REPO_ROOT/extras/manual-tests/TestingSensors/TestingSensors.ino"

INSTALL_DEPS=false

usage()
{
    cat <<'USAGE'
Usage: scripts/compile-testing-sensors-matrix.sh [--install-deps]

Compile TestingSensors once for every supported local-sensor configuration.
The script restores TestingSensors.ino byte-for-byte before exiting.

Options:
  --install-deps  Install the required Arduino libraries first.
  -h, --help      Show this help.

Examples:
  scripts/compile-testing-sensors-matrix.sh
  scripts/compile-testing-sensors-matrix.sh --install-deps
USAGE
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --install-deps)
            INSTALL_DEPS=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

if [[ ! -f "$SKETCH" ]]; then
    echo "TestingSensors sketch not found: $SKETCH" >&2
    exit 2
fi

if command -v python3 >/dev/null 2>&1; then
    PYTHON=python3
elif command -v python >/dev/null 2>&1; then
    PYTHON=python
else
    echo "python3/python was not found in PATH." >&2
    exit 127
fi

if ! command -v sha256sum >/dev/null 2>&1; then
    echo "sha256sum was not found in PATH." >&2
    exit 127
fi

if [[ "$INSTALL_DEPS" == true ]]; then
    "$REPO_ROOT/scripts/compile-examples.sh" \
        --install-deps extras/manual-tests/TestingSensors || exit $?
fi

variants=(
    USE_BH1750
    USE_BME280
    USE_BMP280
    USE_MAX9814
    USE_KY037
    USE_DHT22
    USE_MPU6050
)

tmp_dir="$(mktemp -d)"
backup="$tmp_dir/TestingSensors.ino"
cp "$SKETCH" "$backup"

before_sha="$(sha256sum "$SKETCH" | awk '{print $1}')"

restore()
{
    cp "$backup" "$SKETCH"
    rm -rf "$tmp_dir"
}
trap restore EXIT INT TERM

failures=()

for variant in "${variants[@]}"; do
    echo
    echo "================================================================"
    echo "TestingSensors configuration: $variant"
    echo "================================================================"

    if ! VARIANT="$variant" "$PYTHON" - "$SKETCH" <<'PY'
import os
import re
import sys
from pathlib import Path

path = Path(sys.argv[1])
variant = os.environ["VARIANT"]

names = (
    "USE_BH1750",
    "USE_BME280",
    "USE_BMP280",
    "USE_MAX9814",
    "USE_KY037",
    "USE_DHT22",
    "USE_MPU6050",
)

text = path.read_text(encoding="utf-8")

pattern = re.compile(
    r"^(?://\s*)?#define\s+("
    + "|".join(map(re.escape, names))
    + r")\s*$",
    re.MULTILINE,
)

seen = []

def replace(match):
    name = match.group(1)
    seen.append(name)
    if name == variant:
        return f"#define {name}"
    return f"// #define {name}"

updated = pattern.sub(replace, text)

if sorted(seen) != sorted(names):
    raise SystemExit(
        "Could not identify exactly the seven sensor-selection #define lines. "
        f"Found: {seen}"
    )

path.write_text(updated, encoding="utf-8", newline="\n")
PY
    then
        echo "FAIL: could not select $variant" >&2
        failures+=("$variant")
        continue
    fi

    if "$REPO_ROOT/scripts/compile-examples.sh" \
        extras/manual-tests/TestingSensors; then
        echo "PASS: $variant"
    else
        echo "FAIL: $variant"
        failures+=("$variant")
    fi
done

restore
trap - EXIT INT TERM

after_sha="$(sha256sum "$SKETCH" | awk '{print $1}')"

echo
echo "================================================================"
echo "TestingSensors matrix summary"
echo "================================================================"

if [[ "$before_sha" != "$after_sha" ]]; then
    echo "ERROR: TestingSensors.ino was not restored exactly." >&2
    exit 3
fi

echo "Original TestingSensors.ino restored: $after_sha"

if (( ${#failures[@]} == 0 )); then
    echo "PASS: all ${#variants[@]} configurations compiled successfully."
    exit 0
fi

echo "FAIL: ${#failures[@]} configuration(s): ${failures[*]}" >&2
exit 1
