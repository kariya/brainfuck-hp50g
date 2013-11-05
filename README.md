# BrainF*ck imlementation for HP50g calculator

## What's this?
This is a implementation (currently interpreter done, jit compiler in development) of BrainF*ck language[2] for HP50g calculator.
It's an ARM native code application, not emulated Saturn code.
(See [4] if you are looking for SysRPL version.)


## Usage
Copy bin/bfi.hp and data/*.b to HP50g.

Push a string of brainf*ck code(*.b) to the calculator stack.

Optionally you can push to the stack another string as input.

Execute the BFI.HP program and you can get the output string on the stack.

(See also screenshots in the img/screenshot directory)

## Caution
If brainf*ck code goes to infinite loop, there's no way to interrupt it.
You might need a paper clip to reset the calculator.


## How to compile
Install hpgcc (see [1]) and do "make bfi.hp" in src/interpreter directory.


## Purpose
The purpose of this work is my exercise of learning ARM programming.

Now I prototyped an JIT compiler successfully with a few brainf*ck code running.
My aim was partially done but I'll keep brushing it up.

### Really JIT?
The Current version is a kind of 'compile-and-go' style compiler,
but it shares a lot of ideas with JIT.
It seems easy to make a trace-based JIT but it is questionable whether to get more speed.


## License
GPL


## Reference
[1] Extend your 50g with C. http://sense.net/~egan/hpgcc/

[2] BrainF*ck http://en.wikipedia.org/wiki/Brainfuck

[3] http://stackoverflow.com/questions/4461609/native-self-modifying-code-on-android

[4] https://groups.google.com/forum/#!searchin/comp.sys.hp48/brainfuck/comp.sys.hp48/vjbBV5OKxjY/EKysmIrSMuoJ
