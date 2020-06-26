CC=OMPI_CC=clang mpicc
FLAGS= -g -O0
ALL_SRC=$(wildcard *.c)
ALL_EXE=$(patsubst %.c, %.exe, $(ALL_SRC))
ALL_BC=$(patsubst %.c, %.bc, $(ALL_SRC))
ALL_LL=$(patsubst %.c, %.ll, $(ALL_SRC))

all: info $(ALL_EXE) $(ALL_BC) $(ALL_LL)
exe: $(ALL_EXE)
bc: $(ALL_BC)
ll: $(ALL_LL)

%.bc : %.c
	$(CC) $(FLAGS) -emit-llvm $< -c -o $@

%.ll : %.c
	$(CC) $(FLAGS) -emit-llvm $< -S -o $@

#%.ll : %.bc
	#llvm-dis < $< > $@
    
%.exe : %.c
	$(CC) $(FLAGS) -o $@ $<

clean:
	rm -f $(ALL_EXE) *.o *.bc *.ll

info:
	echo ${PATH}
	flang --version
	clang --version
	llvm-dis --version
	#ompi_info
