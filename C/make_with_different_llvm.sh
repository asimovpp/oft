#llvm=/home/jzarins/install/llvm-old-flang/
#llvm=/home/jzarins/sources/llvm-project/build-v10-release/
#llvm=/home/jzarins/sources/llvm-project/build-v10-debug/
llvm=/home/jzarins/sources/clang+llvm-7.1.0/
#llvm=/home/jzarins/sources/llvm-project/build-master/
export PATH=$llvm/bin:$PATH

make bc

make
OMPI_CC=clang mpicc -g -O0 scale_test.c scale_test_aux.c -o scale_test.exe
llvm-link scale_test.bc scale_test_aux.bc llvm_analysis_funcs.bc -o scale_test_linked.bc
llvm-dis < scale_test.bc > scale_test.ll
llvm-dis < scale_test_linked.bc > scale_test_linked.ll  
llvm-dis < llvm_analysis_funcs.bc > llvm_analysis_funcs.ll 
