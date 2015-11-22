`ADDA` is an open-source parallel implementation of the discrete dipole approximation, capable to simulate light scattering by particles of arbitrary shape and composition in a wide range of particle sizes. The following list of features corresponds to [the latest stable version](Downloads.md) (1.2).



# Parallel implementation #
Distinctive feature of `ADDA` is the ability to run a _single_ DDA simulation in parallel on a cluster or supercomputer, using [MPI](http://www.mpi-forum.org/). Computational grid of 1024×1024×1024 dipoles (1 Gdipole) and even larger can be used.
  * Very large particle sizes can be handled. For instance, light scattering by a sphere with size parameter 320 (diameter equal to 100 wavelengths) can be accurately simulated (see [Largest Simulations](LargestSimulations.md)).
  * Very fine discretization can be used when high accuracy or extra reliability is required.
  * Simple sequential execution is also possible, as well as running on a multi-core processors.
  * Another option is to use [OpenCL version of ADDA](OpenCL.md) to effectively employ modern GPUs.

# Arbitrary particle shape and composition #
  * 16 predefined particle shapes from a sphere to a red blood cell and two touching ellipsoids.
  * Homogeneous axisymmetric shape can be specified by an arbitrary contour in _xz_-plane.
  * Arbitrary 3D geometry can be specified by a shape file. Apart from its own format `ADDA` understands format of `DDSCAT`. Also converters from common 3D shape formats are available.
  * Any representation of the particle shape can be automatically refined, replacing each dipole by a cube _n×n×n_ of smaller dipoles.
  * Particle can be highly inhomogeneous, consisting of up to 255 homogeneous domains with different refractive indices.
  * Refractive index of each domain can be anisotropic, specified by a diagonal tensor.
  * Spherical granules can be automatically added to any of the particle domains (randomly uniformly distributed).
  * Outer medium can be either vacuum or non-absorbing homogeneous one.

# Full control over scattering geometry #
  * Orientation of the particle and/or direction of the incident beam can be varied.
  * Orientation averaging is possible (performed by an efficient algorithm based on Romberg integration).
  * Incident beam can be a plane wave, a (tightly-focused) Gaussian beam, or arbitrary one read from file.

# A wide variety of calculated quantities #
  * Scattering, absorption, and extinction cross sections.
  * Asymmetry parameter (or vector).
  * Angle-resolved amplitude and Mueller matrices. The latter can be automatically integrated over azimuthal angle.
  * Radiation forces - both total and for each single dipole.
  * Electric fields in and near the particle.

# Fast and accurate #
  * A large linear system is solved by an iterative solver. Matrix-vector product is computed using FFT-convolution method, based on the regular spacing of dipoles. Thus, simulation time is, to some extent, an order of number of dipoles.
  * By default [FFTW3 package](http://www.fftw.org/) is used to perform FFT, which automatically adapts to a particular hardware. A fall-back FFT implementation, not requiring any separate package installation, is also available.
  * Seven Krylov-subspace-type iterative solvers are available, four of them directly employ the complex-symmetric structure of the interaction matrix.
  * Modern DDA formulations, namely filtered coupled dipoles and integration of Green's tensor, are available. They may grossly outperform standard formulations (e.g. lattice dispersion relation) in certain cases.
  * Overall, the code is highly optimized both for speed and memory consumption. This was exemplified by [comparison with other DDA codes](ComparisonOtherCodes#Comparison_with_other_DDA_codes.md).
  * A special [sparse mode](SparseMode.md) is efficient for very porous particles.

# Easy to use #
  * Powerful command line interface.
  * Internal help system (`-h [...]` command line option).
  * A lot of information in the [wiki pages](GettingStarted.md) and [manual](http://a-dda.googlecode.com/svn/trunk/doc/manual.pdf).
  * Active [discussion group](http://groups.google.com/group/adda-discuss) to help users.
  * Checkpoint system to break and then continue long simulations.
  * Simulation results are saved into separate directory with automatically generated unique name. Thus, multiple parallel runs can be easily performed.
  * Many of the internally-used quantities can be saved to file to be further used by other programs. This includes particle shape, position of granules, values of incident beam and dipole polarizations at every dipole.
  * Easy to compile on almost any platform using [extensive instructions](CompilingADDA.md).
  * Ready-to-use [executables for Windows](PackageDescription#Windows_executables.md) are available.

# Reliable #
  * Tested and trusted by the community. As of summer 2013 it is used by about 60 researchers, who published [more than 100 journal papers](Publications.md), which somehow used `ADDA`.
  * Tested by [comparison with other methods and codes](ComparisonOtherCodes.md).
  * New releases are tested against the previous ones by automatic testing suites.
  * Open development process, including [issue tracker](http://code.google.com/p/a-dda/issues) and availability of the [current source](http://code.google.com/p/a-dda/source/browse/), allows pre-release testing.

# Actively developed #
  * Although `ADDA` is already a mature package, there are large [plans for further development](http://code.google.com/p/a-dda/issues).
  * [Development process](http://code.google.com/feeds/p/a-dda/issueupdates/basic) is ongoing albeit irregular. The goal is to make at least one release per year.
  * Number of [contributors](http://code.google.com/p/a-dda/people/list) is slowly increasing due to the open nature of Google Code platform.
  * New contributors are [welcome](InstructionCommitters.md)! In particular, there are extensive instructions, how to make simple changes to the code (e.g. [adding new predefined shape](AddingShape.md)).
