# Project Structure

```text
CAP-final/
├── src/
│   └── main.c
├── stdlib/
│   └── README.md
├── examples/
│   ├── hello.cap
│   └── salute.cap
├── tests/
│   ├── basic.cap
│   ├── functions.cap
│   ├── logic.cap
│   ├── loops.cap
│   ├── while.cap
│   ├── errors/
│   │   ├── undefined.cap
│   │   └── divide_zero.cap
│   └── run.sh
├── docs/
│   ├── ARCHITECTURE.md
│   ├── LANGUAGE.md
│   └── PROJECT_STRUCTURE.md
├── Makefile
├── install.sh
├── README.md
├── CHANGELOG.md
├── LICENSE
└── VERSION
```

`src/main.c` is the native implementation of the v1 runtime. The directories are separated around the language concepts so the project can be split into dedicated source modules in a future major runtime revision without changing CAP source syntax.
