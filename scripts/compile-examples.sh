#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

FQBN="${FQBN:-arduino:renesas_uno:unor4wifi}"
COMPILE_JOBS="${COMPILE_JOBS:-0}"

# ArduinoCore-renesas 1.5.2 emits a known discarded-qualifiers warning from
# its own C sources. Keep it hidden by default without suppressing warnings from
# sketches or C++ library code. Set C_WARNING_SUPPRESSIONS to an empty value to
# show it.
if [[ -z "${C_WARNING_SUPPRESSIONS+x}" ]]; then
    C_WARNING_SUPPRESSIONS="-Wno-discarded-qualifiers"
fi

INSTALL_DEPS=false
CLEAN_BUILD=false

usage()
{
    cat <<'USAGE'
Usage: scripts/compile-examples.sh [options] [example ...]

Compile all examples, or only the examples named on the command line.

Options:
  --install-deps  Install the libraries required by the examples first.
  --clean         Disable cached build artifacts for this run.
  --jobs N        Maximum parallel compiler jobs (default: 0, no limit).
  --fqbn FQBN     Board identifier (default: arduino:renesas_uno:unor4wifi).
  -h, --help      Show this help.

Environment variables:
  FQBN             Default board identifier.
  COMPILE_JOBS     Default value for --jobs.
  C_WARNING_SUPPRESSIONS
                   Extra warning options for C sources in ArduinoCore-renesas.
                   Set to an empty value to show all core warnings.

Examples:
  scripts/compile-examples.sh --install-deps
  scripts/compile-examples.sh 09_ForwardSensorWithDHT11
  scripts/compile-examples.sh --clean 03_CreateAnalogSensor 06_GateSensor
USAGE
}

filter_known_core_warnings()
{
    # arm-none-eabi-gcc 7-2017q4 emits this TinyUSB diagnostic without an
    # associated -W option, so -Wno-return-type cannot silence it selectively.
    # Filter only the exact known dcd_rusb2.c warning block. If the diagnostic
    # changes, it remains visible rather than being hidden accidentally.
    awk '
        /dcd_rusb2\.c: In function .*wait_pipe_fifo_empty.*:$/ {
            pending_context = $0
            next
        }

        pending_context != "" {
            if ($0 ~ /dcd_rusb2\.c:[0-9]+:[0-9]+: warning: .*return.* with a value, in function returning void$/) {
                pending_context = ""
                skip_known_lines = 5
                next
            }

            print pending_context
            pending_context = ""
        }

        skip_known_lines > 0 {
            --skip_known_lines
            next
        }

        { print }

        END {
            if (pending_context != "") {
                print pending_context
            }
        }
    '
}

install_dependencies()
{
    arduino-cli lib install \
        Stepper \
        Servo \
        "DHT sensor library" \
        "Adafruit Unified Sensor" \
        BH1750 \
        "Adafruit BME280 Library" \
        "Adafruit BMP280 Library" \
        MPU6050
}

compile_sketch()
{
    local sketch="$1"
    local sketch_name
    local -a compile_args

    sketch_name="$(basename "$sketch")"

    if [[ ! -d "$sketch" ]]; then
        echo "Example directory not found: $sketch" >&2
        return 1
    fi

    if [[ ! -f "$sketch/$sketch_name.ino" ]]; then
        echo "Missing primary sketch: $sketch/$sketch_name.ino" >&2
        return 1
    fi

    compile_args=(
        --fqbn "$FQBN"
        --library "$REPO_ROOT"
        --warnings default
        --jobs "$COMPILE_JOBS"
    )

    # These options apply only to C files. The TinyUSB return-value warning is
    # filtered separately below because this GCC version exposes no warning
    # option that disables only that diagnostic.
    if [[ "$FQBN" == arduino:renesas_uno:* && -n "$C_WARNING_SUPPRESSIONS" ]]; then
        compile_args+=(
            --build-property
            "compiler.c.extra_flags=$C_WARNING_SUPPRESSIONS"
        )
    fi

    if [[ "$CLEAN_BUILD" == true ]]; then
        compile_args+=(--clean)
    fi

    echo
    echo "Compiling ${sketch#$REPO_ROOT/}"

    arduino-cli compile "${compile_args[@]}" "$sketch" 2>&1 |
        filter_known_core_warnings
}

examples=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --install-deps)
            INSTALL_DEPS=true
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --jobs)
            if [[ $# -lt 2 ]]; then
                echo "Missing value after --jobs" >&2
                exit 2
            fi
            COMPILE_JOBS="$2"
            shift 2
            ;;
        --fqbn)
            if [[ $# -lt 2 ]]; then
                echo "Missing value after --fqbn" >&2
                exit 2
            fi
            FQBN="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            while [[ $# -gt 0 ]]; do
                examples+=("$1")
                shift
            done
            ;;
        -* )
            echo "Unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
        *)
            examples+=("$1")
            shift
            ;;
    esac
done

if ! command -v arduino-cli >/dev/null 2>&1; then
    echo "arduino-cli was not found in PATH." >&2
    exit 127
fi

cd "$REPO_ROOT"

if [[ "$INSTALL_DEPS" == true ]]; then
    install_dependencies
fi

if [[ ${#examples[@]} -eq 0 ]]; then
    for sketch in "$REPO_ROOT"/examples/*; do
        [[ -d "$sketch" ]] || continue
        compile_sketch "$sketch"
    done
else
    for sketch in "${examples[@]}"; do
        if [[ "$sketch" != */* ]]; then
            sketch="$REPO_ROOT/examples/$sketch"
        elif [[ "$sketch" != /* ]]; then
            sketch="$REPO_ROOT/$sketch"
        fi

        compile_sketch "$sketch"
    done
fi
