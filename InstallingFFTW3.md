

The following guidelines are bases on `FFTW` version 3.2.2.

# Unix #

The installation of [FFTW 3](http://www.fftw.org) on any Unix system is straightforward and therefore is _highly recommended_, since it greatly improves the performance of `ADDA`. The easiest is to [install FFTW 3 for the entire system](http://www.fftw.org/fftw3_doc/Installation-on-Unix.html), using `root` account. Moreover, it is available as a standard package in many Unix/Linux distributions (be sure to also install development package, which includes headers).

If `root` is unavailable, `FFTW3` can also be installed under any user account as follows:
  * Download the [latest version of FFTW 3](http://www.fftw.org/download.html)
  * Unpack it, cd into its directory, and type
```
./configure --prefix=$HOME [--enable-sse2]
```
> where `'--enable'` option allows effective use of modern processors. You may also look through the [whole list of configuration options](http://www.fftw.org/fftw3_doc/Installation-on-Unix.html). Then type
```
make
make install
```
  * Modify the initialization of internal variables `FFTW3_INC_PATH` and `FFTW3_LIB_PATH` in the `Makefile`, as described in [CompilingADDA](CompilingADDA.md).

[Installation of FFTW 3 on non-Unix systems](http://www.fftw.org/fftw3_doc/Installation-on-non_002dUnix-systems.html) is slightly more complicated.

# Windows #
If you just want to use [ADDA executables for Windows](PackageDescription#Windows_executables.md), do not worry about `FFTW3` at all. An appropriate DLL is included in the corresponding package.

If you want to compile `ADDA` on Windows yourself, [install FFTW3 on Windows](http://www.fftw.org/install/windows.html). Although building `FFTW3` from source is possible, a recommended simpler way is the following:
  * Download a [precompiled package](http://www.fftw.org/install/windows.html) (32 or 64-bit), containing DLLs and header files. It contains a lot of files, some of which are needed for compilation in, e.g., Microsoft Visual Studio. However, we discuss below compilation by GNU compilers using the [MinGW environment](InstallingMinGW.md). For that you need only `fftw3.h` and `libfftw3-3.dll`.
  * Create a copy of `libfftw3-3.dll` and rename it to `libfftw3.dll`. On Windows Vista and newer you may instead create a symbolic link (administrator privileges may be required for that)
```
mklink libfftw3.dll libfftw3-3.dll
```
> This is required because the `FFTW3` DLL package do not contain appropriate import-library file to use during linking. The DLL itself can be used for that, but it needs to be named a little bit differently to be automatically found by the linker.
  * Either
    * modify the initialization of internal variables `FFTW3_INC_PATH` and `FFTW3_LIB_PATH` in the `Makefile` to point to the directory(ies) contating `fftw3.h` and `libfftw3.dll` respectively (see [CompilingADDA](CompilingADDA.md)).
    * or move these files to corresponding `include` and `lib` folders of the [MinGW/MSYS environment](InstallingMinGW#Advanced_options.md).
  * To avoid need to move `libfftw3-3.dll` together with `ADDA` executables, either
    * add folder with `libfftw3-3.dll` to Windows environmental variable `PATH`
    * or move this file to a folder already included in `PATH`, e.g., `bin` subfolder of the [MinGW installation directory](InstallingMinGW#Basic_installation.md). If you used `mklink` option above, make sure that the link points to the final location of `libfftw3-3.dll`.