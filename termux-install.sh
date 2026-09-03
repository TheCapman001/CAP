#!/data/data/com.termux/files/usr/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PREFIX="${PREFIX:-/data/data/com.termux/files/usr}"

printf '%s\n' 'CAP Android / Termux installer'
printf '%s\n' 'Checking Termux compiler...'
if ! command -v clang >/dev/null 2>&1; then
  printf '%s\n' 'clang is not installed. Install it with: pkg install clang make'
  exit 1
fi
if ! command -v make >/dev/null 2>&1; then
  printf '%s\n' 'make is not installed. Install it with: pkg install make clang'
  exit 1
fi

cd "$ROOT"
make clean >/dev/null 2>&1 || true
make CC=clang
mkdir -p "$PREFIX/bin"
cp cap "$PREFIX/bin/cap"
chmod 755 "$PREFIX/bin/cap"

printf '\nCAP installed successfully.\n'
printf 'Binary: %s/bin/cap\n' "$PREFIX"
printf '\nTry:\n  cap --version\n  cap --help\n  cap examples/hello.cap\n'
