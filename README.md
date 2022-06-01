# What is OFT

Overflow Tool (OFT) is an LLVM based tool for predicting Scaling bugs – errors that only manifest at large scale simulations, in terms of number of parallel workers or input size. OFT can analyse applications to identify and instrument potentially overflowable integer instructions. Running the instrumented application at a few small scales can show trends in integer value scaling and reduce the false positive rate of the static analysis part.

OFT is described in more detail in (reference TBD).


# Install

## Pre-requisites


### Binutils
The gold linker with plugin support and the LLVM plugin is a good way of producing whole-application bitcode when dealing with complicated code.

See `utils/scripts/misc/install_binutils.sh` for an example build script.
_Note_: you may need to also install `texinfo` to have access to `makeinfo`, which is used during the build.

After you build the LLVM plugin (see next section), copy `your-llvm-install-dir/lib/LLVMgold.so` to `your-binutils-install-dir/lib/bfd-plugins/`.


### LLVM and Flang
OFT can be built out-of-tree, but still requires access to LLVM source code, so it is recommended to compile LLVM before building OFT.

If you plan to use OFT for Fortran code, you will need the flang compiler which requires its own LLVM distribution: https://github.com/flang-compiler/classic-flang-llvm-project and itself resides at https://github.com/flang-compiler/flang

See `utils/scripts/misc/install_llvm_flang.sh` for an example build script.
_Note_: this script assumes that binutils headers are available, in order to build the gold linker LLVM plugin. 


### OpenMPI
If your code uses MPI, you will also need an MPI library installation that uses clang/flang compilers. 

See `utils/scripts/misc/install_openmpi4.sh` for an example build script.


## Build OFT
Make sure the pre-requisites are available on `PATH` and `LD_LIBRARY_PATH`. Then from the root directory of OFT:

```bash
mkdir build
cd build
bash ../utils/scripts/build/build.sh
ninja
ninja install
```

_Note_: delete the line `-G Ninja \` from `build.sh` to use `make` instead of `ninja`.

_Note_: OFT will be installed to `oft_root/install`.

_Note_: you may have to edit `utils/scripts/build/export_deps.sh` to suit your environment.


## Test suite
In the build directory, run

```bash
ninja check
```

to execute OFT's suite.



# Use


### Marking scale variables
_Scale variables_ are variables that correspond to some aspect of an application's scale, e.g. the level of parallelism or the problem size; they are the starting points for tracing that OFT performs. Instructions that are directly or indirectly influenced by scale variables are called _scale instructions_.

By default OFT will look for MPI instructions which set communicator size and rank, and trace these scale variables.
Other scale variables (usually pertaining to problem size) can be set by inserting calls to `oft_mark` (Fortran) or `oft_mark_` (C/C++) in the original source code (before converting it to whole-application bitcode). For example:

```fortran
! Fortran
program my_app
  implicit none
  external oft_mark
  ...
  integer number_of_particles
  ...
  call oft_mark(numer_of_particles)
  ...
end 
```

```c
// C/C++
extern void oft_mark_(void *);
int main() {
  int number_of_particles;
  ...  
  oft_mark_(&size);
  ...
}
```

The call to `oft_mark` does not perform any actions. It is used by OFT simply to identify scale variables to trace.


## Produce whole-application bitcode
OFT analyses code at the LLVM bitcode level. Whole-application bitcode in a single file is required to do a full analysis of a code.
It is possible to compile individual bitcode files and link them together using `llvm-link`.

A more automatic option is to exploit the link time optimisation (LTO) route together with the gold linker with an LLVM plugin (see previous sections for guidance on setting this up).
If this is set up, then a few flag changes should result in the whole build process of your application taking place in terms of bitcode. You need to add LTO flags to compilation and gold flags to linking, e.g.

```Makefile
CFLAGS += -flto
LFLAGS += -fuse-ld=gold -Wl,-plugin-opt=emit-llvm
```

Any external libraries that have not been compiled as bitcode will simply be ignored. In the tracing process they will appear as dead-end references without an available implementation.


## Analyse bitcode using OFT
You can invoke OFT directly by doing:

```bash
opt -load-pass-plugin $OFT_INSTALL/lib/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o instrumented_app.ll < my_app.ll 2> oft_instrumentation.txt
```

where `my_app.ll` is your whole-application bitcode, `instrumented_app.ll` is your application with max-value recording instrumentation around potentially overflowable scale instructions and `oft_instrumentation.txt` is information given by OFT.


## Static analysis output
After running OFT as in the example in the previous section, the file `oft_instrumentation.txt` will look something like:

```
Library scale variables found:
├  %14 = bitcast i8* %13 to i64*, !dbg !22 on Line 89 in file foo.f90
├  %21 = bitcast i8* %20 to i64*, !dbg !23 on Line 90 in file foo.f90
--------------------------------------------
--------------------------------------------
Annotation scale variables found:
├  %27 = bitcast i32* %2 to i64*, !dbg !21 on Line 366 in file bar.f90
├  %47 = bitcast i32* %8 to i64*, !dbg !27 on Line 378 in file bar.f90
--------------------------------------------
ID 0 given to ├  %2776 = mul nsw i32 %2774, %2775, !dbg !186 on Line 563 in file baz.f90
ID 1 given to ├  %2717 = lshr i32 %2716, 31, !dbg !186 on Line 563 in file baz.f90
```

"Library scale variables" list scale variable tracing starting points found in MPI functions. "Annotation scale variables" list scale variable tracing starting points found due to `oft_mark` annotations. The remaining output lists instructions (with an assigned ID) that have been deemed "potentially overflowable" and have been instrumented. An instruction is overflowable if it is directly or indirectly affected by a scale variable, operates on 32-bit integers and is an arithmetic operation. Currently the following arithmetic operations are considered:  Add, Sub, Mul, Shl, LShr and AShr.


## Dynamic analysis
To compile the instrumented application into an executable, do:

```bash
llc -o instrumented_app.s instrumented_app.ll
mpifort -o instrumented_app.exe instrumented_app.s $OFT_INSTALL/runtime/lib/liboft_rt.a
```

Note the inclusion of the `liboft_rt.a` library on the second line. This is adds function bodies to the functions inserted during instrumentation. The inserted functions keep track of the maximum value of the detected potentially overflowable scale instructions.

After execution, e.g. `mpirun -n 4 instrumented_app.exe`, instrumentation output will be written to `oft_output.txt`:

```
Rank 0; Value 0 maxed at 2752
Rank 0; Value 1 maxed at 0
Rank 0; Value 2 maxed at 2188
...
Rank 1; Value 0 maxed at -1
Rank 1; Value 1 maxed at -1
Rank 1; Value 2 maxed at -1
...
Rank 99; Value 0 maxed at 34566
Rank 99; Value 1 maxed at 234
Rank 99; Value 2 maxed at 9483
```

Here "Value _N_" refers to "ID _N_" from the output of the static analysis step, `oft_instrumentation.txt`. Values that have "maxed at -1" have not been touched (this may have only happened on some ranks, and not others). This reflects the fact that a single run of an application may not exercise all parts of a complex codebase. 

OFT may also produce `oft_debug.txt`. Currently this only happens if there are multiple code paths to `MPI_Finalize` and OFT instrumented the wrong one, so the results may have incorrect max values.


## Performing scaling tests

Once the application is instrumented, a scaling test can be performed to reduce the number of false positives identified in the static analysis part. This involves running the instrumented application at a few small scales (e.g. using 4, 8, 16, 32 MPI ranks or a few small problem sizes) and observing which instrumented values grow quickly with increasing scale of the application. 
This analysis can be made easier with the additional tools we provide. These tools parse the output of OFT runs and classify the trends as "falling", "static" or "growing". Furthermore, a polynomial trend line is fit to each value and extrapolated to predict the scale at which the value would overflow.

The tools are available at: https://github.com/asimovpp/oft-result-analysis
