#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

pattern='(afac|\bR[0-9]{2,}\b|gatehi|delta(world|phase)|cokernel)'
mapfile -t public_files < <(
    find include src tests examples negative_compile -type f \
        ! -path 'src/internal/frozen_types.hpp' -print | sort
)

if rg -n -i "$pattern" "${public_files[@]}" CMakeLists.txt build_and_test.sh; then
    echo "PUBLIC_INTERNAL_CODENAME_FOUND" >&2
    exit 1
fi

if find docs -type f -printf '%f\n' | rg -i "$pattern"; then
    echo "DOCUMENT_FILENAME_INTERNAL_CODENAME_FOUND" >&2
    exit 1
fi

if find tests examples negative_compile -type f -printf '%f\n' | rg -i "$pattern"; then
    echo "CODE_FILENAME_INTERNAL_CODENAME_FOUND" >&2
    exit 1
fi

echo "PUBLIC_INTERNAL_CODENAMES=0"
echo "DOCUMENT_FILENAME_INTERNAL_CODENAMES=0"
echo "PRIVATE_FROZEN_ADAPTER_EXCLUDED=YES"
