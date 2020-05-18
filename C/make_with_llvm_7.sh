llvm=/home/jzarins/install/llvm-old-flang/
llvm=/home/jzarins/sources/llvm-project/build-v10/
export PATH=$llvm/bin:$PATH

echo $PATH
flang --version
clang --version
llvm-dis --version
#ompi_info

make
