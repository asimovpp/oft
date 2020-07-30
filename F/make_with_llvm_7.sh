llvm=/home/jzarins/install/llvm-old-flang/
export PATH=$llvm/bin:$PATH

make clean
make ll

llvm-link -S -o use_module_linked.ll use_module.ll set_module.ll scale_module.ll

#make all
