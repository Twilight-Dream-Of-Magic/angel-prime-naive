#!/usr/bin/env bash
set -euo pipefail

SDK_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$SDK_ROOT"

sha256sum -c manifest/legacy_public_headers.sha256
sha256sum -c manifest/sdk_1_1_public_headers.sha256
echo "LEGACY_PUBLIC_HEADERS_BYTE_PRESERVED=YES"
echo "SUPPLIED_SDK_PUBLIC_HEADERS_BYTE_PRESERVED=YES"
echo "ADDITIVE_PUBLIC_API_ONLY=YES"
