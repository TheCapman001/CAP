#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
command -v cc >/dev/null 2>&1 || { echo "CAP ALERT [E901] C compiler not found." >&2; exit 1; }
cd "$ROOT"
make clean >/dev/null 2>&1 || true
make
BIN_DIR="${PREFIX:-$HOME/.local}/bin"
mkdir -p "$BIN_DIR"
cp cap "$BIN_DIR/cap"
chmod 755 "$BIN_DIR/cap"
printf '%s\n' "CAP installed to $BIN_DIR/cap"
printf '%s\n' "If 'cap' is not found, add $BIN_DIR to PATH."
