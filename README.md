# torino

A small Lisp interpreter written in C, in the spirit of
[Build Your Own Lisp](https://buildyourownlisp.com/).

```lisp
; square.to
(fun {square x} {* x x})
(print (square 7))                           ; 49
(print (if (> 3 2) {square 3} {square 2}))   ; 9
(def {nums} {1 2 3})
(print (head nums) (tail nums))              ; {1} {2 3}
```

## Language

- **Values**: numbers (integers and decimals), strings, booleans (`true`, `false`), symbols
- **S-expressions** `( … )` are evaluated, **Q-expressions** `{ … }` are quoted lists
- **Comments** start with `;`
- **Arithmetic**: `+ - * /` (integers and decimals can be mixed)
- **Comparison**: `< <= > >= == !=`
- **Lists**: `list`, `head`, `tail`, `join`, `eval`
- **Variables**: `def` (global) and `=` (local)
- **Functions**: lambdas with `\`, and `fun` to define named functions
- **Control flow**: `if`
- **Programs**: `load` a file, `print`, `error`

## Build and run

Needs a C compiler and libedit (`sudo apt install libedit-dev` on Debian/Ubuntu).

```sh
make all
./torino            # interactive prompt
./torino test.to    # run a file
```

## License

MIT, see [LICENSE](LICENSE). The parser uses [mpc](https://github.com/orangeduck/mpc)
by Daniel Holden (`mpc.c`, `mpc.h`), which is under the BSD license in
[LICENSE.mpc.md](LICENSE.mpc.md).
