Development of `ADDA` is mostly performed in the framework of scientific projects. In the following, a list of publications, relevant to different parts of `ADDA`, is presented. Although the manual provides a brief description of all `ADDA` functionality, we recommend citing the original papers listed below (containing more details) when you use the corresponding functions of `ADDA`. Please cite the manual only for those parts which are not published elsewhere.



# General description #

M. A. Yurkin and A. G. Hoekstra,“The discrete-dipole-approximation code ADDA: capabilities and known limitations,” [J. Quant. Spectrosc. Radiat. 112, 2234–2247 (2011)](http://dx.doi.org/10.1016/j.jqsrt.2011.01.031).

# Specific aspects #
## OpenCL (GPU-accelerated) version ##
M. Huntemann, G. Heygster, and G. Hong, “Discrete dipole approximation simulations on GPUs using OpenCL - Application on cloud ice particles,” [J. Comput. Sci. 2, 262–271 (2011)](http://dx.doi.org/10.1016/j.jocs.2011.05.011).

## Sparse (non-FFT) mode ##
J. Leinonen, D. Moisseev, and T. Nousiainen, “Linking snowflake microstructure to multi-frequency radar observations,” [J. Geophys. Res.: Atmos. 118, 3259–3270 (2013)](http://dx.doi.org/10.1002/jgrd.50163).

## Different implemented DDA formulations ##
### Filtered coupled dipoles (FCD) ###
  * N. B. Piller and O. J. F. Martin, “Increasing the performance of the coupled-dipole approximation: A spectral approach,” [IEEE Trans. Antennas Propag. 46, 1126–1137 (1998)](http://dx.doi.org/10.1109/8.718567).
  * M. A. Yurkin, M. Min, and A. G. Hoekstra, “Application of the discrete dipole approximation to very large refractive indices: Filtered coupled dipoles revived,” [Phys. Rev. E 82, 036703 (2010)](http://dx.doi.org/10.1103/PhysRevE.82.036703).

### Integration of Green's tensor (IGT) ###
P. C. Chaumet, A. Sentenac, and A. Rahmani, “Coupled dipole method for scatterers with large permittivity,” [Phys. Rev. E 70, 036606 (2004)](http://dx.doi.org/10.1103/PhysRevE.70.036606).

## Particles much larger than the wavelength ##
M. A. Yurkin, V. P. Maltsev, and A. G. Hoekstra, “The discrete dipole approximation for simulation of light scattering by particles much larger than the wavelength,” [J. Quant. Spectrosc. Radiat. Transfer 106, 546–557 (2007)](http://dx.doi.org/10.1016/j.jqsrt.2007.01.033).

## Gold nanoparticles ##
M. A. Yurkin, D. de Kanter, and A. G. Hoekstra, “Accuracy of the discrete dipole approximation for simulation of optical properties of gold nanoparticles,” [J. Nanophoton. 4, 041585 (2010)](http://dx.doi.org/10.1117/1.3335329).

## Surface mode ##
  * (2D-FFT implementation) R. Schmehl, B. M. Nebeker, and E. D. Hirleman, “Discrete-dipole approximation for scattering by features on surfaces by means of a two-dimensional fast Fourier transform technique,” [J. Opt. Soc. Am. A 14, 3026–3036 (1997)](http://dx.doi.org/10.1364/JOSAA.14.003026).
  * (full 3D-FFT acceleration) to be published.

## Shapes ##
### Axisymmetric ###
K. V. Gilev, E. Eremina, M. A. Yurkin, and V. P. Maltsev, “Comparison of the discrete dipole approximation and the discrete source method for simulation of light scattering by red blood cells,” [Opt. Express 18, 5681–5690 (2010)](http://dx.doi.org/10.1364/OE.18.005681).

### Bisphere ###
K. Schmidt, M. A. Yurkin, and M. Kahnert, “A case study on the reciprocity in light scattering computations,” [Opt. Express 20, 23253–23274 (2012)](http://dx.doi.org/10.1364/OE.20.023253).

### Capsule ###
D. V. Hahn, D. Limsui, R. I. Joseph, K. C. Baldwin, N. T. Boggs, A. K. Carr, C. C. Carter, T. S. Han, and M. E. Thomas, “Shape characteristics of biological spores,” [SPIE Proc. 6954, 69540W (2008)](http://dx.doi.org/10.1117/12.777637).

### Сhebyshev particle ###
K. Schmidt, M. A. Yurkin, and M. Kahnert, “A case study on the reciprocity in light scattering computations,” [Opt. Express 20, 23253–23274 (2012)](http://dx.doi.org/10.1364/OE.20.023253).

### Coated spheres ###
J. Tyynelä, T. Nousiainen, S. Göke, and K. Muinonen, “Modeling C-band single scattering properties of hydrometeors using discrete-dipole approximation and T-matrix method,” [J. Quant. Spectrosc. Radiat. Transfer 110, 1654–1664 (2009)](http://dx.doi.org/10.1016/j.jqsrt.2009.02.020).

### Cylinder ###
  * K. Schmidt, M. A. Yurkin, and M. Kahnert, “A case study on the reciprocity in light scattering computations,” [Opt. Express 20, 23253–23274 (2012)](http://dx.doi.org/10.1364/OE.20.023253).
  * L. Bi, P. Yang, and G. W. Kattawar, “Edge-effect contribution to the extinction of light by dielectric disks and cylindrical particles,” [Appl. Opt. 49, 4641–4646 (2010)](http://dx.doi.org/10.1364/AO.49.004641).

### Cube (box) ###
M. A. Yurkin and M. Kahnert, “Light scattering by a cube: accuracy limits of the discrete dipole approximation and the T-matrix method,” [J. Quant. Spectrosc. Radiat. Transfer 123, 176–183 (2013)](http://dx.doi.org/10.1016/j.jqsrt.2012.10.001).

### Egg ###
D. V. Hahn, D. Limsui, R. I. Joseph, K. C. Baldwin, N. T. Boggs, A. K. Carr, C. C. Carter, T. S. Han, and M. E. Thomas, “Shape characteristics of biological spores,” [SPIE Proc. 6954, 69540W (2008)](http://dx.doi.org/10.1117/12.777637).

### Ellipsoid ###
L. Bi, P. Yang, G. W. Kattawar, and R. Kahn, “Single-scattering properties of triaxial ellipsoidal particles for a size parameter range from the Rayleigh to geometric-optics regimes,” [Appl. Opt. 48, 114–126 (2009)](http://dx.doi.org/10.1364/AO.48.000114).

### Red blood cell (RBC) ###
M. A. Yurkin, “Discrete dipole simulations of light scattering by blood cells,” [PhD thesis, University of Amsterdam (2007)](http://www.science.uva.nl/research/scs/papers/archive/Yurkin2007c.pdf).

### Granule generator ###
M. A. Yurkin, K. A. Semyanov, V. P. Maltsev, and A. G. Hoekstra, “Discrimination of granulocyte subtypes from light scattering: theoretical analysis using a granulated sphere model,” [Opt. Express 15, 16561–16580 (2007)](http://dx.doi.org/10.1364/OE.15.016561).

## Calculated quantities ##
### Radiation forces ###
A. G. Hoekstra, M. Frijlink, L. B. F. M. Waters, and P. M. A. Sloot, “Radiation forces in the discrete-dipole approximation,” [J. Opt. Soc. Am. A 18, 1944–1953 (2001)](http://dx.doi.org/10.1364/JOSAA.18.001944).

### Internal fields ###
A. G. Hoekstra, J. Rahola, and P. M. A. Sloot, “Accuracy of internal fields in volume integral equation simulations of light scattering,” [Appl. Opt. 37, 8482–8497 (1998)](http://dx.doi.org/10.1364/AO.37.008482).

### Mueller matrix integrated over the azimuthal angle ###
M. A. Yurkin, “Symmetry relations for the Mueller scattering matrix integrated over the azimuthal angle,” [J. Quant. Spectrosc. Radiat. Transfer 131, 82–87 (2013)](http://dx.doi.org/10.1016/j.jqsrt.2012.11.023).

### Decay rate (emission) enhancement ###
S. D’Agostino, F. D. Sala, and L. C. Andreani, “Dipole decay rates engineering via silver nanocones,” [Plasmonics 8, 1079–1086 (2013)](http://dx.doi.org/10.1007/s11468-013-9512-3).

## Comparisons ##
### with other DDA codes ###
A. Penttila, E. Zubko, K. Lumme, K. Muinonen, M. A. Yurkin, B. T. Draine, J. Rahola, A. G. Hoekstra, and Y. Shkuratov, “Comparison between discrete dipole implementations and exact techniques,” [J. Quant. Spectrosc. Radiat. Transfer 106, 417–436 (2007)](http://dx.doi.org/10.1016/j.jqsrt.2007.01.026).

### with other light-scattering methods ###
  * M. A. Yurkin, A. G. Hoekstra, R. S. Brock, and J. Q. Lu, “Systematic comparison of the discrete dipole approximation and the finite difference time domain method for large dielectric scatterers,” [Opt. Express 15, 17902–17911 (2007)](http://dx.doi.org/10.1364/OE.15.017902).
  * K. V. Gilev, E. Eremina, M. A. Yurkin, and V. P. Maltsev, “Comparison of the discrete dipole approximation and the discrete source method for simulation of light scattering by red blood cells,” [Opt. Express 18, 5681–5690 (2010)](http://dx.doi.org/10.1364/OE.18.005681).
  * C. Liu, L. Bi, R. L. Panetta, P. Yang, and M. A. Yurkin, “Comparison between the pseudo-spectral time domain method and the discrete dipole approximation for light scattering simulations,” [Opt. Express 20, 16763–16776 (2012)](http://dx.doi.org/10.1364/OE.20.016763).
  * M. A. Yurkin and M. Kahnert, “Light scattering by a cube: accuracy limits of the discrete dipole approximation and the T-matrix method,” [J. Quant. Spectrosc. Radiat. Transfer 123, 176–183 (2013)](http://dx.doi.org/10.1016/j.jqsrt.2012.10.001).

# Additional packages #
## hyperfun ##
J. Gasteiger, M. Wiegner, S. Groß, V. Freudenthaler, C. Toledano, M. Tesche, and K. Kandler, “Modelling lidar-relevant optical properties of complex mineral dust aerosols,” [Tellus B 63, 725–741 (2011)](http://dx.doi.org/10.1111/j.1600-0889.2011.00559.x).

## near\_field ##
S. D'Agostino, P. P. Pompa, R. Chiuri, R. J. Phaneuf, D. G. Britti, R. Rinaldi, R. Cingolani, and F. Della Sala, “Enhanced fluorescence by metal nanospheres on metal substrates,” [Opt. Lett. 34, 2381–2383 (2009)](http://dx.doi.org/10.1364/OL.34.002381).

## pip ##
R. Schuh, “Arbitrary particle shape modeling in DDSCAT and validation of simulation results,” in [Proceedings of the DDA-Workshop](http://diogenes.iwt.uni-bremen.de/vt/laser/papers/DDA-Workshop-final-proceedings.pdf), T. Wriedt and A. G. Hoekstra, Eds., pp. 22–24, Bremen, Germany (2007).

# General DDA theory #
## Review of the DDA theory ##
M. A. Yurkin and A. G. Hoekstra, “The discrete dipole approximation: an overview and recent developments,” [J. Quant. Spectrosc. Radiat. Transfer 106, 558–589 (2007)](http://dx.doi.org/10.1016/j.jqsrt.2007.01.034).

## Convergence of the DDA ##
M. A. Yurkin, V. P. Maltsev, and A. G. Hoekstra, “Convergence of the discrete dipole approximation. I. Theoretical analysis,” [J. Opt. Soc. Am. A 23, 2578–2591 (2006)](http://dx.doi.org/10.1364/JOSAA.23.002578).

## Extrapolation technique ##
M. A. Yurkin, V. P. Maltsev, and A. G. Hoekstra, “Convergence of the discrete dipole approximation. II. An extrapolation technique to increase the accuracy,” [J. Opt. Soc. Am. A 23, 2592–2601 (2006)](http://dx.doi.org/10.1364/JOSAA.23.002592).

## Symmetry tests for the DDA results ##
K. Schmidt, M. A. Yurkin, and M. Kahnert, “A case study on the reciprocity in light scattering computations,” [Opt. Express 20, 23253–23274 (2012)](http://dx.doi.org/10.1364/OE.20.023253).