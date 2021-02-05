
FC=OMPI_FC=flang mpifort
CC=OMPI_FC=clang mpicc
#FC=flang
FLAGS=-O0
CLAGS=-O0
ALL_SRC=$(wildcard *.f90)
#ALL_SRC=hello_world.f90
ALL_BC=$(patsubst %.f90, %.bc, $(ALL_SRC))
ALL_LL=$(patsubst %.f90, %.ll, $(ALL_SRC))
ALL_OBJ=$(patsubst %.f90, %.o, $(ALL_SRC))
ALL_MOD=$(patsubst %.f90, %.mod, $(ALL_SRC))


all: info $(ALL_EXE) $(ALL_BC) $(ALL_LL) 
bc: $(ALL_BC)
ll: $(ALL_LL)

%.bc : %.f90 
	$(FC) $(FLAGS) -emit-llvm $< -c -o $@

%.ll : %.bc
	llvm-dis < $< > $@

%.o : %.f90 
	$(FC) $(FLAGS) -c $<


# I use the same command for linking, just pass it different object files. 
# Can I make this nicer? just specify dependencies 
allocate_test.exe : allocate_test.o
	$(FC) $(FLAGS) $< -o $@

hello_world.exe : hello_world.o
	$(FC) $(FLAGS) $< -o $@

scale_test_nompi.exe : scale_test_nompi.o
	$(FC) $(FLAGS) $< -o $@

scale_test.exe : scale_test.o
	$(FC) $(FLAGS) $< -o $@

set_module.o use_module.o : scale_module.o
use_module.o : set_module.o 

use_module.exe : use_module.o scale_module.o set_module.o  
	$(CC) $(CFLAGS) -c c_interface.c
	$(FC) $(FLAGS) $^ c_interface.o -o $@
    

%.exe : %.f90
	$(FC) $(FLAGS) -o $@ $<

clean:
	rm -f *.exe *.o *.bc *.ll *.dbg *.mod

info:
	echo ${PATH}
	flang --version
	clang --version
	llvm-dis --version
	#ompi_info
