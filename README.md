# Bytecode generator

A relatively simple script that generates pretty fast bytecode systems.
Relies on some non-standard C, but should in theory be completely portable.

# Usage

```
bcgen <rules.py>
```

The script is pretty bare bones at the moment. Essentially, it generates a number of files in a
directory called `gen` in the working directory. These generated files are included by `bcode.c`,
which you can compile into an object file that provides an interface to compiling and running your
bytecode system.

`rules.py` is a file that describes which operations to generate. Here's an example `rules.py`:
```
g = BCGen(r = 4, f = 0)
g.rule('addi', 'rRR__I', 'R0 = R1 + IMM;')
```

`BCGen()` is the generator class. It has three optional parameters:
+ `r` for number of general purpose registers.
+ `f` for number of floating point registers.
+ `head` for user configurable code, placed just before execution of the bytecode.

`rule()` is the workhorse here. It takes three parameters:
+ `name` for the instruction name.
+ `fmt` for the instruction format.
+ `body` for the instruction body.

The `fmt` parameter is a string of six characters, from left to right:

1) Relocation slot, `r` or `_`

`r` means that the when compiling the instruction, it should return a relocation that can
later be patched with a value. `_` means that no relocation is necessary.

2) Four register slots, `R`, `F` or `_`:

`R` refers to a general purpose register, and `F` refers to a floating point register.
`_` means the register slot is unused. When compiling an operation, the user can pass
in a number for each register slot that is the index of the register to use. Each operation
can take a maximum of four register arguments.

3) Immediate slot, `I`, `D` or `_`:

`I` refers to an integer immediate value, `D` refers to a floating point immediate value.
`_` means the slot is unused. The user can pass in an immediate value that is accessible
to the operation at runtime.

The body of the instruction is what gets executed. The register arguments are referenced
via `R` + index for the general purpose registers and `F` + index for the floating point registers.
Immediate values are accessed via `IMM` for integer and `DIMM` for doubles, respectively.

The index to use when specifying register arguments is the register slot index, so for exampl
the format `_R_FR_` makes registers `R0`, `F2` and `R3` available to the body. It's *very* important
to note that `R0` does not refer to the literal first register in the machine, rather it's the
first register argument the user gives when compiling the instruction.

Each instruction can be compiled via the generated procedure `select_name()`. For the example
`rules.py` above, to compile the instruction `addi` you would call `select_addi(0, 2, 500);`.
The first and second arguments are the register indexes to use, and map to `R0` and `R1`.
The immediate is always last, and in this case it's `500`. One way to look at the situation is
that we're requesting that a bytecode instruction be compiled where register 2 plus `500` be
placed into register 0.

## Jumps

By default the following bytecode instruction is executed automatically. You can however specify
explicit jumps, such as in branches, by `JUMP(target)`. The target can be patched in later or be
specified as an immediate. For example:
```
# rules.py
...
g.rule('beq', 'rRR__I', 'if (R0 == R1) JUMP(IMM);')
```
```
// file.c
breg_t label = label();
...
select_beq(0, 1, label.g); // we already know where we want to jump

// OR
breg_t reloc = select_beq(0, 1, 0); // zero is placeholder
...
breloc_t label = label();
patch(reloc, label); // now we know where we want to jump
```

(Note that the patch system should be improved, but good enough for now)

See `example` for a simple test program that generates a limited set of operations that are enough
to sum the first billion integers.

## Performance

As a very basic test, below is a comparison of `example/exec` found in this repo 
in relation to some other methods to accomplish summing the first billion integers.

| 'Method'               | Time (s) |
|------------------------|----------|
| C, `-O3 -march=native` | 0.062    |
| C, `-O2`               | 0.221    |
| LuaJIT                 | 0.656    |
| C, `-O0`               | 0.709    |
| `example/exec`         | 1.533    |
| Lua                    | 18.717   |
| Perl                   | 27.962   |

The full C program is
```{C}
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
        assert(argc == 2);
        unsigned long sum = 0;
        unsigned long n = strtoull(argv[1], 0, 0);
        for (unsigned long i = 0; i < n; ++i)
                sum += i;

        printf("%llu\n", sum);
        return 0;
}
```

Note the user input so as to avoid completely optimizing away the loop.

# Notes

This software is currently very crude and pretty much only suitable for a
simple demonstration. However, I think the performance figures are
pretty impressive for what it is. A JIT system will essentially always
beat this system in speed, but this is trivially portable and the
code generation is ligtning fast.

A fairly close match in the JIT world is `lightning` (and its lighter fork,
`lightening`). My intention is to try and use this tool to generate a bytecode
backend for `lightening` as a fallback for platforms that `lightening` doesn't
yet support to create a 'universal' low level virtual machine.

Currently you may have to manually modify `bcode.c` to suit your project,
for example adding headers etc. I might improve the tool to output a single
header + implementation file, similar to `flex` and/or `bison` but the current
method is good enough for now.

No restrictions are placed on register count or instruction count.
The instruction count increases the size and compile time linearly, but register
count affects the size and compile time exponentially so be careful.
Also, to maintain the highest performance, the register count should be
smaller than the register count of the underlying hardware. This allows
the compiler to lower bytecode registers to hardware registers, speeding things
up considerably.

Finally, note that the system doesn't assign any specific usage conventions
to registers. You are responsible for maintaining an ABI of some kind, with
stack/frame register(s), callee-save vs. caller-save, etc.
