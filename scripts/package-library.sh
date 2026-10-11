#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

if command -v python3 >/dev/null 2>&1; then
    PYTHON=python3
elif command -v python >/dev/null 2>&1; then
    PYTHON=python
else
    echo "python3/python was not found in PATH." >&2
    exit 127
fi

cd "$REPO_ROOT"

"$PYTHON" - "$REPO_ROOT" <<'PY'
from __future__ import annotations

import hashlib
from pathlib import Path
import sys
import zipfile


repo_root = Path(sys.argv[1]).resolve()
properties_path = repo_root / "library.properties"

if not properties_path.is_file():
    raise SystemExit("library.properties was not found.")


def read_properties(path: Path) -> dict[str, str]:
    properties: dict[str, str] = {}
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        properties[key.strip()] = value.strip()
    return properties


properties = read_properties(properties_path)
library_name = properties.get("name", "")
version = properties.get("version", "")

if not library_name:
    raise SystemExit("library.properties does not define name.")
if not version:
    raise SystemExit("library.properties does not define version.")
if library_name != "AmperiaSTEM":
    raise SystemExit(
        f"Unexpected library name {library_name!r}; expected 'AmperiaSTEM'."
    )

required_paths = [
    Path("library.properties"),
    Path("src/AmperiaSTEM.h"),
    Path("examples"),
]

for relative in required_paths:
    if not (repo_root / relative).exists():
        raise SystemExit(f"Required package path is missing: {relative}")

top_level_files = [
    "library.properties",
    "README.md",
    "README.en.md",
    "LICENSE",
    "LICENSE.txt",
    "LICENSE.md",
    "keywords.txt",
]

included_directories = [
    Path("src"),
    Path("examples"),
    Path("extras/reference"),
    Path("photos"),
]

files: list[Path] = []

for name in top_level_files:
    path = repo_root / name
    if path.is_file():
        files.append(path)

for relative_directory in included_directories:
    directory = repo_root / relative_directory
    if not directory.exists():
        continue
    for path in directory.rglob("*"):
        if path.is_symlink():
            raise SystemExit(
                f"Symlinks are not allowed in the release package: "
                f"{path.relative_to(repo_root)}"
            )
        if path.is_file():
            files.append(path)

files = sorted(set(files), key=lambda path: path.relative_to(repo_root).as_posix())

dist_dir = repo_root / "dist"
dist_dir.mkdir(exist_ok=True)
archive_path = dist_dir / f"{library_name}-{version}.zip"

# Use fixed timestamps and permissions so packaging the same source tree
# produces byte-for-byte identical archives.
fixed_timestamp = (1980, 1, 1, 0, 0, 0)

with zipfile.ZipFile(
    archive_path,
    mode="w",
    compression=zipfile.ZIP_DEFLATED,
    compresslevel=9,
) as archive:
    for path in files:
        relative = path.relative_to(repo_root).as_posix()
        archive_name = f"{library_name}/{relative}"

        info = zipfile.ZipInfo(archive_name, date_time=fixed_timestamp)
        info.compress_type = zipfile.ZIP_DEFLATED
        info.external_attr = (0o100644 & 0xFFFF) << 16

        archive.writestr(info, path.read_bytes())

required_archive_entries = {
    f"{library_name}/library.properties",
    f"{library_name}/src/AmperiaSTEM.h",
}

forbidden_prefixes = (
    f"{library_name}/.git/",
    f"{library_name}/.github/",
    f"{library_name}/scripts/",
    f"{library_name}/stuff/",
    f"{library_name}/extras/manual-tests/",
)

with zipfile.ZipFile(archive_path, mode="r") as archive:
    entries = set(archive.namelist())

    missing = required_archive_entries - entries
    if missing:
        raise SystemExit(
            "Package verification failed; missing: "
            + ", ".join(sorted(missing))
        )

    forbidden = sorted(
        entry
        for entry in entries
        if entry.startswith(forbidden_prefixes)
    )
    if forbidden:
        raise SystemExit(
            "Package verification failed; forbidden entries: "
            + ", ".join(forbidden)
        )

digest = hashlib.sha256(archive_path.read_bytes()).hexdigest()
relative_archive = archive_path.relative_to(repo_root).as_posix()

print(
    f"PACKAGE_LIBRARY_PASS|NAME={library_name}|VERSION={version}|"
    f"FILES={len(files)}"
)
print(f"Created: {relative_archive}")
print(f"SHA256: {digest}")
PY
