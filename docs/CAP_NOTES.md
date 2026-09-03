# CAP Notes — Complete Core Build

## 1. Project goal

CAP is a real programming-language project. A CAP user writes `.cap` source. The C program is the native runtime that reads and executes that source.

## 2. Design decisions

- **CAP syntax first:** examples are written in CAP, not Python.
- **Native runtime:** C11 keeps the runtime small and portable.
- **Line-oriented blocks:** `if ... end`, `repeat ... end`, `while ... end`, and `function ... end`.
- **Readable declarations:** `the name = value`.
- **Explicit reassignment:** `name = value` only works for an existing variable.
- **CAP ALERT:** errors use stable diagnostic codes.
- **Staged expansion:** larger subsystems are added as real layers instead of fake placeholder commands.

## 3. C implementation

`src/main.c` contains the v1 native implementation.

The important parts are:

- `lex()` — converts source characters into tokens.
- `Value` — represents runtime values.
- `parse_primary()` — literals, variables, and calls.
- `parse_unary()` / `parse_mul()` / `parse_add()` / `parse_cmp()` / `parse_and()` / `parse_expr()` — operator precedence.
- `execute_range()` — executes CAP statements.
- `register_functions()` — records function definitions before execution.
- `set_var()` / `find_var()` — variable storage.
- `alert()` — CAP diagnostic reporting.
- `main()` — CLI entry point, file loading, lexing, registration, execution, and cleanup.

## 4. Build flow

```text
CAP .cap file
   ↓
read_file()
   ↓
lex()
   ↓
Token[]
   ↓
register_functions()
   ↓
execute_range()
   ↓
Value / variables / control flow
   ↓
stdout or CAP ALERT
```

## 5. What is complete in v1

The packaged core has working implementations for:

- source-file loading
- tokenization
- comments
- strings and escape sequences
- numbers
- booleans
- `nothing`
- variables
- assignment
- arithmetic
- comparisons
- boolean logic
- conditions
- fixed loops
- conditional loops
- functions and parameters
- return values with `give`
- output
- input
- CLI help/version
- automated success and failure tests
- installation

## 6. Next architectural layers

These are intentionally not claimed as finished in v1:

- lists/maps and richer collections
- a real module loader
- package manager
- larger standard library
- structured data formats
- time/date APIs
- permission-aware networking
- controlled process/system APIs
- concurrency
- native extension interface
- IDE/editor tooling

Keeping these outside the v1 core makes the current build honest, testable, and easier to extend.
