# BrainF*ck imlementation for HP50g calculator

## What's this?
This is a implementation (currently interpreter only) of BrainF*ck language[2] for HP50g calculator.
It's an ARM native code application, not emulated Saturn code.


## Usage
Copy bin/bfi.hp and data/*.b to HP50g.

Push a string of brainf*ck code(*.b) to the calculator stack.

Optionally you can push to the stack another string as input.

Execute the BFI.HP program and you can get the output string on the stack.


## Caution
If brainf*ck code goes to infinite loop, there's no way to interrupt it.
You might have a paper clip to reset the calculator.


## How to compile
Install hpgcc (see [1]) and do "make bfi.hp" in src/interpreter directory.


## Purpose
The purpose of this work is my exercise of learning ARM programming.
Of cource the current version is a toy and very simple implementation, but ... (see TOOD)


## TODO
normal and JIT compiler. (I say it a bit seriously.)


## License
GPL


## Reference
[1] Extend your 50g with C. http://sense.net/~egan/hpgcc/

[2] BrainF*ck http://en.wikipedia.org/wiki/Brainfuck


