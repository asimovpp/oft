LIBOFT_A=/home/jzarins/sources/llvm_hpc_static_analysis/llvm/OverflowTool/build/runtime/liboft_rt.a

make ll
make obj

OMPI_CC=clang mpicc -g -O0 scale_test.c scale_test_aux.c -o scale_test.exe
llvm-link scale_test.ll scale_test_aux.ll llvm_analysis_funcs.ll -S -o scale_test_linked.ll

OMPI_CC=clang mpicc -g -O0 -o annotation_test.exe annotation_test.o $LIBOFT_A 
