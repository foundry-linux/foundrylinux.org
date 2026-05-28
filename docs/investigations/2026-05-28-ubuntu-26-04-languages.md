# Languages available on Ubuntu/Kubuntu 26.04 LTS (Resolute Raccoon)

**Date:** 2026-05-28  
**Scope:** Any Turing-complete language (or plausible candidate) available via `apt` on a stock
Ubuntu/Kubuntu 26.04 install. "Available" means in `main`, `universe`, or `multiverse` — one
`apt install` away, no PPA, no snap, no manual download.

"Language" is interpreted very liberally: anything for which a Turing-completeness proof exists
or is plausible, including build tools, stream processors, document compilers, and calculators.

Ubuntu 26.04 base references: [releases.ubuntu.com/resolute](https://releases.ubuntu.com/resolute/) ·
[packages.ubuntu.com/resolute](https://packages.ubuntu.com/resolute/) ·
[toolchain blog post](https://ubuntu.com/blog/from-jammy-to-resolute-how-ubuntus-toolchains-have-evolved)

---

## Pre-installed (zero setup, Kubuntu default)

These are present before any `apt install`. All are Turing-complete.

| Language | Package | Notes |
|---|---|---|
| [Python](https://python.org) 3.14 | `python3` | System Python; used by Ubuntu internals |
| [Perl](https://perl.org) 5.x | `perl` | Required by `dpkg`; always present |
| [Bash](https://gnu.org/software/bash/) 5.x | `bash` | Default interactive shell |
| [Dash](https://gondor.apana.org.au/~herbert/dash/) | `dash` | `/bin/sh`; POSIX-compliant |
| [AWK](https://www.gnu.org/software/gawk/) | `mawk` | Pre-installed mawk; gawk available separately |
| [sed](https://www.gnu.org/software/sed/) | `sed` | GNU sed — Turing-complete (labels + branches) |
| [m4](https://www.gnu.org/software/m4/) | `m4` | Macro processor; required by autoconf |

---

## Compiled / systems languages

Installed via a single `apt install`. GCC 15.2, Clang 17–22, and LLVM 21 are the compiler
backbone. Cross-compilation targets available for ARM, RISC-V, MIPS, etc.

| Language | Package(s) | Version / notes |
|---|---|---|
| [C](https://gcc.gnu.org) | `gcc` | GCC 15.2 |
| [C++](https://gcc.gnu.org) | `g++` | GCC 15.2 |
| [C / C++](https://clang.llvm.org) | `clang` (17–22) | Multiple LLVM versions co-installable |
| [Fortran](https://gcc.gnu.org/fortran/) | `gfortran` | GCC Fortran; also `flang-17` – `flang-22` (LLVM) |
| [Fortran 77](https://netlib.org/f2c/) | `f2c`, `fort77` | FORTRAN 77 → C translator |
| [Ada](https://gcc.gnu.org/onlinedocs/gnat/) | `gnat` | GNAT via GCC |
| [D](https://dlang.org) | `gdc`, `ldc2` | GCC-D and LLVM-D |
| [Go](https://go.dev) | `golang` | 1.26 (default in 26.04) |
| [Rust](https://rust-lang.org) | `rustc`, `cargo` | 1.93 |
| [Zig](https://ziglang.org) | `zig` | 0.14.1 — new in 26.04 universe |
| [Nim](https://nim-lang.org) | `nim` | 2.2.4 |
| [Crystal](https://crystal-lang.org) | `crystal` | In `devel` section |
| [Pascal / Free Pascal](https://freepascal.org) | `fp-compiler`, `fpc` | Free Pascal Compiler |
| [COBOL](https://gnucobol.sourceforge.io) | `gnucobol` | GnuCOBOL; transpiles to C |
| [Assembly (x86)](https://nasm.us) | `nasm` | NASM; also `yasm` |
| [Assembly (GNU)](https://sourceware.org/binutils/) | `binutils` | `as` (GAS); cross assemblers available |

---

## JVM & managed runtimes

| Language | Package(s) | Version / notes |
|---|---|---|
| [Java](https://openjdk.org) | `openjdk-25-jdk` | OpenJDK 25 LTS with CRaC; also OpenJDK 21 |
| [Kotlin](https://kotlinlang.org) | `kotlin` | 2.0.4 |
| [Scala](https://scala-lang.org) | `scala` | |
| [Groovy](https://groovy-lang.org) | `groovy` | |
| [Clojure](https://clojure.org) | `clojure` | Lisp on JVM |
| [C# / F# / .NET](https://dotnet.microsoft.com) | `dotnet-sdk-10.0` | .NET 10 LTS; also `dotnet-sdk-8.0` |
| [Mono C#](https://mono-project.com) | `mono-complete` | Dedicated `mono` section in universe |
| [JavaScript (Rhino)](https://mozilla.github.io/rhino/) | `rhino` | JS in Java (JVM-hosted) |

---

## Scripting / interpreted languages

| Language | Package(s) | Version / notes |
|---|---|---|
| [Python](https://python.org) | `python3`, `pypy3` | 3.14; also PyPy |
| [Ruby](https://ruby-lang.org) | `ruby` | Full section in universe |
| [PHP](https://php.net) | `php` | Full section in universe |
| [Perl](https://perl.org) | `perl` | Full section; also CPAN modules |
| [Raku / Perl 6](https://raku.org) | `raku`, `rakudo`, `moarvm`, `nqp` | Rakudo on MoarVM |
| [JavaScript (Node.js)](https://nodejs.org) | `nodejs` | |
| [JavaScript (GJS)](https://gitlab.gnome.org/GNOME/gjs) | `gjs` | GNOME JavaScript; SpiderMonkey |
| [JavaScript (Duktape)](https://duktape.org) | `duktape` | Embeddable JS engine |
| [Lua](https://lua.org) | `lua5.1` – `lua5.5`, `luajit` | All maintained versions co-installable |
| [Tcl](https://tcl.tk) | `tcl8.6`, `tcl9.0`, `tcl` | Including Tk; 30+ extension packages |
| [PHP](https://php.net) | `php` | |
| [Pike](https://pike.lysator.liu.se) | `pike8.0` | Dynamic language from Linköping |
| [Neko](https://nekovm.org) | `neko` | VM used by Haxe |
| [Squirrel](http://squirrel-lang.org) | `squirrel3` | Lightweight; used in games |
| [Tengo](https://tengolang.com) | `tengo` | Go-like scripting |
| [Nickle](https://nickle.org) | `nickle` | Desk calculator with full language |
| [REXX](https://regina-rexx.sourceforge.io) | `regina-rexx` | Classic IBM scripting language |
| [Expect](https://core.tcl.tk/expect/) | `expect` | Tcl-based automation |
| [Haserl](https://haserl.sourceforge.net) | `haserl` | Shell/Lua CGI scripting |
| [GNU Data Language](https://gnudatalanguage.github.io) | `gnudatalanguage` | IDL/GDL-compatible |
| [Afnix](https://www.afnix.org) | `afnix` | Multi-threaded multi-paradigm |

---

## Functional languages

| Language | Package(s) | Version / notes |
|---|---|---|
| [Haskell](https://haskell.org) | `ghc` | GHC 9.6.6; dedicated `haskell` section with 1000+ libraries |
| [OCaml](https://ocaml.org) | `ocaml` | Dedicated `ocaml` section |
| [Standard ML (Poly/ML)](https://polyml.org) | `polyml` | Full SML implementation |
| [Erlang](https://erlang.org) | `erlang` | OTP with 40+ sub-packages |
| [Elixir](https://elixir-lang.org) | `elixir` | Runs on BEAM |
| [Scheme — Chicken](https://call-cc.org) | `chicken-bin` | Compiles to C |
| [Scheme — GNU Guile](https://gnu.org/software/guile/) | `guile-3.0` | Extension language; Turing complete |
| [Scheme — MIT](https://groups.csail.mit.edu/mac/projects/scheme/) | `mit-scheme` | |
| [Scheme — scheme48](https://s48.org) | `scheme48` | |
| [Scheme — SCM](https://people.csail.mit.edu/jaffer/SCM.html) | `scm` | Aubrey Jaffer's implementation |
| [Scheme — tinyscheme](https://tinyscheme.sourceforge.net) | `tinyscheme` | Embeddable |
| [Scheme — SigScheme](https://sigscheme.sourceforge.net) | `sigscheme` | Used in uim |
| [Scheme — SISC](https://sisc-scheme.org) | `sisc` | JVM-hosted |
| [Common Lisp — CLISP](https://clisp.sourceforge.io) | `clisp` | |
| [Common Lisp — GCL](https://gnu.org/software/gcl/) | `gcl` | GNU Common Lisp |
| [Common Lisp — ECL](https://ecl.common-lisp.dev) | `ecl` | Embeds in C |
| [Scheme — Elk](https://sam.zoy.org/elk/) | `elk` | Extension Language Kit |
| [R](https://r-project.org) | `r-base` | Statistical; Turing complete; dedicated `gnu-r` section |
| [Julia](https://julialang.org) | `julia` | Scientific computing |
| [Factor](https://factorcode.org) | `factor` | Concatenative; stack-based |
| [Racket](https://racket-lang.org) | `racket` | Multi-paradigm Lisp/Scheme descendant |

---

## Logic, constraint & declarative languages

| Language | Package(s) | Notes |
|---|---|---|
| [SWI-Prolog](https://swi-prolog.org) | `swi-prolog` | 9.2.9; ISO/Edinburgh; many modules |
| [GNU Prolog](https://gprolog.org) | `gprolog` | Native-code compiler |
| [Answer Set Programming](https://potassco.org/clingo/) | `clasp`, `gringo` | Potassco / clingo ecosystem |
| [CLIPS](https://clipsrules.net) | `clips` | Rule-based expert system language |
| [Datalog / QBF](https://lonsing.github.io/depqbf/) | `depqbf` | QBF solver with scripting interface |

---

## Shells & command languages

Every shell below is Turing-complete by the POSIX branching + looping spec.

| Shell | Package | Notes |
|---|---|---|
| [Bash](https://gnu.org/software/bash/) | `bash` | Default; pre-installed |
| [Dash](https://gondor.apana.org.au/~herbert/dash/) | `dash` | `/bin/sh`; pre-installed |
| [Zsh](https://zsh.org) | `zsh` | Highly configurable |
| [Fish](https://fishshell.com) | `fish` | Friendly interactive |
| [mksh / MirBSD Korn Shell](https://mirbsd.org/mksh.htm) | `mksh` | |
| [ksh / KornShell](https://kornshell.com) | `ksh` | |
| [tcsh / C Shell](https://tcsh.org) | `tcsh` | |
| [Nushell](https://nushell.sh) | `nushell` | Structured data shell |
| [rc](https://doc.cat-v.org/plan_9/4th_edition/papers/rc) | `rc` | Plan 9 shell |
| [es](https://wryun.github.io/es-shell/) | `es` | Extensible rc-derived shell |

---

## Turing-complete non-traditional

The most interesting category: tools that are technically programming languages
but marketed as something else entirely. All available via `apt`.

| Language | Package | Why it's Turing-complete |
|---|---|---|
| [GNU Make](https://gnu.org/software/make/) | `make` | Recursive variable expansion + computed targets; proven TC |
| [GNU M4](https://gnu.org/software/m4/) | `m4` | Macro processor with recursion and conditionals |
| [GNU AWK](https://gnu.org/software/gawk/) | `gawk` | Full Turing-complete; arrays, functions, arbitrary I/O |
| [GNU sed](https://gnu.org/software/sed/) | `sed` | Labels + branches; proven TC (Rule 110 implementations exist) |
| [jq](https://jqlang.org) | `jq` | Recursive functions, reduce, `recurse`; TC proven |
| [bc](https://gnu.org/software/bc/) | `bc` | Arbitrary-precision calculator with functions, loops |
| [dc](https://gnu.org/software/bc/) | `dc` | RPN stack calculator; older than bc; TC via macros + branching |
| [SQLite / SQL](https://sqlite.org) | `sqlite3` | Recursive CTEs (SQL:1999 + Postgres/SQLite) — TC proven |
| [PostgreSQL PL/pgSQL](https://postgresql.org) | `postgresql` | Full procedural SQL language |
| [PostScript](https://ghostscript.com) | `ghostscript` | Turing-complete stack language; PDF is a subset |
| [TeX / LaTeX](https://tug.org) | `texlive` | Donald Knuth's macro system; proven TC (quines exist) |
| [XSLT 2.0](https://saxonica.com) | `libsaxonhe-java` | TC via recursion and higher-order templates |
| [Emacs Lisp](https://gnu.org/software/emacs/) | `emacs` | Full Lisp; the editor is the language |
| [Vim / Neovim script](https://neovim.io) | `neovim` | VimL has loops, functions, eval(); Lua in Neovim |
| [CMake](https://cmake.org) | `cmake` | Scripting layer with loops and recursion is TC |
| [Bazel / Starlark](https://bazel.build) | `bazel` | Starlark is a restricted Python; BUILD files are TC |
| [GNU Guile](https://gnu.org/software/guile/) | `guile-3.0` | Scheme; also the extension language for many GNU tools |
| [GDScript](https://godotengine.org) | `godot4` | Godot's Python-like scripting language |
| [Nix expression language](https://nixos.org) | `nix` | Lazy functional language; TC (despite being "config") |

---

## Mathematical / scientific computing

| System | Package | Notes |
|---|---|---|
| [GNU Octave](https://octave.org) | `octave` | MATLAB-compatible; full TC language |
| [Maxima](https://maxima.sourceforge.io) | `maxima` | CAS based on Macsyma; Lisp-backed; TC |
| [SageMath](https://sagemath.org) | `sagemath` | Python-based math system |
| [Scilab](https://scilab.org) | `scilab` | MATLAB-like; French INRIA |
| [YACAS](https://yacas.org) | `yacas` | Yet Another Computer Algebra System |
| [GNU Calc](https://gnu.org/software/emacs/manual/html_mono/calc.html) | `emacs` | Emacs calc; RPN + algebraic |
| [Genius](https://jirka.org/genius.html) | `genius` | GNOME calculator with a full scripting language (GEL) |

---

## Assembly & low-level / IR (Intermediate Representation)

| Language | Package | Notes |
|---|---|---|
| [NASM](https://nasm.us) | `nasm` | x86/x86-64 assembler |
| [YASM](https://yasm.tortall.net) | `yasm` | x86/x86-64; supports NASM + GAS syntax |
| [GAS (GNU Assembler)](https://sourceware.org/binutils/) | `binutils` | AT&T syntax; multi-arch |
| [LLVM IR](https://llvm.org) | `llvm` | `llc`, `lli` — compile or interpret LLVM bitcode |
| [WebAssembly (WAT)](https://webassembly.org) | `wabt` | `wat2wasm`, `wasm2wat`, `wasm-interp` |
| [BPF / eBPF](https://ebpf.io) | `linux-tools-*` | Bytecode language; runs in kernel; restricted TC |

---

## Hardware description languages

Simulation mode is Turing-complete (arbitrary computation); synthesis is not.

| Language | Package | Notes |
|---|---|---|
| [VHDL](https://ghdl.github.io/ghdl/) | `ghdl` | IEEE 1076 VHDL simulator |
| [Verilog](https://iverilog.icarus.com) | `iverilog` | Icarus Verilog simulator |
| [SystemVerilog](https://verilator.org) | `verilator` | Lint + simulation |

---

## Proof assistants & type theory

These are dependently-typed languages where programs are proofs. All TC.

| Language | Package | Version |
|---|---|---|
| [Coq](https://coq.inria.fr) | `coq` | 8.20.1 |
| [Agda](https://agda.readthedocs.io) | `agda` | 2.6.4.3 |
| [Isabelle](https://isabelle.in.tum.de) | `isabelle` | Proof assistant / ML descendant |
| [HOL](https://hol-theorem-prover.org) | `hol88` | Higher-Order Logic |

---

## Esoteric languages

| Language | Package | Notes |
|---|---|---|
| [INTERCAL](https://intercal.sourceforge.net) | `intercal` | The original joke language (1972); fully TC |
| [Brainfuck](https://esolangs.org/wiki/Brainfuck) | `bf` | Classic 8-instruction TC language |
| [Malbolge](https://esolangs.org/wiki/Malbolge) | `malbolge` | Intentionally the hardest language ever |
| [Befunge](https://esolangs.org/wiki/Befunge) | `cfunge` | 2D stack-based; funge-98 |
| [Unlambda](https://esolangs.org/wiki/Unlambda) | `unlambda` | Combinator-based; no variables |

---

## Notable omissions (snap / PPA / manual only)

Not in `apt` on a standard 26.04 install:

| Language | Why missing | Route |
|---|---|---|
| [Swift](https://swift.org) | Apple's language; no Debian packaging | `snap install swift` |
| [Dart](https://dart.dev) | Google PPA only | [dart.dev install](https://dart.dev/get-dart) |
| [V (vlang)](https://vlang.io) | Not packaged | Build from source |
| [Lean 4](https://lean-lang.org) | Not in Ubuntu repos | [elan installer](https://github.com/leanprover/elan) |
| [Standard ML of NJ](https://smlnj.org) | Dropped from Debian | Build from source (polyml is the ubuntu option) |
| [Pharo Smalltalk](https://pharo.org) | No apt package | Download from [pharo.org](https://pharo.org) |
| [MATLAB / Mathematica / Maple](https://mathworks.com) | Commercial | Vendor installer |
| [CUDA C](https://developer.nvidia.com/cuda-toolkit) | Driver-dependent | NVIDIA CUDA toolkit |

### Out of scope: ML/AI frameworks and fuzzy logic libraries

Neural networks and fuzzy logic systems are computational paradigms, not languages — they're implemented *in* languages already listed above (Python, C++, Java). The `apt` repos do carry relevant libraries (`libfann-dev` for FANN neural nets, `python3-sklearn` for scikit-learn, `python3-scipy` for scientific computing), but those are libraries, not languages.

Turing-completeness note: recurrent neural networks with sufficient precision have been formally proven Turing-complete. Standard fuzzy inference systems (Mamdani, Sugeno) have *not* — they're bounded function approximators. So "not Turing-complete" is actually the correct answer for plain fuzzy logic, but irrelevant for neural nets.

---

## Quick count

| Category | Count |
|---|---|
| Pre-installed | 7 |
| Compiled / systems | 17 |
| JVM / managed | 8 |
| Scripting / interpreted | 17 |
| Functional | 17 |
| Logic / constraint | 5 |
| Shells | 10 |
| Turing-complete non-traditional | 19 |
| Math / scientific | 7 |
| Assembly / IR | 6 |
| Hardware description | 3 |
| Proof assistants | 4 |
| Esoteric | 5 |
| **Total** | **~125** |

The "non-traditional TC" row is the most surprising: `sed`, `make`, `m4`, `jq`, `TeX`,
`PostScript`, `dc`, and `SQL` are all available on a default Ubuntu install (or one package
away) and are all definitively Turing-complete — they just don't get called "programming
languages" in polite company.
