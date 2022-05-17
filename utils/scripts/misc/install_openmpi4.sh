INSTALL_DIR=$WORK"/openmpi-4.1.0-llvm-12/"

wget https://download.open-mpi.org/release/open-mpi/v4.1/openmpi-4.1.0.tar.gz
tar -xf openmpi-4.1.0.tar.gz
cd openmpi-4.1.0
mkdir build
cd build
../configure CFLAGS=-fPIC CXXFLAGS=-fPIC FFLAGS=-fPIC CC=clang CXX=clang++ F77=flang FC=flang --prefix=${INSTALL_DIR}
make -j 8
make install
