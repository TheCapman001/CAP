# CAP Language Reference — v1.0

CAP is designed to read like a small, practical programming language.

## 1. Comments
```cap
# this is a comment
```

## 2. Variables
Declare with `the`:
```cap
the name = "CAP"
the score = 100
the passed = score >= 50
```
Existing variables can be changed:
```cap
score = 120
```
An assignment to an undeclared variable raises a CAP ALERT.

## 3. Values
CAP v1 supports:
- numbers: `10`, `3.14`
- strings: `"hello"`
- booleans: `true`, `false`
- `nothing`

## 4. Operators
Arithmetic: `+ - * /`

Comparison: `> < >= <= == !=`

Logic: `AND OR NOT`

`+` can add numbers or concatenate when a string participates.

## 5. Output
```cap
say "Salute!"
say 2 + 3 * 4
say name
```

## 6. Input
```cap
ask "What is your name? "
```
Input is read from standard input.

## 7. Conditions
```cap
if score >= 50
    say "Passed"
ifnot
    say "Try again"
end
```

## 8. Repetition
Fixed repetition:
```cap
repeat 5
    say "CAP"
end
```

Conditional repetition:
```cap
while score < 100
    score = score + 10
end
```
The runtime places a safety ceiling on `while` iterations.

## 9. Functions
```cap
function add(a, b)
    give a + b
end

the result = add(10, 20)
say result
```

A function can use `give` to return a value. Calling a function without `give` returns `nothing`.

## 10. Module placeholder
`load "name"` is recognized by the language, but module loading is intentionally reserved for the next runtime layer. The current build reports a CAP ALERT instead of silently pretending to load a module.

## 11. Errors
CAP uses diagnostics in the form:
```text
CAP ALERT [E205]
Undefined variable: score
```
The program exits non-zero after a runtime/parse error.

## 12. Source-file convention
CAP programs use the `.cap` extension and should contain CAP syntax. The native runtime is written in C, but CAP users write CAP source files.

## Precedence
From strongest to weakest:
1. unary `-`, `NOT`
2. `*`, `/`
3. `+`, `-`
4. comparisons
5. `AND`
6. `OR`
