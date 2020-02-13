exe:
	OMPI_CC=clang mpicc -o scale_test scale_test.c 	
ir:
	OMPI_CC=clang mpicc -emit-llvm scale_test.c -c -o scale_test.bc
	llvm-dis < scale_test.bc > scale_test.ll
clean:
	rm -f scale_test *.o *.bc *.ll
