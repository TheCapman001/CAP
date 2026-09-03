# CAP 1.0.0

CAP is a standalone programming language runtime written in C11. CAP programs use `.cap` source files; the native executable is `cap`.

## What is included

- Lexer/tokenizer
- Expression parser with operator precedence
- Runtime/evaluator
- Variables and assignment
- Numbers, strings, booleans, `nothing`
- Arithmetic, comparison, and boolean operators
- `if`, `ifnot`, `end`
- `repeat`, `while`
- Functions, parameters, `give`, calls
- `say` output and `ask` input
- Comments with `#`
- CAP ALERT diagnostic errors
- CLI `--help` and `--version`
- Examples and automated tests
- Installation script
- Language and architecture documentation

## Requirements

A C11-compatible compiler (`cc`, `clang`, or `gcc`) and a POSIX-like shell environment. Termux on Android is supported. See `TERMUX.md` for the Android setup.

## Android / Termux

For Android, install Termux packages with `pkg install clang make unzip`, extract this package, and run `./termux-install.sh`. See [`TERMUX.md`](TERMUX.md) for the complete Termux procedure.

## Build

```sh
make
```

Run:

```sh
./cap examples/hello.cap
./cap examples/salute.cap
```

Help/version:

```sh
./cap --help
./cap --version
```

## Test everything

```sh
./tests/run.sh
```

A successful run ends with:

```text
CAP tests: PASS
```

## Install

```sh
chmod +x install.sh
./install.sh
```

The installer uses `$PREFIX/bin` when `PREFIX` is set (such as Termux), otherwise `$HOME/.local/bin`.

## Learn CAP

Start with [`docs/LANGUAGE.md`](docs/LANGUAGE.md), then read [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## Development status

This is the complete packaged CAP 1.0 core build. It is not a claim that every future CAP ecosystem feature already exists. The documented expansion layers are collections, a real module/package system, richer standard library APIs, file/network/process/system APIs, concurrency, native extensions, and package tooling.

The goal is to keep each layer real and testable instead of filling the project with placeholder code that only looks complete.
