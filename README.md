# Test cases

## C

This folder contains some overflow test cases written in C.

## F

This folder contains some overflow test cases written in Fortran.


# LLVM overflow analysis tool

The `llvm` folder contains the integer static analysis tool.
It has to be built as part of LLVM, for example:

1. download llvm source code
2. copy (or create symbolic links) the `llvm` directory into the root of the download llvm directory, so that it merges with the llvm directory there
3. build llvm as normal
4. run the tool with something like `opt -load ./lib/HPCAnalysis.so -basicaa -globals-aa -cfl-steens-aa -tbaa -scev-aa -cfl-anders-aa -objc-arc-aa -scoped-noalias -analyse_scale  < ~/llvm_hpc_static_analysis/C/scale_test.ll 2> output.ll`
