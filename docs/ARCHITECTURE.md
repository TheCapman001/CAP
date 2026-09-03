# CAP Architecture

The runtime follows this pipeline:

```text
CAP source
   ↓
Lexer
   ↓
Tokens
   ↓
Parser / expression evaluator
   ↓
Runtime execution
   ↓
Values + variables + control flow
   ↓
Output / CAP ALERT
```

## Components

### Lexer
Turns characters into tokens: words, numbers, strings, operators, assignment, parentheses, commas, and line boundaries. It also removes comments.

### Parser / evaluator
Consumes the token stream using precedence-aware expression parsing. It resolves literals, variables, function calls, unary operators, arithmetic, comparisons, and boolean logic.

### Runtime
Executes statements such as declarations, assignment, `say`, `ask`, `if`, `ifnot`, `repeat`, `while`, `function`, `give`, and `load`.

### Value system
The v1 value model contains `nothing`, number, string, and boolean values. Strings are dynamically allocated and copied when stored.

### Diagnostics
Errors use stable CAP ALERT codes so future tooling can recognize failures.

## Why the current build is intentionally staged

CAP is being built as a real language rather than a collection of unrelated scripts. The v1 runtime is self-contained and has no Python runtime dependency. Larger systems—collections, a real module/package loader, richer standard library, networking, concurrency, native extensions, and a package manager—are separate expansion layers so they can be added without changing the basic language model.

## Security boundary

The runtime does not silently execute arbitrary shell commands, scrape credentials, or provide offensive security automation. Future security-oriented APIs should be designed as controlled learning interfaces with clear permissions and safe defaults.
