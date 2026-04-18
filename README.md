# pydotc: A basic C-based Python interpreter

## Project Architecture:
1. Lexer: takes raw source code and converts it into tokens for the parser. Also takes care of indentation.
2. Recursive Descent Parser: Enforces expression parsing using a top-down execution model and finally creates an Abstract Syntax Tree for the program.
3. Tree-Walking Evaluator: Travels down the AST and executes the operations.

## Supported Features:
1. Mathematical Operations (`+`, `-`, `*`, `/`, `//`, `**`, `%`)
2. Comparison Operations (`<`, `>`, `<=`, `>=`, `==`, `!=`)
3. `print()`
4. Control Flow: (`if`, `elif`, `else`, `while`)

## Installation & Compilation
To compile the `pydotc` executable natively, clone the repository and run `make`:
```bash
git clone https://github.com/AayushRanjan10/pydotc
cd pydotc
make
```
Once compiled, you can pass any valid .py script to the interpreter via the Command Line. Test cases have been provided in the `testcases/input/` directory.

**Example Execution:**
```bash
./pydotc testcases/input/test_tradition.py
```
**Expected CLI Output:**
```text
Hello, World!
```

## Differences from CPython
1. Tree-walking vs Bytecode: CPython compiles the AST into low-level Bytecode which is executed by a Virtual Machine. On the other hand, pydotc has a Tree Walking Evaluator to execute the AST.
2. Static Scope: pydotc utilizes a single, static global memory array to store its runtime variables, restricting scope to the global level.
3. Static Typing: Variables cannot store strings, only store doubles.
4. Multi-Level Escaping: The parser cannot handle nested loops where blocks escape multiple levels of indentation simultaneously (as the lexer only emits 1 DEDENT token per line).
