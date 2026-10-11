#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

INSTALL_DEPS=false

usage()
{
    cat <<'USAGE'
Usage: scripts/validate-library.sh [--install-deps]

Run the complete local validation used before an AmperiaSTEM release:
  1. compile all educational examples;
  2. compile the TestingSensors configuration matrix;
  3. compile AllFeaturesReference;
  4. run Arduino Lint in Library Manager submission mode;
  5. run git diff --check.

Options:
  --install-deps  Install the Arduino libraries required by the examples first.
  -h, --help      Show this help.

Examples:
  scripts/validate-library.sh
  scripts/validate-library.sh --install-deps
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

if command -v arduino-lint >/dev/null 2>&1; then
    ARDUINO_LINT_BIN="$(command -v arduino-lint)"
elif [[ -x "$HOME/bin/arduino-lint" ]]; then
    ARDUINO_LINT_BIN="$HOME/bin/arduino-lint"
else
    echo "arduino-lint was not found in PATH or ~/bin." >&2
    exit 127
fi

if ! command -v git >/dev/null 2>&1; then
    echo "git was not found in PATH." >&2
    exit 127
fi

cd "$REPO_ROOT"

echo
echo "================================================================"
echo "Compiling educational examples"
echo "================================================================"

example_args=(--clean)
if [[ "$INSTALL_DEPS" == true ]]; then
    example_args=(--install-deps --clean)
fi
"$REPO_ROOT/scripts/compile-examples.sh" "${example_args[@]}"

echo
echo "================================================================"
echo "Compiling TestingSensors matrix"
echo "================================================================"
"$REPO_ROOT/scripts/compile-testing-sensors-matrix.sh"

echo
echo "================================================================"
echo "Compiling AllFeaturesReference"
echo "================================================================"
"$REPO_ROOT/scripts/compile-examples.sh"     --clean extras/reference/AllFeaturesReference

echo
echo "================================================================"
echo "Running Arduino Lint"
echo "================================================================"
"$ARDUINO_LINT_BIN"     --compliance permissive     --library-manager submit     --project-type library     --recursive=false     "$REPO_ROOT"

echo
echo "================================================================"
echo "Checking Git whitespace"
echo "================================================================"
git diff --check

echo
echo "PASS: complete AmperiaSTEM library validation succeeded."
