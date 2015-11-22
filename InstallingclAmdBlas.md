

`clAmdBlas` library provides the optimized way to perform linear-algebra operations in OpenCL version of `ADDA`. Currently we are only testing its virtues, but it may become linked by default in the future. The installation and linking instructions are analogous to that of [clAmdFft](InstallingclAmdFft.md) (they are two parts of the same package). The following guidelines are based on `clAmdBlas` version 1.10.274 (or 321) and are less detailed than that for [clAmdFft](InstallingclAmdFft.md). So you are advised to consult the latter if having any problems.

# Unix #

  * [Download](http://developer.amd.com/tools-and-sdks/opencl-zone/opencl-libraries/amd-accelerated-parallel-processing-math-libraries/) the package of `clAmdBlas` for Linux and install it. The default installation directory is `/opt/clAmdBlas-<version>`. It is also possible to specify any other directory, for example, inside the home directory of a particular user.
  * Either
    * specify paths to `include` and `lib64` (or `lib32`) subdirectories of the install directory in file `src/ocl/Makefile`, as described in [CompilingADDA](CompilingADDA.md).
    * or add `.../include` to environmental variable `C_INCLUDE_PATH` and `.../lib64` (or `lib32`) - to `LIBRARY_PATH` and `LD_LIBRARY_PATH` (for linking and runtime). Setting `LD_LIBRARY_PATH` can be replaced by modifying `/etc/ld.so.conf` or by sourcing `appmlEnv.sh` in the installation directory.

# Windows #

If you just want to use [ADDA executables for Windows](PackageDescription#Windows_executables.md), do not worry about `clAmdBlas` at all. An appropriate DLL will be included in the corresponding package (once the executable will be linked to it).

If you want to compile `adda_ocl` on Windows yourself, proceed further:
  * [Download](http://developer.amd.com/tools/heterogeneous-computing/amd-accelerated-parallel-processing-math-libraries/) the installer of `clAmdBlas` for Windows and install it. The default installation directory is `C:\Program Files (x86)\AMD\clAmdBlas` or `C:\Program Files\AMD\clAmdBlas`.
  * Add `bin64` or `bin32` (depending on the Windows bitness) subdirectory in the installation directory to Windows environmental variable `PATH`
  * Either
    * specify paths to `include` and `bin64` (or `bin32`) subdirectories of the install directory in file `src/ocl/Makefile`, as described in [CompilingADDA](CompilingADDA.md). The compiler will link directly to `*.dll` (instead of `*.lib`).
    * or copy contents of `include` folder and `clAmdBlas.dll` of `bin` folder to corresponding `include` and `lib` folders of the [MinGW/MSYS environment](InstallingMinGW#Advanced_options.md).