# CAP on Android with Termux

This package is designed to build and run CAP directly inside Termux on Android.

## 1. Install Termux dependencies

Use a current Termux installation, then run:

```sh
pkg update
pkg install clang make
```

## 2. Extract CAP

Put the CAP ZIP in a folder you can access from Termux. For example, if it is in Downloads:

```sh
termux-setup-storage
cd ~/storage/downloads
unzip CAP-Android-Termux-v1.0.0.zip
cd CAP-Android-Termux-v1.0.0/CAP-final
```

If `unzip` is missing:

```sh
pkg install unzip
```

## 3. Install CAP

```sh
./termux-install.sh
```

Or build manually:

```sh
make CC=clang
cp cap $PREFIX/bin/cap
```

## 4. Test

```sh
cap --version
cap examples/hello.cap
./tests/run.sh
```

## 5. Run your own CAP program

```sh
cap myprogram.cap
```

## Android notes

- CAP itself is a native C11 command-line program, so Termux compiles it for the Android device architecture.
- This package does **not** require root access.
- The recommended distribution format for Termux is the source ZIP plus the Termux installer, rather than shipping one CPU-specific binary.
- On a different Android CPU architecture, Termux's `clang` produces the correct native executable for that environment.
