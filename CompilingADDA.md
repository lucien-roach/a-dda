

# Introduction #

The simplest way to compile `ADDA` is using the provided `Makefile`. This can be done on any system with [GNU make](http://www.gnu.org/software/make/) (version 3.81 or higher) and C, Fortran, and C++ compilers installed, which includes but not limited to almost all Unix-type systems. Core parts of `ADDA` are written in C, while Fortran and C++ are used for relatively small parts that are not always required (see below).

Position yourself in the `src/` directory of the [ADDA package](PackageDescription.md) and type "`make seq`", "`make mpi`", or "`make ocl`" to compile the sequential, MPI, or OpenCL version respectively (on some systems, GNU `make` is available under a different name, e.g. `gmake`) . Compiled executables are then `src/seq/adda`, `src/mpi/adda_mpi`, and `src/ocl/adda_ocl` respectively. Full list of possible targets (arguments to `make`) is the following:

| `all` | The default target (if none is specified). Invokes `seq`, `mpi`, and `ocl`. |
|:------|:----------------------------------------------------------------------------|
| `seq` | Compiles sequential executable of `ADDA`.                                   |
| `mpi` | Compiles parallel ([MPI](http://www.mpi-forum.org/)) executable of `ADDA`.  |
| `ocl` | Compiles [OpenCL](http://www.khronos.org/opencl/) (GPU accelerated) executable of `ADDA` |

| `cleanfull` | Removes all files produced during compilation and `ADDA` runs. Invokes `clean` and `cleanruns`. |
|:------------|:------------------------------------------------------------------------------------------------|
| `clean`     | Removes all files produced during compilation. Invokes `cleanseq`, `cleanmpi`, and `cleanocl`.  |
| `cleanseq`  | Removes files produced during compilation of sequential version (located in `src/seq/`).        |
| `cleanmpi`  | Removes files produced during compilation of MPI version (located in `src/mpi/`).               |
| `cleanocl`  | Removes files produced during compilation of OpenCL version (located in `src/ocl/`).            |

| `cleanruns` | Removes all files produced during `ADDA` runs. Invokes `cleanrunsseq`, `cleanrunsmpi`, and `cleanrunsocl`. |
|:------------|:-----------------------------------------------------------------------------------------------------------|
| `cleanrunsseq` | Removes files produced during run of sequential version (located in `src/seq/`).                           |
| `cleanrunsmpi` | Removes files produced during run of MPI version (located in `src/mpi/`).                                  |
| `cleanrunsocl` | Removes files produced during run of OpenCL version (located in `src/ocl/`).                               |

The [Makefile](http://code.google.com/p/a-dda/source/browse/trunk/src/Makefile) is designed to work out-of-box on many systems using [GNU compilers](http://www.gnu.org/software/gcc/) (intercompatible versions of `gcc` and `gfortran` are required). However, [FFTW3 package need to be installed](InstallingFFTW3.md) on your system. Moreover, MPI version of `ADDA` requires [installed MPI implementation](InstallingMPI.md). Using the latter, `ADDA` can run both on multiprocessor systems (clusters) and on multicore processors of a single PC, as well as on heterogeneous hardware.

The OpenCL version of `ADDA` is designed to use graphic cards (GPUs) to accelerate computations. It requires [OpenCL environment](OpenCL.md) and [clAmdFft library](InstallingclAmdFft.md) (alternatively, slower built-in OpenCL FFT routines can be used). The future goal is to make OpenCL version capable of running on other compatible hardware, including hybrid CPU-GPUs and [Cell processors](http://en.wikipedia.org/wiki/Cell_(microprocessor)).

`ADDA` is fully ready for 64-bit mode, which allows it to use practically unlimited memory per single process. Moreover, it will automatically compile as such in a 64-bit operating system.

# Compilation options #

The details of compilation are controlled by a number of options (described in a table below), which can be specified by three different methods. First, one may change or uncomment the corresponding lines in the [Makefile](http://code.google.com/p/a-dda/source/browse/trunk/src/Makefile), starting with "`name = ...`" or "`override name += `", where `name` is the option name. These lines are accompanied by a lot of descriptive comments. Second, these options may be set as environmental variables before executing `make`. Third, they may be given as additional command line parameters when invoking `make`, for example
```
make COMPILER=intel OPTIONS="DEBUG FFT_TEMPERTON" ...
```
Note that multi-word value of `OPTIONS` need to be enclosed in quotes.

| **Name** | **Value** | **Description** |
|:---------|:----------|:----------------|
| `COMPILER` | one of    |                 |
|          | `gnu`     | [GNU compilers](http://www.gnu.org/software/gcc/). The default value. |
|          | `intel`   | [Intel compilers](http://software.intel.com/en-us/intel-compilers/). There are [problems](http://code.google.com/p/a-dda/issues/detail?id=146) with v.11.1. |
|          | `compaq`  | [Compaq compilers](http://h30097.www3.hp.com/dtk/compaqc_ov.html). Has not been recently tested, hence may need modification of compiler flags. |
|          | `ibm`     | [IBM compilers](http://www-142.ibm.com/software/products/us/en/subcategory/SW760). Has not been recently tested, hence may need modification of compiler flags. |
|          | `hpux`    | Specific options for HP-UX ia64 environment |
|          | `other`   | Use this if none of the above works. Need manual specification of compiler flags (see below). |

| `OPTIONS` | any combination of | |
|:----------|:-------------------|:|
|           | `DEBUG`            | Turns on producing debug symbols (`-g`) and warnings by the compiler and slightly reduces optimization level. This option is irrelevant, if `DEBUGFULL` is used. |
|           | `DEBUGFULL`        | Same as `DEBUG`, but turns off optimization completely and turns on additional diagnostic messages in the source code. |
|           | `FFT_TEMPERTON`    | Uses built-in FFT routines by C. Temperton. This is recommended if you have problems installing `FFTW3`. |
|           | `PRECISE_TIMING`   | Enables precise timing routines, which give extensive timing of all the computation parts of `ADDA`, useful for debugging or optimization studies. |
|           | `NOT_USE_LOCK`     | Does not use file locking for `ExpCount` at all. Enable this flag if you get compilation errors due to unsupported operation system (e.g. [issue 83](https://code.google.com/p/a-dda/issues/detail?id=83)) or experience permanent locks. |
|           | `ONLY_LOCKFILE`    | Uses lock file to lock `ExpCount`, but do not additionally lock this file. Use it if you get warnings when running `ADDA` stating that your system does not support advanced file locking. This option is irrelevant for non-POSIX systems or if `NOT_USE_LOCK` is used. |
|           | `NO_FORTRAN`       | Disables compilation of all Fortran sources. Use it only if compilation of these files causes problems. Corresponding `ADDA` features, namely integration of Green's tensor (`-int igt`), are then disabled. Also, this option is incompatible with `FFT_TEMPERTON`. |
|           | `NO_CPP`           | Disables compilation of all C++ sources. Use it only if compilation of these files causes problems. Currently, it is only relevant for OpenCL version with Apple clFFT routines. |
|           | `OVERRIDE_STDC_TEST` | Disables compilation errors if compiler does not declare its [C99](http://www.open-std.org/JTC1/SC22/WG14/www/standards) conformance. Use it only if you are sure that compiler supports those C99 features that are essential for `ADDA`. |
|           | `OCL_READ_SOURCE_RUNTIME` | Enables reading of OpenCL sources from file during the runtime of `ADDA`. This option is recommended if the default way (incorporating it into `ADDA` executable) is not working. The drawback is that [oclkernels.cl](http://code.google.com/p/a-dda/source/browse/trunk/src/oclkernels.cl) should be moved together with executable. |
|           | `SPARSE`           | Compile `ADDA` in SparseMode (non-FFT). |
|           | `USE_SSE3`         | Enable SSE3 optimizations for the `gcc` compiler and hardware that supports it (most hardware do nowadays). This is used for the calculation of the dipole interaction term, and should yield significant acceleration when used in conjunction with the `SPARSE` option. In normal mode the benefits are minor. |
|           | `CLFFT_APPLE`      | Uses built-in clFFT routines by Apple for OpenCL version (requires C++ compiler). This is recommended if you have problems installing `clAmdFft`. |
|           | `NO_SVNREV`        | Disables obtaining the current subversion revision number of the source code during compilation. Use it only if the corresponding procedure causes any problems (including significant delays). |

| `EXTRA_FLAGS` | any | Additional flags to be passed to the compiler (for all languages). For instance, `-m32` to compile 32-bit binaries in a 64-bit environment. |
|:--------------|:----|:--------------------------------------------------------------------------------------------------------------------------------------------|
| `FFTW3_INC_PATH` | path | Path to `FFTW3` headers. May be not needed, if `FFTW3` is [installed](InstallingFFTW3.md) on a whole system.                                |
| `FFTW3_LIB_PATH` | path | Path to `FFTW3` library. May be not needed, if `FFTW3` is [installed](InstallingFFTW3.md) on a whole system.                                |

Although [Makefile](http://code.google.com/p/a-dda/source/browse/trunk/src/Makefile) contain options for several compilers, described above, the new releases are usually tested only with GNU and Intel compilers. So the options for other compilers may be outdated. If you modify these options to successfully compile the current version of `ADDA`, please communicate these modifications to the [discussion group](http://groups.google.com/group/adda-discuss), so they can be incorporated in future releases.

To use `COMPILER=other` one should manually add definitions of corresponding compiler flags to the [Makefile](http://code.google.com/p/a-dda/source/browse/trunk/src/Makefile), according to the pattern used for predefined compilers, after the line
```
ifeq ($(COMPILER),other)
```
If you do so, please write to the [discussion group](http://groups.google.com/group/adda-discuss), so this compiler can be incorporated in future releases. Please also write to this group if you use one of the supported compilers with `OPTIONS=DEBUG` and obtain any warnings.

If compilation of MPI version does not work out-of-box, you may need to modify [mpi/Makefile](http://code.google.com/p/a-dda/source/browse/trunk/src/mpi/Makefile). This may include specifying a particular compiler wrapper (setting variable `MPICC`) and paths to compiler headers and libraries. For the latter uncomment and adjust lines starting with "`CFLAGS += -I`" and "`LFLAGS += -L`" accordingly. Please see comments in the file for more details.

If compilation of OpenCL version does not work out-of-box, you may need to modify [ocl/Makefile](http://code.google.com/p/a-dda/source/browse/trunk/src/ocl/Makefile) to explicitly specify paths to headers and libraries. For this uncomment and adjust lines starting with "`CFLAGS += -I`" and "`LFLAGS += -L`" accordingly. Please see comments in the file for more details.

When asked to repeat compilation, `make` automatically determines, which source files need to be recompiled. Apart from changes in the source files themselves the logic of `Makefile` also considers the differences in compilation options between current and previous compilation. Thus, it is not necessary to perform any cleaning between the compilations with different options.

# Different platforms #
`ADDA` is intended to be portable to as wide range of platforms as possible. There are only two operating-system-dependent aspects of `ADDA`: precise timing and file locking. Both are optional (controlled by compilation flags, see above) and should be functional on any Windows or POSIX-compliant (Unix) operating system.

GNU `make` and GNU compiler collection has been ported to a wide variety of platforms (including Windows). This makes it possible to compile `ADDA` on all of these platforms exactly as described above. If for some reason it doesn't work, one may compile all source files (with maximum possible optimizations) and link them into an executable. In that case adjusting some of compilation options is possible by uncommenting defines in corresponding header files (see comments in the [Makefile](http://code.google.com/p/a-dda/source/browse/trunk/src/Makefile)).

If you wish to compile `ADDA` on Windows, we recommend using [MinGW](http://www.mingw.org/) (or [MinGW-w64](http://mingw-w64.sourceforge.net/)) environment and provide [detailed instructions](InstallingMinGW.md) for that. However, a simpler option is to use precompiled [executables for Windows](PackageDescription#Windows_executables.md), which are available for [downloading](Downloads.md).