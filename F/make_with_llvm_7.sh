llvm=/home/jzarins/install/llvm-old-flang/
export PATH=$llvm/bin:$PATH

make clean
make use_module.exe
make ll

cp /home/jzarins/sources/llvm_hpc_static_analysis/C/llvm_analysis_funcs.ll .
llvm-link -S -o use_module_linked.ll use_module.ll set_module.ll scale_module.ll llvm_analysis_funcs.ll

#make all
