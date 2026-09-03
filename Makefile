CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra
LDFLAGS ?= -lm

all: cap

cap: src/main.c
	$(CC) $(CFLAGS) src/main.c -o cap $(LDFLAGS)

test: cap
	sh tests/run.sh

clean:
	rm -f cap

install: cap
	@BIN_DIR="$${PREFIX:-$$HOME/.local}/bin"; mkdir -p "$$BIN_DIR"; cp cap "$$BIN_DIR/cap"; chmod 755 "$$BIN_DIR/cap"; echo "Installed CAP to $$BIN_DIR/cap"

.PHONY: all test clean install
