# BrainF*ck implementation for HP50g calculator

## What's this?
This is a implementation (interpreter, jit compiler) of BrainF*ck language[2] for HP50g calculator.
It's an ARM native code application, not emulated Saturn code.
(See [4] if you are looking for SysRPL version.)


## Usage
Copy bin/bfi.hp, bin/bfij16.hp and data/*.b to your HP50g.

Push a string of brainf*ck code(*.b) to the calculator stack.

Optionally you can push to the stack another string as input.

Execute either of the BFI.HP or BFJ16.HP programs and you can get the output string on the stack.

(See also my screenshots in the img/screenshot directory)

## Caution
If brainf*ck code goes to infinite loop, there's no way to interrupt it.
You might need a paper clip to reset the calculator.


## Files
bin/bfi.hp interpreter

bin/bfj16.hp jit compiler(thumb version)


## How to compile
Install hpgcc (see [1]) and do "make bfi.hp" in src/interpreter directory.
Or "make bfjit16.hp" in src/jit directory.

## Purpose
The purpose of this work is my exercise of learning ARM programming.

Now I prototyped an JIT compiler successfully with a few brainf*ck code running.
My aim was achieved but I'll keep brushing it up.

### Really JIT?
The current version is a kind of 'compile-and-go' style compiler,
but it shares a lot of ideas with JIT (dynamic code generation).
It seems easy to make a trace-based JIT but it is questionable whether to get more speed.


## TODO
Making 32 bit (non-thumb) version seems trivial, but I'd like to see difference of speed between 16 bit and 32 bit versions.

I have a plan, in my mind, to implement a Forth interpreter and (JIT) compiler on HP50g.



## License
GPL


## Reference
[1] Extend your 50g with C. http://sense.net/~egan/hpgcc/

[2] BrainF*ck http://en.wikipedia.org/wiki/Brainfuck

[3] http://stackoverflow.com/questions/4461609/native-self-modifying-code-on-android

[4] https://groups.google.com/forum/#!searchin/comp.sys.hp48/brainfuck/comp.sys.hp48/vjbBV5OKxjY/EKysmIrSMuoJ


[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/kariya/brainfuck-hp50g/trend.png)](https://bitdeli.com/free "Bitdeli Badge")

