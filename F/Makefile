exe:
	OMPI_FC=flang mpifort -o scale_test scale_test.f90 	
ir:
	OMPI_FC=flang mpifort -emit-llvm scale_test.f90 -c -o scale_test.bc
	llvm-dis < scale_test.bc > scale_test.ll
clean:
	rm -f scale_test *.o *.bc *.ll
