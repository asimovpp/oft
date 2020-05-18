
FC=OMPI_FC=flang mpifort
#FC=flang
FLAGS=-O0 
ALL_SRC=$(wildcard *.f90)
#ALL_SRC=hello_world.f90
ALL_EXE=$(patsubst %.f90, %.exe, $(ALL_SRC))
ALL_BC=$(patsubst %.f90, %.bc, $(ALL_SRC))
ALL_LL=$(patsubst %.f90, %.ll, $(ALL_SRC))


all: info $(ALL_EXE) $(ALL_BC) $(ALL_LL) 
exe: $(ALL_EXE)
bc: $(ALL_BC)
ll: $(ALL_LL)

%.bc : %.f90 
	$(FC) $(FLAGS) -emit-llvm $< -c -o $@

%.ll : %.bc
	llvm-dis < $< > $@
    
%.exe : %.f90
	$(FC) $(FLAGS) -o $@ $<

clean:
	rm -f $(ALL_EXE) *.o *.bc *.ll *.dbg

info:
	echo ${PATH}
	flang --version
	clang --version
	llvm-dis --version
	#ompi_info
