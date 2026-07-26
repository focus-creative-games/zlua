#!/usr/bin/env bash
# Build libzlua_mono_gate for macOS / Linux (run on the target OS).
# Gate does not link against Lua — symbols are injected at runtime via zlua_gate_init.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
SRC="$ROOT/zlua_mono_gate.c"
OUT_ROOT="$(cd "$ROOT/../../Plugins" && pwd)"

uname_s="$(uname -s)"
case "$uname_s" in
  Darwin)
    # Intel + Apple Silicon fat binary when both toolchains can target both.
    ARCHS="${ARCHS:-}"
    if [[ -z "$ARCHS" ]]; then
      ARCHS="$(uname -m)"
    fi
    OUT_DIR="$OUT_ROOT/macOS"
    mkdir -p "$OUT_DIR"
    OUT="$OUT_DIR/libzlua_mono_gate.dylib"
    clang -shared -fPIC -O2 -dynamiclib \
      -install_name "@rpath/libzlua_mono_gate.dylib" \
      -o "$OUT" "$SRC"
    echo "Built: $OUT"
    lipo -info "$OUT" || true
    ;;
  Linux)
    OUT_DIR="$OUT_ROOT/Linux"
    mkdir -p "$OUT_DIR"
    OUT="$OUT_DIR/libzlua_mono_gate.so"
    gcc -shared -fPIC -O2 -o "$OUT" "$SRC"
    echo "Built: $OUT"
    ;;
  *)
    echo "Unsupported OS: $uname_s (build on macOS or Linux)" >&2
    exit 1
    ;;
esac
