# Compiling with libclang for dummies, by dummies

You probably need to install
- LLVM 
- clang

I think I am compiling with 6.0. I have many versions installed, but that is the one `llvm-config` is running.

Compile:

```bash
$ g++ -std=c++11 compiler.cpp LLVMInstanceManager.cpp `llvm-config --ldflags --libs --cxxflags` -lclangDriver -lclangBasic
```

Then run the executable generated and it will compile `compile_me.c`