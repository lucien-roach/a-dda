

If you plan to run `ADDA` on a cluster, MPI is probably already installed on your system. You should consult someone familiar with the particular MPI package. `ADDA` usage of MPI is based on the [MPI 2.1](http://www.mpi-forum.org) standard,  and it should work with any implementation that is compliant with this or higher versions of the standard. Version of MPI standard is checked for conformity by `ADDA` both during compilation and at runtime.

`ADDA` will run on any hardware compatible with MPI, but, in principle, it may run more efficiently on hardware with shared memory address space, e.g. multi-core PCs. However, such hardware also has its drawbacks, e.g. the performance may be limited by memory access speed. A few tests performed on different computers showed that currently using two cores of a dual-core PC results in computational time from 60% to 75% of that for sequential execution on the same machine. We plan to optimize `ADDA` specifically for such hardware, using e.g. [OpenMP](http://www.openmp.org/).

If you plan to run a parallel version of `ADDA` on a single computer using a multi-core processor, you need first to install some implementation of MPI. Installation instruction can be found in the manual of a particular MPI package. In the following we briefly describe examples of installation of [MPICH2](http://www.mcs.anl.gov/research/projects/mpich2/) on Unix and Windows. `MPICH2` has a lot of advanced features, like combining several single- and multi-core computers in a cluster. However, we do not discuss it here. These guidelines are based on `MPICH2` version 1.4.

# Unix #
In many popular Unix distributions, `MPICH2` can be installed through the package manager - see [the full list](http://www.mcs.anl.gov/research/projects/mpich2/downloads/index.php?s=downloads).

A detailed description for compiling `MPICH2` yourself are given in its [Installer's Guide](http://www.mcs.anl.gov/research/projects/mpich2/documentation/index.php?s=docs), see especially Section 2.2 "From A Standing Start to Running an MPI Program". Below we summarize the most important points.
  * [Download the source code](http://www.mcs.anl.gov/research/projects/mpich2/downloads/index.php?s=downloads) of `MPICH2` and unzip it.
  * If you are installing MPI under a user account, choose installation directory and create it, for instance:
```
mkdir $HOME/mpich2-install
```
  * Cd into the directory with `MPICH2` source (which we here use also as a build directory) and type
```
configure -prefix=$HOME/mpich2-install -enable-fast
```
> where `-prefix` specifies installation directory, and `-enable-fast` is recommended for production runs (at expense of some error reporting). Other useful configuration options to consider can be found in the [Installer's Guide](http://www.mcs.anl.gov/research/projects/mpich2/documentation/index.php?s=docs).
  * Type
```
make
make install
```
  * Add corresponding `bin` directory to the `$PATH`. For example, if you are using BASH, add
```
export PATH=$HOME/mpich2-install/bin:$PATH
```
> to your `.bash_profile` and relogin. Now you should be able to [compile MPI-version of ADDA](CompilingADDA.md) using `mpicc` (automatically detected by the `Makefile`) and run it using `mpiexec`.

# Windows #
`MPICH2` [Installer's Guide](http://www.mcs.anl.gov/research/projects/mpich2/documentation/index.php?s=docs) contains instructions for Windows - Section 9 "Windows Version". However, they seem somewhat incomplete in comparison with the following practical guide.
  * [Download the installer](http://www.mcs.anl.gov/research/projects/mpich2/downloads/index.php?s=downloads) of `MPICH2` for your combination of Windows and hardware and install it.
  * Add `bin` subdirectory to Windows environmental variable `PATH`. This will give you direct access to `mpiexec` and `smpd`.
  * On some systems (e.g. Vista) you may need to manually install `smpd` as a system service. For that open Windows console (`cmd`) with administrator's privileges (e.g. right-clicking on its icon and choosing corresponding option). Then run
```
smpd -install -phrase behappy
```
> The (pass)phrase should be the same for all computers running in a cluster. Specifying it at this moment will stop `mpiexec` from asking for it at every run.
  * First time you run `mpiexec` you may be prompted for Windows account name and password, which will be used to run MPI processes. These data is then stored encrypted in Windows registry. `MPICH2` can not use an account with empty password. So if that is the case for your account, you need to either add a password (you may then also [turn-on automatic login](http://www.howtogeek.com/howto/windows-vista/make-windows-vista-log-on-automatically/)) or create a new account specially to be used by `MPICH2`.
  * Moreover, you will be once prompted by the Windows firewall, whether to allow `mpiexec` and `smpd` to access network. If you plan to run `ADDA` only on a single PC, you may block them. Otherwise, you should unblock them. You may also get similar prompts for the parallel program itself, e.g., `adda_mpi`.

The above actions are necessary to use precompiled `adda_mpi`, included in [Windows packages](http://code.google.com/p/a-dda/wiki/PackageDescription#Windows_executables). If you want to compile MPI version of `ADDA` yourself, then additionally
  * Either
    * specify paths to `include` and `lib` subdirectories of `MPICH2` directory in file `src/mpi/Makefile`, as described in [CompilingADDA](CompilingADDA.md).
    * or copy contents of these folders to corresponding `include` and `lib` folders of the [MinGW/MSYS environment](InstallingMinGW#Advanced_options.md).
> This is required since Windows package of `MPICH2` does not contain compiler wrapper `mpicc`.