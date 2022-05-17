source $WORK/setup_work_env.sh

INSTALL_DIR=$WORK"/binutils/"

git clone --depth 1 --branch binutils-2_35_1 git://sourceware.org/git/binutils-gdb.git binutils_source
cd binutils_source
mkdir build
cd build
CC=gcc CXX=g++ ../configure --prefix=${INSTALL_DIR} --enable-gold --enable-plugins --disable-werror 
make all-gold
make install-gold
mkdir -p ${INSTALL_DIR}lib/bfd-plugins
