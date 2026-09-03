#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
make clean >/dev/null 2>&1 || true
make >/dev/null

check_output() {
    file="$1"
    expected="$2"
    actual="$(./cap "$file" | tr -d '\r')"
    if [ "$actual" != "$expected" ]; then
        echo "FAIL: $file"
        echo "Expected:"
        printf '%s\n' "$expected"
        echo "Actual:"
        printf '%s\n' "$actual"
        exit 1
    fi
}

check_output tests/basic.cap "30
14"
check_output examples/salute.cap "Salute Cap Man!"
check_output tests/functions.cap "15"
check_output tests/logic.cap "true
true
false
true"
check_output tests/loops.cap "loop
loop
loop"
check_output examples/hello.cap "salute cap
Cap
Passed
CAP is coming
CAP is coming
CAP is coming
CAP is coming
CAP is coming"

if ./cap tests/errors/undefined.cap >/tmp/cap-test.err 2>&1; then
    echo "FAIL: undefined-variable test unexpectedly succeeded"; exit 1
fi
grep -q "E205" /tmp/cap-test.err

if ./cap tests/errors/divide_zero.cap >/tmp/cap-test.err 2>&1; then
    echo "FAIL: divide-by-zero test unexpectedly succeeded"; exit 1
fi
grep -q "E202" /tmp/cap-test.err

./cap --version | grep -q "CAP 1.0.0"
./cap --help | grep -q "Usage: cap"

echo "CAP tests: PASS"
