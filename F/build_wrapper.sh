#make obj
make ll

cp /home/jzarins/sources/llvm_hpc_static_analysis/C/llvm_analysis_funcs.ll .
llvm-link -S -o use_module_linked.ll use_module.ll set_module.ll scale_module.ll llvm_analysis_funcs.ll
llvm-link -S -o annotate_test_linked.ll annotate_test.ll llvm_analysis_funcs.ll

make exe
