# BrainF*ck imlementation for HP50g calculator

## What's this?
This is a implementation (currently interpreter done, jit compiler in development) of BrainF*ck language[2] for HP50g calculator.
It's an ARM native code application, not emulated Saturn code.


## Usage
Copy bin/bfi.hp and data/*.b to HP50g.

Push a string of brainf*ck code(*.b) to the calculator stack.

Optionally you can push to the stack another string as input.

Execute the BFI.HP program and you can get the output string on the stack.


## Caution
If brainf*ck code goes to infinite loop, there's no way to interrupt it.
You might need a paper clip to reset the calculator.


## How to compile
Install hpgcc (see [1]) and do "make bfi.hp" in src/interpreter directory.


## Purpose
The purpose of this work is my exercise of learning ARM programming.

Now I prototyped an JIT compiler successfully with a few brainf*ck code running.
My aim was partially done but I'll keep brushing it up.


## License
GPL


## Reference
[1] Extend your 50g with C. http://sense.net/~egan/hpgcc/

[2] BrainF*ck http://en.wikipedia.org/wiki/Brainfuck

[3] http://stackoverflow.com/questions/4461609/native-self-modifying-code-on-android

