This list is far from being complete, therefore please send your questions to [adda-discuss@googlegroups.com](mailto:adda-discuss@googlegroups.com).



# General #

## What is the official name of the code? ##
It is `ADDA`. You may wonder why the corresponding Google Code identifier is "a-dda", as shown in address bar and in the top-left corner of the page. That is because "adda" was already occupied at Google Code by unrelated project, when we moved here.

## I am new to `ADDA` and want to simulate ... What should I start with? ##
  * Read GettingStarted and other pages it links to.
  * In particular, go through the [Tutorial](Tutorial.md) to understand basic `ADDA` functionality.
  * Scan through the [manual](http://a-dda.googlecode.com/svn/trunk/doc/manual.pdf) to understand where to get more details if needed.

## Do you have GUI/wrapper for `ADDA`? Can `ADDA` plot its results? ##
Development of `ADDA` is focused on its core functionality (that is efficient and large-scale simulations of interaction of electromagnetic waves with particles). Any kind of pre- and post-processing can be implemented by scripts or independent programs without changing the core code. Some examples can be found in [misc/](http://code.google.com/p/a-dda/source/browse/#svn%2Ftrunk%2Fmisc) folder ([short overview](PackageDescription.md)). Also a few [wrappers](https://code.google.com/p/a-dda/wiki/Links#Codes_that_use_ADDA) are available.

## I run `ADDA` and get strange output (different from my expectations). What should I do? ##
  * Make sure you are using the latest version of `ADDA`. Read carefully the relevant parts of the [manual](http://a-dda.googlecode.com/svn/trunk/doc/manual.pdf).
  * Check this FAQ.
  * If that does not help, you're encouraged to send your question to [adda-discuss@googlegroups.com](mailto:adda-discuss@googlegroups.com). Please try to make your question as specific as possible. If your question is based on a particular run, provide all relevant input and output files, at least `log`.

## I have found a bug in `ADDA`. What should I do? ##
  * Make sure you are using the latest version of `ADDA`. If you modified the code yourself, try the original release.
  * Look at the list of known bugs at the [issue tracker](http://code.google.com/p/a-dda/issues/list?q=type=Defect), maybe your bug is already known.
  * Try to understand how exactly the bug occurs, write down the sequence of actions that lead to it.
  * Please try to localize the bug, i.e. try to remove as much command line parameters as possible without removing the bug. Also try to use default versions of input files, which you have modified.
  * [Submit your bug](http://code.google.com/p/a-dda/issues/entry) to the issue tracker, including input files and `Makefile` that you used for compilation. Do not forget to include all the relevant output files, at least `log`. Please also include a brief description of your operation system and hardware. We will try to fix the bug as soon as possible.

# Defining scattering problem #

## How to simulate light scattering by a particle which shape can not be described by any of the `ADDA` predefined shapes? ##
The simplest is to specify your particle by a shape file. However, if your shape can easily be described in some parametric form and you are going to simulate a lot of particles with shapes from this class or if you think other users will probably simulate the same shapes, you are encouraged to [implement new shape inside ADDA](AddingShape.md).

To produce a shape file you can use any programming language/software. `ADDA` contains a couple of auxiliary packages to alleviate this problem:
  * Point-in-polyhedron (`misc/pip`, see [description](http://a-dda.googlecode.com/svn/trunk/misc/pip/README)) allows transformation of [.obj](http://en.wikipedia.org/wiki/Wavefront_.obj_file) format (and several other common 3D formats) into the format readable by `ADDA`. There is also [another code](http://nanohub.org/resources/ddaconvert/) (related) with similar functionality.
  * Hyperfun (`misc/hyperfun`, see [description](http://a-dda.googlecode.com/svn/trunk/misc/hyperfun/README)) provides a toolchain for working with [Hyperfun models](http://hyperfun.org). Part of this toolchain can be used to transform other shape formats, like [.wrl](http://en.wikipedia.org/wiki/VRML).

## How to visualize shape files of `ADDA`? ##
The most general solution is freely available software [LiteBil](http://albin.abo.fi/~jkniivil/litebil/). It allows both quick testing of shape files produced by `ADDA` or other software, and producing nice pictures for publication.

There are also tools developed by Roman Schuh, which are available upon request. See [this discussion](http://groups.google.com/group/adda-discuss/browse_thread/thread/c715bd07cc9274d8#) for details.

## How to simulate light scattering by a particle embedded in a homogeneous non-absorbing medium? ##
Just apply `ADDA` to an equivalent problem of a particle in vacuum with both wavelength and particle refractive index divided by the refractive index of the medium. Most of the calculated scattering quantities can then be used without modification. See section _Applicability of the DDA_ of the [manual](http://a-dda.googlecode.com/svn/trunk/doc/manual.pdf) for details.

## How to calculate scattering by a particle near or inside the infinite plane-parallel plane substrate? ##
Starting from v.1.3b4 `ADDA` can rigorously and efficiently handle particles near the plane homogeneous substrate. Possible approximate approaches for more complicated cases, like multi-layered substrates or particles crossing the substrate, are discussed in Section _Extensions of the DDA_ of the [manual](http://a-dda.googlecode.com/svn/trunk/doc/manual.pdf).

# Calculated quantities #

## How is the Mueller matrix, produced by `ADDA`, defined and/or normalized? ##
It is defined as in Bohren & Huffman "Absorption and scattering of Light by Small Particles" (1983), and it is not normalized. Some other codes may compute Stokes scattering matrix, which is normalized so that 1,1-element is equal to 1 after averaging over the whole solid angle. This matrix should be multiplied by _πC_<sub>sca</sub>/_λ_<sup>2</sup> to get Mueller matrix. _C_<sub>sca</sub> is the scattering cross section for unpolarized light, equal to average of scattering cross sections for any two perpendicular incident polarizations. See section _Mueller matrix_ of the [manual](http://a-dda.googlecode.com/svn/trunk/doc/manual.pdf) for more details.

## How to calculate near-field using `ADDA`? ##
We are currently working to implement this feature in a convenient manner ([issue 10](https://code.google.com/p/a-dda/issues/detail?id=10)). For now there are two possible workarounds:
  * (Recommended)  You may save dipole polarizations to file by `ADDA` and then use auxiliary package `misc/near_field` (see [description](http://a-dda.googlecode.com/svn/trunk/misc/near_field/nearfield_manual.txt)).
  * Or you may extend the computational box with "dummy" dipoles located at points where you wish to calculate near-field. Saving the internal fields by `ADDA` will then produce the desired result, but at expense of extra computational time. Please see the section _Near-field_ of the [manual](http://a-dda.googlecode.com/svn/trunk/doc/manual.pdf) for details.

## How to calculate the wavelength spectrum of extinction/scattering/etc.? ##
Currently there is no automatic way to do it, although we plan to implement it ([issue 35](https://code.google.com/p/a-dda/issues/detail?id=35)). Currently one may either perform separate simulation for each value of wavelength, or semi-automate this process by some kind of script or wrapper. The latter options are discussed in comments to [issue 35](https://code.google.com/p/a-dda/issues/detail?id=35).