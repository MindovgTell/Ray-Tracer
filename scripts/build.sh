#!/bin/bash

set -euo pipefail

# -------- Paths --------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$PROJECT_ROOT"
BUILD_DIR="$PROJECT_ROOT/build"
BIN_DIR="$PROJECT_ROOT/bin"

# -------- Config --------
TARGET_NAME="${TARGET_NAME:-RayTracer}"      # change if your CMake target is named differently
BUILD_TYPE="${BUILD_TYPE:-Release}"    # Debug/Release/RelWithDebInfo/MinSizeRel

# Portable core count (works on macOS/Linux)
JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu)"

mkdir -p "$BUILD_DIR" "$BIN_DIR"

echo "Configuring $TARGET_NAME in $BUILD_DIR …"
cmake -S "$SRC_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "$@"

echo "Building $TARGET_NAME …"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -- -j"$JOBS"

echo "Saving built files to $BIN_DIR …"
# Common CMake output locations to check
CANDIDATES=(
  "$BUILD_DIR/$TARGET_NAME"
  "$BUILD_DIR/$TARGET_NAME.exe"
  "$BUILD_DIR/$BUILD_TYPE/$TARGET_NAME"
  "$BUILD_DIR/$BUILD_TYPE/$TARGET_NAME.exe"
)

COPIED=0
for f in "${CANDIDATES[@]}"; do
  if [[ -f "$f" && -x "$f" ]]; then
    cp -f "$f" "$BIN_DIR/"
    echo "  -> $(basename "$f")"
    COPIED=1
  fi
done

if [[ $COPIED -eq 0 ]]; then
  echo "Warning: built executable not found in expected locations."
  echo "Searched:"
  printf '  - %s\n' "${CANDIDATES[@]}"
  echo "Tip: set TARGET_NAME env var if your target isn't 'app', e.g.:"
  echo "     TARGET_NAME=raytracer ./scripts/build.sh"
fi

echo "Configuration is Done! Binaries (if found) are in: $BIN_DIR"
