Here most important changes are listed grouped by releases in reverse chronological order. Full history with all hardly significant details can be found in [doc/history](http://a-dda.googlecode.com/svn/trunk/doc/history). Very early history starting from 1990 is [documented separately](EarlyHistory.md). Packages of versions starting from 0.73 are available in [Downloads](Downloads.md), while earlier versions can be extracted from the [Subversion repository](http://code.google.com/p/a-dda/source/browse/tags/).



# Ver. 1.2 - 08.06.13 #

  * [Sparse mode](SparseMode.md) is now fully integrated into `ADDA` and significantly improved ([issue 98](https://code.google.com/p/a-dda/issues/detail?id=98)). It is designed for very porous particles.
  * Linked to new OpenCL FFT library from AMD ([clAmdFft](InstallingclAmdFft.md)), it is now the default one in OpenCL mode ([issue 144](https://code.google.com/p/a-dda/issues/detail?id=144)). Speed improvement is a few times depending on the GPU. But there are some limitations ([issue 157](https://code.google.com/p/a-dda/issues/detail?id=157)).
  * New iterative solver `BCGS2` - an improved version of `BICGSTAB` ([issue 24](https://code.google.com/p/a-dda/issues/detail?id=24)).
  * New command line option argument `-init_field read <filenameY> [<filenameX>]` to initialize an iterative solver by the field read from file ([issue 2](https://code.google.com/p/a-dda/issues/detail?id=2)).
  * Behavior of WKB initial field was slightly improved. Before WKB result was assigned to **E**<sub>exc</sub>, now it is assigned to **E** (similar to the behavior of `-init_field read`).
  * Subversion revision number was added to the output of `-V` command line option ([issue 125](https://code.google.com/p/a-dda/issues/detail?id=125)).
  * Timing for generation of incident beam was added.
  * Memory usage during prognosis was slightly reduced.
  * Finally removed deprecated options `-Cpr_mat` and `-sg_format ddscat`.
  * Fixed inaccuracy in `input/scat_params.dat` ([r1159](https://code.google.com/p/a-dda/source/detail?r=1159)).
  * A few minor bugs were fixed ([issue 83](https://code.google.com/p/a-dda/issues/detail?id=83), [issue 159](https://code.google.com/p/a-dda/issues/detail?id=159), [issue 162](https://code.google.com/p/a-dda/issues/detail?id=162)).
  * Several improvements of tests/2exec to accomodate OpenCL and sparse modes, as well as comparison of different FFT implementations ([r1192](https://code.google.com/p/a-dda/source/detail?r=1192), [r1200](https://code.google.com/p/a-dda/source/detail?r=1200), [r1203](https://code.google.com/p/a-dda/source/detail?r=1203)).
  * Added wiki pages: SparseMode, InstallingclAmdFft, CodeDesign ([issue 75](https://code.google.com/p/a-dda/issues/detail?id=75)), and CodeStyleGuide ([issue 74](https://code.google.com/p/a-dda/issues/detail?id=74)).
  * Added new folder `devtools`. It contains a couple of text files, bash scripts for preparing files for release and other tasks.
  * Added [doxygen](http://www.doxygen.org) config file (`doc/doxy/Doxyfile`) for automatic generation of extensive docs and [GraphViz](http://www.graphviz.org/) files (`doc/gv/`) with schemes describing `ADDA` design (for `svg` files in CodeDesign).
  * Several improvements to `Makefile`:
    * Improved the automatic dependency generation - now it is faster and more robust.
    * Full support for compilation in 32-bit mode on 64-bit OS, using `EXTRA_FLAGS=-m32`, also for `misc/` packages.
    * New `make` targets: `cleanfull`, `cleanruns`, `cleanrunsseq`, `cleanrunsmpi`, `cleanrunsocl`. They enable cleaning output from `ADDA` runs.

# Ver. 1.1 - 05.06.12 #

  * New [OpenCL-based](OpenCL.md) version of `ADDA` to effectively use GPUs for simulations ([issue 118](https://code.google.com/p/a-dda/issues/detail?id=118)).
  * Calculation of radiation forces was largely improved, fixing several bugs. Now it is well-tested with only two remaining limitations ([issue 14](https://code.google.com/p/a-dda/issues/detail?id=14), [issue 135](https://code.google.com/p/a-dda/issues/detail?id=135)). Connected changes:
    * Output files `VisFrp-?.dat` were renamed into `RadForce-?`.
    * Command line option `-Cpr_mat` was renamed into `-Cpr`. The former was marked deprecated.
  * Three new `misc/` packages. In particular, it greatly simplifies using many common 3D shape formats together with `ADDA` ([issue 34](https://code.google.com/p/a-dda/issues/detail?id=34)).
    * [hyperfun](http://a-dda.googlecode.com/svn/trunk/misc/hyperfun/README), contributed by Josef Gasteiger.
    * [pip](http://a-dda.googlecode.com/svn/trunk/misc/pip/README), contributed by Roman Schuh and Thomas Wriedt.
    * [parse\_log](http://a-dda.googlecode.com/svn/trunk/misc/parse_log/README) ([issue 82](https://code.google.com/p/a-dda/issues/detail?id=82)).
  * Added support for `DDSCAT7` shape format, the previous one was labeled `DDSCAT6`. Command line option '`-sg_format ...`' was updated accordingly ([issue 105](https://code.google.com/p/a-dda/issues/detail?id=105)).
  * New predefined shapes: `chebyshev`, `plate`, and `prism`.
  * New command line option to read incident beam from file '`-beam read <filenameY> [<filenameX>]`' ([issue 2](https://code.google.com/p/a-dda/issues/detail?id=2)).
  * New polarizability formulations: DGF and LAK ([issue 13](https://code.google.com/p/a-dda/issues/detail?id=13)).
  * New formulation IGT\_SO, which is a much faster version of IGT. It applies to polarizability, interaction term, and scattering quantities ([issue 84](https://code.google.com/p/a-dda/issues/detail?id=84)).
  * New iterative solvers: CSYM and QMR2 ([issue 24](https://code.google.com/p/a-dda/issues/detail?id=24)).
  * New command line option to choose how to initialize the iterative solver: '`-init_field {auto|zero|inc|wkb}`'. First implementation of the WKB incident field is done ([issue 59](https://code.google.com/p/a-dda/issues/detail?id=59)).
  * New command line option `-recalc_resid` to directly recalculate residual at the end of the iterative solver ([issue 25](https://code.google.com/p/a-dda/issues/detail?id=25)).
  * Updated the default build platform to gcc 4 (tested with 4.5.2). Many connected changes, in particular [issue 122](https://code.google.com/p/a-dda/issues/detail?id=122). Speed of Windows executables increased by about 10-20%.
  * `ADDA` executables for 64-bit Windows are now available ([issue 66](https://code.google.com/p/a-dda/issues/detail?id=66)).
  * Windows executables for routines from `misc/` are now available, both 32- and 64-bit ([issue 104](https://code.google.com/p/a-dda/issues/detail?id=104)).
  * Added automatic tests of `ADDA` executables to `/tests/2exec` ([issue 6](https://code.google.com/p/a-dda/issues/detail?id=6)).
  * Minor bugs were fixed ([issue 123](https://code.google.com/p/a-dda/issues/detail?id=123), [issue 124](https://code.google.com/p/a-dda/issues/detail?id=124), [issue 128](https://code.google.com/p/a-dda/issues/detail?id=128), [issue 136](https://code.google.com/p/a-dda/issues/detail?id=136), [issue 139](https://code.google.com/p/a-dda/issues/detail?id=139), [issue 146](https://code.google.com/p/a-dda/issues/detail?id=146)).
  * Minor bug in handling input shape filename was fixed ([r1141](https://code.google.com/p/a-dda/source/detail?r=1141)).
  * Several wiki pages were added: AddingShapeFileFormat, [Awards](Awards.md), [InstallingMinGW](InstallingMinGW.md), InstructionCommitters, LargestSimulations, [OpenCL](OpenCL.md), [Publications](Publications.md), and [References](References.md) ([issue 117](https://code.google.com/p/a-dda/issues/detail?id=117)).
  * Deprecated command line option `-prognose` was finally removed.
  * Automatic names for output directory were slightly changed ([r1106](https://code.google.com/p/a-dda/source/detail?r=1106)).
  * Two new development branches (try them at your own risk): weighted discretization ([issue 12](https://code.google.com/p/a-dda/issues/detail?id=12)) and sparse (non-FFT) DDA ([issue 98](https://code.google.com/p/a-dda/issues/detail?id=98)).

# Ver. 1.0 - 10.09.10 #
  * Fabio Della Sala and Stefania D'Agostino contributed code to calculate near-field with `ADDA` ([misc/near\_field](http://code.google.com/p/a-dda/source/browse/trunk/misc/near_field)). Now it is included in the main package.
  * "Integration of Green's tensor" formulation of the DDA was implemented based on the code contributed by Patrick Chaumet and Adel Rahmani (part of [issue 84](https://code.google.com/p/a-dda/issues/detail?id=84)).
  * New predefined shapes: `bisphere`, `bicoated`, and `biellipsoid`.
  * New command line option `-scat_matr {muel|ampl|both|none}` to specify which scattering matrix (amplitude and/or Mueller) need to be stored to file ([issue 7](https://code.google.com/p/a-dda/issues/detail?id=7)).
  * Calculation of Green's tensor was accelerated by doing all 6 components at once ([issue 58](https://code.google.com/p/a-dda/issues/detail?id=58)).
  * The code was refactored (see [current state](PackageDescription.md)). Makefiles were significantly improved (see [compilation instructions](CompilingADDA.md)). In particular, [issue 65](https://code.google.com/p/a-dda/issues/detail?id=65) and [issue 81](https://code.google.com/p/a-dda/issues/detail?id=81) were fixed.
  * [Homepage at Google Code](http://code.google.com/p/a-dda/) was completely rewritten. Now it is is the main official page of `ADDA`.
  * Documentation was significantly reworked. Many sections of the [manual](http://a-dda.googlecode.com/svn/trunk/doc/manual.pdf), not directly related to performing light scattering simulations were moved to wiki pages. A number of other [wiki pages](GettingStarted.md) were added. `doc/faq` was moved to a wiki page [FAQ](FAQ.md).
  * Several bugs in granule generator were fixed ([issue 108](https://code.google.com/p/a-dda/issues/detail?id=108), [issue 109](https://code.google.com/p/a-dda/issues/detail?id=109)).
  * Fixed bug in calculation of radiative forces ([r972](https://code.google.com/p/a-dda/source/detail?r=972)).
  * Minor bugs were fixed ([issue 93](https://code.google.com/p/a-dda/issues/detail?id=93), [issue 96](https://code.google.com/p/a-dda/issues/detail?id=96), [issue 97](https://code.google.com/p/a-dda/issues/detail?id=97), [issue 107](https://code.google.com/p/a-dda/issues/detail?id=107),  [issue 110](https://code.google.com/p/a-dda/issues/detail?id=110)).

# Ver. 0.79 - 29.05.09 #
  * Added using SLURM job ID, when available.
  * Saving of granule coordinates to a file was implemented.
  * The code was tested for 64bit, implementing a number of corrections/improvements.
  * New shape `axisymmetric` was added. The internals of shape generation routines were adjusted so that shape can now define the absolute size of the particle.
  * A new option `'-scat fin'` was implemented, which uses finite dipole correction for calculation of _C_<sub>abs</sub> and _C_<sub>ext</sub>.
  * Breakdown detection of iterative solvers was improved.
  * `-prognose` command line option was replaced by `-prognosis` (correct  spelling). The former is still operational but marked as deprecated.
  * The code was transformed to C99 standard without using its parts, which are not yet widely supported by compilers (such as inline and complex). Explicit support of this standard is required during compilation. Some style changes were done, and some work in this direction is still due.
  * Affiliation of one of the author (Maxim Yurkin) has changed to Institute of Chemical Kinetics & Combustion (Novosibirsk, Russia). Full name of the package was changed from 'Amsterdam DDA' to `ADDA`. This does not change the global strategy of `ADDA` development.
  * Code repository has been [moved to Google Code](http://a-dda.googlecode.com). Google groups [adda-announce](http://groups.google.com/group/adda-announce), [adda-discuss](http://groups.google.com/group/adda-discuss) and [adda-develop](http://groups.google.com/group/adda-develop) were created to facilitate interaction with users. This replaces the previous mailing list.
  * Copyright was updated to [GPL 3](http://www.gnu.org/copyleft/gpl.html).
  * To-do list was replaced by [issue tracker](http://code.google.com/p/a-dda/issues).
  * Additions to the manual: short history of earlier `ADDA` development, description how to add new beam type and new command line options, a small summary of the comparison of the DDA with the FDTD, discussion of difference between two ways to calculate _C_<sub>sca</sub>, possibilities to improve the accuracy of these calculations, and connection between differential cross section and the Mueller matrix.
  * Information on how to cite the manual was added to its first page. It includes a direct link to the particular version of the manual on Google code.
  * Manual license is now explicitly set to [Creative Commons Attribution 3.0 license](http://creativecommons.org/licenses/by/3.0/).
  * Several minor bugs were fixed.

# Ver. 0.78.2 - 11.04.08 #
  * An implementation of a new FCD polarization prescription (`-pol fcd`) was somewhat faulty. Correction terms of orders (_kd_)<sup>2</sup> and ln(...)(_kd_)<sup>3</sup> were interchanged. Fixed.
  * Critical bug in `cDiv` and `cDivSelf` functions in source file [cmplx.h](http://code.google.com/p/a-dda/source/browse/trunk/src/cmplx.h) was fixed (introduced in version 0.78). This bug made BiCGSTAB and BiCG iterative solvers to fail. Moreover, even when other two iterative solvers were used, erroneous results were calculated for certain values of refractive index.

# Ver. 0.78 - 19.03.08 #
  * Name of MPI executable is now `adda_mpi`; this allows automatic compilation of both sequential and MPI versions from the same source folder.
  * Parallel executable for Win32, compiled with MPICH 2, is included in the package.
  * Filtered coupled dipoles formulation was implemented (both full and quasistatic version). New arguments `fcd` and `fcd_st` for interaction term (`-int` command line option) and `fcd` for polarization term (`-pol` command line option). It greatly improves DDA performance for large refractive indices.
  * To-do list added to the package: `doc/todo.xls` and `todo.pdf`.
  * Hands-on tutorial was added to the manual. It is based on the one carried out during the DDA Workshop in Bremen.
  * New command line option `'-opt {speed|mem}'` was added. It allows choosing whether `ADDA` will optimize itself for maximum speed or for minimum memory usage.
  * Calculation of scattered fields is accelerated by more than 4 times (compiled with `gcc`). Thanks to R. Scott Brock for this idea. This may also significantly accelerate orientation averaging, depending on the problem.
  * New command line option `'-eq_rad <arg>'` was added, it allows one to specify volume-equivalent radius of a scatterer.
  * Shapes `box` and `spherebox` no longer depend on the second and third argument of `-grid` option.
  * Shape `box` now accepts two optional arguments: y/x and z/x aspect ratios, defining a rectangular parallelepiped.
  * Automatic line wrapping was added for most output to stdout and stderr. `ADDA` tries to get terminal width from `COLUMNS` environmental variables and, if fails, uses the default value.
  * Two new shapes were added: `capsule` and `egg`. They were implemented by Daniel Hahn and Richard I. Joseph.
  * Now, if grid size is auto-initialized from default dpl, `ADDA` ensures that it is not smaller than 16.
  * Limitation on positiveness of dipole coordinates in shape file was removed. Now `ADDA` automatically determines the minimum box around the particle and centers it as usual.
  * Orientation averaging was optimized. If _α_ is varied in a full range [0°,360°), only one value of _γ_ is simulated for _β_=0° and 180°, saving a few evaluations of internal fields. Thanks to Antti Penttila for this idea.
  * Limited two-way compatibility with DDSCAT 6.1 geometry format was added. `ADDA` automatically detects DDSCAT format during reading of dipole file. When saving geometry to the file, the format is determined by new command line option `'-sg_format {text|text_ext|ddscat}'`.
  * Several minor bugs were fixed.

# Ver. 0.77 - 05.06.07 #
  * The bug was fixed that crashed the calculation of radiation forces.
  * Handling of large integers was improved throughout the program. Now it should work for any problem that fits into memory. Checks of integer overflow were added where necessary to avoid crashes.
  * Robustness of handling the command line and input files was improved.
  * Makefiles were improved, in particular, an option was added to use Intel compilers without static linking.
  * Command line option `-store_dip_pol` was added to save dipole polarizations to file.
  * The breakdown detection of the iterative solvers was improved. Now it should be much more sensitive. Thanks to Sorin Pulbere for reporting a test problem.
  * A minor bug in Romberg integration, reported by Antti Penttila, was fixed.
  * Locking of files was made more flexible. A compile option was added to independently turn off the advanced file locking.
  * Manual was significantly improved. "DDA formulation" section was rewritten to be self-contained, sections "Near-field" and "Comparison with other DDA codes" were added. Sections on compiling and running the code were extended to discuss in detail multi-core PCs. Sections "Applicability of DDA" and "System requirements" were extended to include recent benchmark results and discussion. Thanks to Vitezslav Karasek and Liviu Clime for their feedback.

# Ver. 0.76 - 14.10.06 #

  * Makefiles modified to include Compaq compiler. New variables added to set path for `FFTW3` header and library.
  * New option `-V` added to show version, compiler used, and copyright information.
  * Robustness of memory handling improved.
  * Timing is now produced even when `-prognose` option is used. Communication times are shown only in parallel mode, they are now more accurate.
  * All timing in parallel mode is now done through `MPI_Wtime`, which measures wall time.
  * Memory requirement in the manual are slightly corrected. The values produced by the program itself were correct.
  * New option `-granul` to granulate one of the domains.
  * Orientation averaging did not work in parallel (reported by Antti Penttila). Fixed.
  * Symmetries were not switched off by non-symmetric Gaussian beams. Fixed.
  * Computation of scattering of a tightly focused Gaussian beam was compared to multiple multipole method (data by Roman Schuh and Thomas Wriedt). Results of the comparison show perfect agreement.
  * New chapter in the manual to facilitate adding a new predefined shape.
  * New file `doc/faq` added for frequently asked questions.
  * New option `-store_beam` to save incident electric fields to a file.
  * Starting from this release `ADDA` executable for 32 bit Windows is available. It is compiled with `MinGW` and distributed with `FFTW3` dynamic link libraries. Thanks to Alexander Shvalov for information on `MinGW`.
  * Added support for anisotropic refractive index (option `-anisotr`). Currently it is limited to refractive index tensors that are diagonal in the particle reference frame. Thanks to Michiel Min for fruitful discussions.

# Ver. 0.75 - 24.05.06 #
  * All `system` calls were removed to improve robustness under certain MPI implementations.
  * Possible size of FFT-grid (and non-weird number of processors) has been extended to full range of `FFTW`. One possible divisor of either 11 or 13 has been added.
  * Robustness of makefiles with Intel compilers 8.1 and 9.0 in conjunction with MPICH has been increased.
  * Sample script for Sun Grid Engine has been added: `sample/test.sge`.
  * Different descriptions of Gaussian beam are now fully operational.
  * Origin is now always in the center of the computational box (before it was not so when _jagged_>1).
  * When incident field was used as an initial vector of the iterative solver, more iterations than necessary to reach epsilon were performed. Fixed.
  * Progress reported in the first iteration after checkpoint loading was incorrect in certain cases. To fix it required loss of checkpoint compatibility with earlier versions. More information is now displayed after checkpoint loading.
  * Robustness of generation of error messages in parallel mode has been increased.
  * Calculation of _C_<sub>sca</sub> or **g** caused both polarizations to be calculated even for particles that are symmetric for 90 degree rotation over the _z_-axis. That is not necessary - fixed.
  * Integration module has been significantly improved. It automatically uses the trapezoid rule for periodic function. Error estimates now seem to be reliable. Format of parameters for integration module has been changed because of the improvement.
  * [Homepage of ADDA](http://www.science.uva.nl/research/scs/Software/adda/) has been created.

# Ver. 0.74.2 - 05.04.06 #
  * Makefiles has been improved to allow simple usage of Intel compilers.
  * A section describing `FFTW3` installation has been added to the manual.

# Ver. 0.74 - 29.03.06 #
  * `buggy` beam type has been removed (obsolete).
  * Command line options `-nosym` and `-sym_enf` has been combined into `'-sym {no|enf}'`.
  * Internal fields are now automatically collected into one file, when running in parallel mode (named `IntField-X` and `IntField-Y`)
  * MPI-specific command line options has been enabled.
  * Geometry filename given to the command line option `-save_geom` should now be specified relative to the output directory.
  * In parallel mode each processor (except root) produces a separate logfile, if any errors or warnings were detected (on this specific processor).
  * Overall robustness of the code is improved (added many consistency checks).
  * Help system has been added (new command line option `-h`).

# Ver. 0.73 - 02.03.06 #
  * First stable release that has a manual and other documentation