LIBOFT_A=/home/jzarins/sources/llvm_hpc_static_analysis/llvm/OverflowTool/build/runtime/liboft_rt.a

FC=OMPI_FC=flang mpifort
CC=OMPI_FC=clang mpicc
FFLAGS=-O0
CFLAGS=-O0
ALL_SRC=$(wildcard *.f90)
ALL_BC=$(patsubst %.f90, %.bc, $(ALL_SRC))
ALL_LL=$(patsubst %.f90, %.ll, $(ALL_SRC))
ALL_OBJ=$(patsubst %.f90, %.o, $(ALL_SRC))
ALL_MOD=$(patsubst %.f90, %.mod, $(ALL_SRC))
ALL_EXE=allocate_test.exe annotate_test.exe hello_world.exe scale_test.exe scale_test_nompi.exe use_module.exe

all: info ll exe
obj: $(ALL_OBJ)
exe: $(ALL_EXE)
bc: $(ALL_BC)
ll: $(ALL_LL)


%.bc : %.f90 
	$(FC) $(FFLAGS) -emit-llvm $< -c -o $@

%.ll : %.f90
	$(FC) $(FFLAGS) -emit-llvm $< -S -o $@

%.o : %.f90 
	$(FC) $(FFLAGS) -c $<

%.o : %.c
	$(CC) $(CFLAGS) -c $< 

%.mod : %.f90 
	$(FC) $(FFLAGS) -c $<

use_module.ll: scale_module.mod set_module.mod

annotate_test.exe : annotate_test.o
	$(FC) $(FFLAGS) $< $(LIBOFT_A) -o $@

allocate_test.exe : allocate_test.o
	$(FC) $(FFLAGS) $< -o $@

hello_world.exe : hello_world.o
	$(FC) $(FFLAGS) $< -o $@

scale_test_nompi.exe : scale_test_nompi.o
	$(FC) $(FFLAGS) $< -o $@

scale_test.exe : scale_test.o
	$(FC) $(FFLAGS) $< -o $@

set_module.o use_module.o : scale_module.o
use_module.exe : use_module.o scale_module.o set_module.o c_interface.o 
	$(FC) $(FFLAGS) $^ -o $@
    

clean:
	rm -f *.exe *.o *.bc *.ll *.dbg *.mod

info:
	echo ${PATH}
	flang --version
	clang --version
	llvm-dis --version
	#ompi_info
