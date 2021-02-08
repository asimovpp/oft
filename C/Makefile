CC=OMPI_CC=clang mpicc
FLAGS= -g -O2
ALL_SRC=$(wildcard *.c)
ALL_EXE=$(patsubst %.c, %.exe, $(ALL_SRC))
ALL_OBJ=$(patsubst %.c, %.o, $(ALL_SRC))
ALL_BC=$(patsubst %.c, %.bc, $(ALL_SRC))
ALL_LL=$(patsubst %.c, %.ll, $(ALL_SRC))

all: info obj ll
obj: $(ALL_OBJ)
exe: $(ALL_EXE)
bc: $(ALL_BC)
ll: $(ALL_LL)

%.bc : %.c
	$(CC) $(FLAGS) -emit-llvm $< -c -o $@

%.ll : %.c
	$(CC) $(FLAGS) -emit-llvm $< -S -o $@

%.o : %.c
	$(CC) $(FLAGS) -c -o $@ $<
    

clean:
	rm -f $(ALL_EXE) *.o *.bc *.ll

info:
	echo ${PATH}
	flang --version
	clang --version
	llvm-dis --version
	#ompi_info


#%.exe : %.c
#	$(CC) $(FLAGS) -o $@ $<
