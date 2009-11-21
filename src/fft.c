/* File: fft.c
 * $Author$
 * $Date::                            $
 * Descr: initialization of all FFT for matrix-vector products; and FFT procedures themselves

 *        TODO: A lot of indirect indexing used - way to optimize.
 *
 *        Previous versions by Michel Grimminck and Alfons Hoekstra
 *
 * Copyright (C) 2006-2008 University of Amsterdam
 * Copyright (C) 2009 Institute of Chemical Kinetics and Combustion & University of Amsterdam
 * This file is part of ADDA.
 *
 * ADDA is free software: you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * ADDA is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with ADDA. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vars.h"
#include "cmplx.h"
#include "const.h"
#include "comm.h"
#include "memory.h"
#include "debug.h"
#include "fft.h"
#include "prec_time.h"
#include "io.h"
#include "function.h"

#ifdef FFTW3
#	include <fftw3.h>
/* define level of planning for usual and Dmatrix (DM) FFT: FFTW_ESTIMATE (heuristics),
 * FFTW_MEASURE (default), FTW_PATIENT, or FFTW_EXHAUSTIVE
 */
#	define PLAN_FFTW FFTW_MEASURE
#	define PLAN_FFTW_DM FFTW_ESTIMATE
#endif
// for transpose YZ
#define TR_BLOCK 64

// SEMI-GLOBAL VARIABLES

// defined ant initialized in calculator.c
extern const double *tab1,*tab2,*tab3,*tab4,*tab5,*tab6,*tab7,*tab8,*tab9,*tab10;
extern const int **tab_index;

// defined and initialized in make_particle.c
extern double gridspace;

// defined and initialized in param.c
extern double igt_lim, igt_eps;

// defined and initialized in timing.c
extern TIME_TYPE Timing_FFT_Init,Timing_Dm_Init;

// used in matvec.c
doublecomplex *Dmatrix;       // holds FFT of the interaction matrix
doublecomplex *Xmatrix;       // holds input vector (on expanded grid) to matvec
doublecomplex *slices;        // used in inner cycle of matvec - holds 3 components (for fixed x)
doublecomplex *slices_tr;     // additional storage space for slices to accelerate transpose
size_t DsizeY,DsizeZ,DsizeYZ; // size of the 'matrix' D

// used in comm.c
double *BT_buffer, *BT_rbuffer; // buffers for BlockTranspose

// LOCAL VARIABLES

// D2 matrix and its two slices; used only temporary for InitDmatrix
static doublecomplex *slice,*slice_tr,*D2matrix;
static size_t D2sizeX,D2sizeY,D2sizeZ; // size of the 'matrix' D2
static size_t blockTr=TR_BLOCK;        // block size for TransposeYZ; see fft.h
static bool weird_nprocs;              // whether weird number of processors is used
#ifdef FFTW3
// FFTW3 plans: f - FFT_FORWARD; b - FFT_BACKWARD
static fftw_plan planXf,planXb,planYf,planYb,planZf,planZb,planXf_Dm,planYf_Dm,planZf_Dm;
#elif defined(FFT_TEMPERTON)
# define IFAX_SIZE 20
static double *trigsX,*trigsY,*trigsZ,*work; // arrays for Temperton FFT
static int ifaxX[IFAX_SIZE],ifaxY[IFAX_SIZE],ifaxZ[IFAX_SIZE];
// Fortran routines from cfft99D.f
void cftfax_(const int *nn,int *ifax,double *trigs);
void cfft99_(double *data,double *_work,const double *trigs,const int *ifax,const int *inc,
	const int *jump,const int *nn,const int *lot,const int *isign);
#endif
#ifndef DISABLE_IGT
void propaespacelibreintadda_(const double *Rij,const double *ka,const double *arretecube,
	const double *relreq, double *result);
#endif

// EXTERNAL FUNCTIONS

// sinint.c
void cisi(double x,double *ci,double *si);

//============================================================

INLINE size_t IndexDmatrix(const size_t x,size_t y,size_t z)
// index D matrix to store final result
{
	if (y>=DsizeY) y=gridY-y;
	if (z>=DsizeZ) z=gridZ-z;

	return(NDCOMP*(x*DsizeYZ+z*DsizeY+y));
}

//============================================================

INLINE size_t IndexGarbledD(const size_t x,int y,int z,const size_t lengthN)
// index D2 matrix after BlockTranspose
{
	if (y<0) y+=D2sizeY;
	if (z<0) z+=D2sizeZ;
#ifdef PARALLEL
	return(((z%lengthN)*D2sizeY+y)*gridX+(z/lengthN)*local_Nx+x%local_Nx);
#else
	return((z*D2sizeY+y)*gridX+x);
#endif
}

//============================================================

INLINE size_t IndexD2matrix(int x,int y,int z,const int nnn)
// index D2 matrix to store calculated elements
{
	if (x<0) x+=gridX;
	if (y<0) y+=D2sizeY;
	//  if (z<0) z+=D2sizeZ;
	return(((z-nnn*local_z0)*D2sizeY+y)*gridX+x);
}

//============================================================

INLINE size_t IndexSliceD2matrix(int y,int z)
// index slice of D2 matrix
{
	if (y<0) y+=gridY;
	if (z<0) z+=gridZ;

	return(y*gridZ+z);
}

//============================================================

INLINE size_t IndexSlice_zyD2matrix(const size_t y,const size_t z)
// index transposed slice of D2 matrix
{
	return (z*gridY+y);
}

//============================================================

void TransposeYZ(const int direction)
/* optimized routine to transpose y and z; forward: slices->slices_tr; backward: slices_tr->slices;
 * direction can be made boolean but this contradicts with existing definitions of FFT_FORWARD and
 * FFT_BACKWARD, which themselves are determined by FFT routines invocation format
 */
{
	size_t y,z,Y,Z,y1,y2,z1,z2,i,j,y0,z0,Xcomp;
	doublecomplex *t0,*t1,*t2,*t3,*t4,*w0,*w1,*w2,*w3;

	if (direction==FFT_FORWARD) {
		Y=gridY;
		Z=gridZ;
		w0=slices;
		t0=slices_tr-Y;
	}
	else { // direction==FFT_BACKWARD
		Y=gridZ;
		Z=gridY;
		w0=slices_tr;
		t0=slices-Y;
	}

	y1=Y/blockTr;
	y2=Y%blockTr;
	z1=Z/blockTr;
	z2=Z%blockTr;

	for(Xcomp=0;Xcomp<3;Xcomp++) {
		w1=w0+Xcomp*gridYZ;
		t1=t0+Xcomp*gridYZ;
		for(i=0;i<=y1;i++) {
			if (i==y1) y0=y2;
			else y0=blockTr;
			w2=w1;
			t2=t1;
			for(j=0;j<=z1;j++) {
				if (j==z1) z0=z2;
				else z0=blockTr;
				w3=w2;
				t3=t2;
				for (y=0;y<y0;y++) {
					t4=t3+y;
					for (z=0;z<z0;z++) {
						cEqual(w3[z],*(t4+=Y));
					}
					w3+=Z;
				}
				w2+=blockTr;
				t2+=blockTr*Y;
			}
			w1+=blockTr*Z;
			t1+=blockTr;
		}
	}
}
//============================================================

static void transposeYZ_Dm(doublecomplex *data,doublecomplex *trans)
// optimized routine to transpose y and z for Dmatrix: data -> trans
{
	size_t y,z,Y,Z,y1,y2,z1,z2,i,j,y0,z0;
	doublecomplex *t1,*t2,*t3,*t4,*w1,*w2,*w3;

	Y=gridY;
	Z=gridZ;

	y1=Y/blockTr;
	y2=Y%blockTr;
	z1=Z/blockTr;
	z2=Z%blockTr;

	w1=data;
	t1=trans-Y;

	for(i=0;i<=y1;i++) {
		if (i==y1) y0=y2;
		else y0=blockTr;
		w2=w1;
		t2=t1;
		for(j=0;j<=z1;j++) {
			if (j==z1) z0=z2;
			else z0=blockTr;
			w3=w2;
			t3=t2;
			for (y=0;y<y0;y++) {
				t4=t3+y;
				for (z=0;z<z0;z++) {
					cEqual(w3[z],*(t4+=Y));
				}
				w3+=Z;
			}
			w2+=blockTr;
			t2+=blockTr*Y;
		}
		w1+=blockTr*Z;
		t1+=blockTr;
	}
}

//============================================================

void fftX(const int isign)
// FFT three components of Xmatrix(x) for all y,z; called from matvec
{
#ifdef FFTW3
	if (isign==FFT_FORWARD) fftw_execute(planXf);
	else fftw_execute(planXb);
#elif defined(FFT_TEMPERTON)
	int nn=gridX,inc=1,jump=nn,lot=boxY;
	size_t z;

	for (z=0;z<3*local_Nz;z++) // -f
		cfft99_((double *)(Xmatrix+z*gridX*smallY),work,trigsX,ifaxX,&inc,&jump,&nn,&lot,&isign);
#endif
}

//============================================================

void fftY(const int isign)
// FFT three components of slices_tr(y) for all z; called from matvec
{
#ifdef FFTW3
	if (isign==FFT_FORWARD) fftw_execute(planYf);
	else fftw_execute(planYb);
#elif defined(FFT_TEMPERTON)
	int nn=gridY,inc=1,jump=nn,lot=smallZ,j;
	// cfft99_ slows down rapidly when lot is big, hence a small loop
	for(j=0;j<6;j++)
		cfft99_((double *)(slices_tr+j*gridY*smallZ),work,trigsY,ifaxY,&inc,&jump,&nn,&lot,&isign);
#endif
}

//============================================================

void fftZ(const int isign)
// FFT three components of slices(z) for all y; called from matvec
{
#ifdef FFTW3
	if (isign==FFT_FORWARD) fftw_execute(planZf);
	else fftw_execute(planZb);
#elif defined(FFT_TEMPERTON)
	int nn=gridZ,inc=1,jump=nn,lot=boxY,Xcomp;

	for (Xcomp=0;Xcomp<3;Xcomp++)
		cfft99_((double *)(slices+gridYZ*Xcomp),work,trigsZ,ifaxZ,&inc,&jump,&nn,&lot,&isign);
#endif
}

//============================================================

static void fftX_Dm(const size_t lengthZ)
// FFT(forward) D2matrix(x) for all y,z; used for Dmatrix calculation
{
#ifdef FFTW3
	fftw_execute(planXf_Dm);
#elif defined(FFT_TEMPERTON)
	int nn=gridX,inc=1,jump=nn,lot=D2sizeY,isign=FFT_FORWARD;
	size_t z;

	for (z=0;z<lengthZ;z++)
		cfft99_((double *)(D2matrix+z*gridX*D2sizeY),work,trigsX,ifaxX,&inc,&jump,&nn,&lot,&isign);
#endif
}

//============================================================

static void fftY_Dm(void)
// FFT(forward) slice_tr(y) for all z; used for Dmatrix calculation
{
#ifdef FFTW3
	fftw_execute(planYf_Dm);
#elif defined(FFT_TEMPERTON)
	int nn=gridY,inc=1,jump=nn,lot=gridZ,isign=FFT_FORWARD;

	cfft99_((double *)slice_tr,work,trigsY,ifaxY,&inc,&jump,&nn,&lot,&isign);
#endif
}

//============================================================

static void fftZ_Dm(void)
// FFT(forward) slice(z) for all y; used for Dmatrix calculation
{
#ifdef FFTW3
	fftw_execute(planZf_Dm);
#elif defined(FFT_TEMPERTON)
	int nn=gridZ,inc=1,jump=nn,lot=gridY,isign=FFT_FORWARD;

	cfft99_((double *)slice,work,trigsZ,ifaxZ,&inc,&jump,&nn,&lot,&isign);
#endif
}

//============================================================

void CheckNprocs(void)
// checks for consistency the specified number of processors; called in the beginning from InitComm
{
	int y=nprocs;

	// initialize weird_nprocs
	weird_nprocs=false;
	// remove simple prime divisors of y
	while (y%2==0) y/=2;
	while (y%3==0) y/=3;
	while (y%5==0) y/=5;
#ifdef FFT_TEMPERTON
	if (y!=1) PrintError("Specified number of processors (%d) is weird (has prime divisors larger "
		"than 5). That is incompatible with Temperton FFT. Revise the number of processors "
		"(recommended) or recompile with FFTW 3 support.",nprocs);
#elif defined(FFTW3)
	while (y%7==0) y/=7;
	// one multiplier of either 11 or 13 is allowed
	if (y%11==0) y/=11;
	else if (y%13==0) y/=13;
	if (y!=1) {
		LogError(EC_WARN,ONE_POS,"Specified number of processors (%d) is weird (has prime divisors "
			"larger than 13 or more than one divisor of either 11 or 13). FFTW3 will work less "
			"efficiently. It is strongly recommended to revise the number of processors.",nprocs);
		weird_nprocs=true;
	}
#endif
}

//============================================================

int fftFit(int x,int divis)
/* find the first number >=x divisible by 2,3,5 only (if FFTW3 7 and one of 11 or 13 are allowed),
 * and also divisible by 2 and divis. If weird_nprocs is used, only the latter condition is required
 */
{
	int y;

	if (weird_nprocs) {
		if (!IS_EVEN(divis)) divis*=2;
		return (divis*((x+divis-1)/divis));
	}
	else while (true) {
		y=x;
		while (y%2==0) y/=2;
		while (y%3==0) y/=3;
		while (y%5==0) y/=5;
#ifdef FFTW3
		while (y%7==0) y/=7;
		// one multiplier of either 11 or 13 is allowed
		if (y%11==0) y/=11;
		else if (y%13==0) y/=13;
#endif
		if (y==1 && IS_EVEN(x) && x%divis==0) return(x);
		x++;
	}
}

//=============================================================

static void fftInitBeforeD(const int lengthZ)
// initialize fft before initialization of Dmatrix
{
#ifdef FFTW3
	int grXint=gridX,grYint=gridY,grZint=gridZ; // this is needed to provide 'int *' to grids

	planYf_Dm=fftw_plan_many_dft(1,&grYint,gridZ,slice_tr,NULL,1,gridY,
		slice_tr,NULL,1,gridY,FFT_FORWARD,PLAN_FFTW_DM);
	planZf_Dm=fftw_plan_many_dft(1,&grZint,gridY,slice,NULL,1,gridZ,
		slice,NULL,1,gridZ,FFT_FORWARD,PLAN_FFTW_DM);
	planXf_Dm=fftw_plan_many_dft(1,&grXint,lengthZ*D2sizeY,D2matrix,NULL,1,gridX,
		D2matrix,NULL,1,gridX,FFT_FORWARD,PLAN_FFTW_DM);
#elif defined(FFT_TEMPERTON)
	int size,nn;

	// allocate memory
	MALLOC_VECTOR(trigsX,double,2*gridX,ALL);
	MALLOC_VECTOR(trigsY,double,2*gridY,ALL);
	MALLOC_VECTOR(trigsZ,double,2*gridZ,ALL);
	size=MAX(gridX*D2sizeY,3*gridYZ);
	MALLOC_VECTOR(work,double,2*size,ALL);
	// initialize ifax and trigs
	nn=gridX;
	cftfax_ (&nn,ifaxX,trigsX);
	nn=gridY;
	cftfax_ (&nn,ifaxY,trigsY);
	nn=gridZ;
	cftfax_ (&nn,ifaxZ,trigsZ);
#endif
}

//============================================================

static void fftInitAfterD(void)
// second part of fft initialization
{
#ifdef FFTW3
	int lot;
	fftw_iodim dims,howmany_dims[2];
	int grYint=gridY; // this is needed to provide 'int *' to gridY
#	ifdef PRECISE_TIMING
	SYSTEM_TIME tvp[13];
#	endif
	PRINTZ("Initializing FFTW3\n");
	FFLUSHZ(stdout);
#	ifdef PRECISE_TIMING
	GetTime(tvp);
#	endif
	lot=3*gridZ;
	planYf=fftw_plan_many_dft(1,&grYint,lot,slices_tr,NULL,1,gridY,
		slices_tr,NULL,1,gridY,FFT_FORWARD,PLAN_FFTW);
#	ifdef PRECISE_TIMING
	GetTime(tvp+1);
#	endif
	planYb=fftw_plan_many_dft(1,&grYint,lot,slices_tr,NULL,1,gridY,
		slices_tr,NULL,1,gridY,FFT_BACKWARD,PLAN_FFTW);
#	ifdef PRECISE_TIMING
	GetTime(tvp+2);
#	endif
	dims.n=gridZ;
	dims.is=dims.os=1;
	howmany_dims[0].n=3;
	howmany_dims[0].is=howmany_dims[0].os=gridZ*gridY;
	howmany_dims[1].n=boxY;
	howmany_dims[1].is=howmany_dims[1].os=gridZ;
	planZf=fftw_plan_guru_dft(1,&dims,2,howmany_dims,slices,slices,FFT_FORWARD,PLAN_FFTW);
#	ifdef PRECISE_TIMING
	GetTime(tvp+3);
#	endif
	planZb=fftw_plan_guru_dft(1,&dims,2,howmany_dims,slices,slices,FFT_BACKWARD,PLAN_FFTW);
#	ifdef PRECISE_TIMING
	GetTime(tvp+4);
#	endif
	dims.n=gridX;
	dims.is=dims.os=1;
	howmany_dims[0].n=3*local_Nz;
	howmany_dims[0].is=howmany_dims[0].os=smallY*gridX;
	howmany_dims[1].n=boxY;
	howmany_dims[1].is=howmany_dims[1].os=gridX;
	planXf=fftw_plan_guru_dft(1,&dims,2,howmany_dims,Xmatrix,Xmatrix,FFT_FORWARD,PLAN_FFTW);
#	ifdef PRECISE_TIMING
	GetTime(tvp+5);
#	endif
	planXb=fftw_plan_guru_dft(1,&dims,2,howmany_dims,Xmatrix,Xmatrix,FFT_BACKWARD,PLAN_FFTW);
#	ifdef PRECISE_TIMING
	GetTime(tvp+6);
	// print precise timing of FFT planning
	SetTimerFreq();
	PRINTBOTHZ(logfile,
		"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
		"         FFTW3 planning       \n"
		"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
		"Yf = %4.4f  Total = %4.4f\n"
		"Yb = %4.4f\n"
		"Zf = %4.4f\n"
		"Zb = %4.4f\n"
		"Xf = %4.4f\n"
		"Xb = %4.4f\n\n",
		DiffSec(tvp,tvp+1),DiffSec(tvp,tvp+6),DiffSec(tvp+1,tvp+2),DiffSec(tvp+2,tvp+3),
		DiffSec(tvp+3,tvp+4),DiffSec(tvp+4,tvp+5),DiffSec(tvp+5,tvp+6));
#	endif
	// destroy old plans
	fftw_destroy_plan(planXf_Dm);
	fftw_destroy_plan(planYf_Dm);
	fftw_destroy_plan(planZf_Dm);
#endif
}

//============================================================

static void CalcInterTerm(int i,int j,int k,doublecomplex *result)
/* calculates interaction term between two dipoles; given integer distance vector {i,j,k}
 * (in units of d). All six components of the symmetric matrix are computed at once.
 */
{
	double rr,rtemp[3],qvec[3],q2[3],invr,invr3,qavec[3],av[3];
	double rr2,kr,kr2,kr3,kd2,q4,rn;
	double temp,qmunu[6],qa,qamunu[6],invrn,invrn2,invrn3,invrn4;
	double dmunu[6]; // KroneckerDelta[mu,nu] - can serve both as multiplier, and as bool
	double kfr,ci,si,ci1,si1,ci2,si2,brd,cov,siv,g0,g2;
	doublecomplex expval,br,br1,m,m2,Gf1,Gm0,Gm1,Gc1,Gc2;
	int ind0,ind1,ind2,ind2m,ind3,ind4,indmunu,comp,mu,nu,mu1,nu1;
	int sigV[3],ic,sig,ivec[3],ord[3],invord[3];
	double t3q,t3a,t4q,t4a,t5tr,t5aa,t6tr,t6aa;
	const bool inter_avg=true; // temporary fixed option for SO formulation

	// self interaction; self term is computed in different subroutine
	if (i==0 && j==0 && k==0) for (comp=0;comp<NDCOMP;comp++) {
		result[comp][RE]=result[comp][IM]=0;
		return;
	}
	// initialize rtemp
	rtemp[0]=i*gridspace;
	rtemp[1]=j*gridspace;
	rtemp[2]=k*gridspace;
	//====== calculate some basic constants ======
	rr2=DotProd(rtemp,rtemp);
	rr=sqrt(rr2);
	rn=rr/gridspace; // normalized r
#ifndef DISABLE_IGT
	if (IntRelation==G_IGT && (igt_lim==UNDEF || rn<=igt_lim))
		propaespacelibreintadda_(rtemp,&WaveNum,&gridspace,&igt_eps,(double *)result);
	else {
#endif
		invr=1/rr;
		invr3=invr*invr*invr;
		MultScal(invr,rtemp,qvec);
		kr=WaveNum*rr;
		kr2=kr*kr;
		kfr=PI*rn; // k_F*r, for FCD
		// cov=cos(kr); siv=sin(kr); expval=Exp(ikr)/r^3
		imExp(kr,expval);
		cov=expval[RE];
		siv=expval[IM];
		cMultReal(invr3,expval,expval);
		//====== calculate Gp ========
		for (mu=0,comp=0;mu<3;mu++) for (nu=mu;nu<3;nu++,comp++) {
			dmunu[comp]= mu==nu ? 1 : 0;
			qmunu[comp]=qvec[mu]*qvec[nu];
			// br=delta[mu,nu]*(-1+ikr+kr^2)-qmunu*(-3+3ikr+kr^2)
			br[RE]=(3-kr2)*qmunu[comp];
			br[IM]=-3*kr*qmunu[comp];
			if(dmunu[comp]) {
				br[RE]+=kr2-1;
				br[IM]+=kr;
			}
			// result=Gp=expval*br
			cMult(br,expval,result[comp]);
		}
#ifndef DISABLE_IGT
	} // end of else
#endif
	//====== FCD (static and full) ========
	/* speed of FCD can be improved by using faster version of sici routine, using predefined
	 * tables, etc (e.g. as is done in GSL library). But currently extra time for this computation
	 * is already smaller than one main iteration.
	 */
	if (IntRelation==G_FCD_ST) {
		/* FCD is based on Gay-Balmaz P., Martin O.J.F. "A library for computing the filtered and
		 * non-filtered 3D Green's tensor associated with infinite homogeneous space and surfaces",
		 * Comp. Phys. Comm. 144:111-120 (2002), and Piller N.B. "Increasing the performance of the
		 * coupled-dipole approximation: A spectral approach", IEEE Trans.Ant.Propag. 46(8):
		 * 1126-1137. Here it differs by a factor of 4*pi*k^2.
		 */
		// result = Gp*[3*Si(k_F*r)+k_F*r*cos(k_F*r)-4*sin(k_F*r)]*2/(3*pi)
		cisi(kfr,&ci,&si);
		brd=TWO_OVER_PI*ONE_THIRD*(3*si+kfr*cos(kfr)-4*sin(kfr));
		for (comp=0;comp<NDCOMP;comp++) cMultReal(brd,result[comp],result[comp]);
	}
	else if (IntRelation==G_FCD) {
		// ci,si_1,2 = ci,si_+,- = Ci,Si((k_F +,- k)r)
		cisi(kfr+kr,&ci1,&si1);
		cisi(kfr-kr,&ci2,&si2);
		// ci=ci1-ci2; si=pi-si1-si2
		ci=ci1-ci2;
		si=PI-si1-si2;
		g0=INV_PI*(siv*ci+cov*si);
		g2=INV_PI*(kr*(cov*ci-siv*si)+2*ONE_THIRD*(kfr*cos(kfr)-4*sin(kfr)))-g0;
		temp=g0*kr2;
		for (comp=0;comp<NDCOMP;comp++) {
			// brd=(delta[mu,nu]*(-g0*kr^2-g2)+qmunu*(g0*kr^2+3g2))/r^3
			brd=qmunu[comp]*(temp+3*g2);
			if (dmunu[comp]) brd-=temp+g2;
			brd*=invr3;
			// result=Gp+brd
			result[comp][RE]+=brd;
		}
	}
	//======= second order corrections ========
	else if (IntRelation==G_SO) {
		/* There is still some space for speed optimization here (e.g. move mu,nu-independent
		 * operations out of the cycles over components). But now extra time is equivalent to 2-3
		 * main iterations. So first priority is to make something useful out of SO.
		 */
		// next line should never happen
		if (anisotropy) LogError(EC_ERROR,ONE_POS,"Incompatibility error in CalcInterTerm");
		kd2=kd*kd;
		kr3=kr2*kr;
		// only one refractive index can be used for FFT-compatible algorithm
		cEqual(ref_index[0],m);
		cSquare(m,m2);
		if (!inter_avg) {
			qa=DotProd(qvec,prop);
			for (mu=0,comp=0;mu<3;mu++) for (nu=mu;nu<3;nu++,comp++) {
				// qamunu=qvec[mu]*prop[nu] + qvec[nu]*prop[mu]
				qamunu[comp]=qvec[mu]*prop[nu];
				if (dmunu[comp]) qamunu[comp]*=2;
				else qamunu[comp]+=qvec[nu]*prop[mu];
			}
		}
		if (kr*rn < G_BOUND_CLOSE) {
			//====== G close =============
			// check if inside the table bounds; needed to recompute to make an integer comparison
			if ((i*i+j*j+k*k) > TAB_RMAX*TAB_RMAX) LogError(EC_ERROR,ALL_POS,
				"Not enough table size (available only up to R/d=%d)",TAB_RMAX);

			// av is copy of propagation vector
			if (!inter_avg) memcpy(av,prop,3*sizeof(double));
			ivec[0]=i;
			ivec[1]=j;
			ivec[2]=k;
			// transformation of negative coordinates
			for (ic=0;ic<3;ic++) {
				if (ivec[ic]<0) {
					sigV[ic]=-1;
					av[ic]*=-1;
					qvec[ic]*=-1;
					ivec[ic]*=-1;
				}
				else sigV[ic]=1;
			}
			i=ivec[0];
			j=ivec[1];
			k=ivec[2];
			// transformation to case i>=j>=k>=0
			// building of ord; ord[x] is x-th largest coordinate (0-th - the largest)
			if (i>=j) {
				if (i>=k) {
					ord[0]=0;
					if (j>=k) {
						ord[1]=1;
						ord[2]=2;
					}
					else {
						ord[1]=2;
						ord[2]=1;
					}
				}
				else {
					ord[0]=2;
					ord[1]=0;
					ord[2]=1;
				}
			}
			else {
				if (i>=k) {
					ord[0]=1;
					ord[1]=0;
					ord[2]=2;
				}
				else {
					ord[2]=0;
					if (j>=k) {
						ord[0]=1;
						ord[1]=2;
					}
					else {
						ord[0]=2;
						ord[1]=1;
					}
				}
			}
			// change parameters according to coordinate transforms
			Permutate(qvec,ord);
			if (!inter_avg) Permutate(av,ord);
			Permutate_i(ivec,ord);
			i=ivec[0];
			j=ivec[1];
			k=ivec[2];
			// compute inverse permutation
			memcpy(invord,ord,3*sizeof(int));
			Permutate_i(invord,ord);
			if (invord[0]==0 && invord[1]==1 && invord[2]==2) memcpy(invord,ord,3*sizeof(int));
			// temp = kr/24
			temp=kr/24;
			// cycle over tensor components
			for (mu=0,comp=0;mu<3;mu++) for (nu=mu;nu<3;nu++,comp++) {
				sig=sigV[mu]*sigV[nu]; // sign of some terms below
				/* indexes for tables of different dimensions based on transformed indices mu and nu
				 * '...munu' variables are invariant to permutations because both constituent
				 * vectors and indices are permutated. So this variables can be used, as precomputed
				 * above.
				 */
				mu1=invord[mu];
				nu1=invord[nu];
				/* indmunu is a number of component[mu,nu] in a symmetric matrix, but counted
				 * differently than comp. This is {{0,1,3},{1,2,4},{3,4,5}}
				 */
				indmunu=mu1+nu1;
				if (mu1==2 || nu1==2) indmunu++;

				ind0=tab_index[i][j]+k;
				ind1=3*ind0;
				ind2m=6*ind0;
				ind2=ind2m+indmunu;
				ind3=3*ind2;
				ind4=6*ind2;
				// computing several quantities with table integrals
				t3q=DotProd(qvec,tab3+ind1);
				t4q=DotProd(qvec,tab4+ind3);
				t5tr=TrSym(tab5+ind2m);
				t6tr=TrSym(tab6+ind4);
				if (inter_avg) {
					// <a[mu]*a[nu]>=1/3*delta[mu,nu]
					t5aa=ONE_THIRD*t5tr;
					t6aa=ONE_THIRD*t6tr;
				}
				else {
					t3a=DotProd(av,tab3+ind1);
					t4a=DotProd(av,tab4+ind3);
					t5aa=QuadForm(tab5+ind2m,av);
					t6aa=QuadForm(tab6+ind4,av);
				}
				//====== computing Gc0 =====
				/* br = delta[mu,nu]*(-I7-I9/2-kr*(i+kr)/24+2*t3q+t5tr)
				 *    - (-3I8[mu,nu]-3I10[mu,nu]/2-qmunu*kr*(i+kr)/24+2*t4q+t6tr)
				 */
				br[RE]=sig*(3*(tab10[ind2]/2+tab8[ind2])-2*t4q-t6tr)+temp*qmunu[comp]*kr;
				br[IM]=3*temp*qmunu[comp];
				if (dmunu[comp]) {
					br[RE]+=2*t3q+t5tr-temp*kr-tab9[ind0]/2-tab7[ind0];
					br[IM]-=temp;
				}
				// br*=kd^2
				cMultReal(kd2,br,br);
				// br+=I1*delta[mu,nu]*(-1+ikr+kr^2)-sig*I2[mu,nu]*(-3+3ikr+kr^2)
				br[RE]+=sig*tab2[ind2]*(3-kr2);
				br[IM]-=sig*tab2[ind2]*3*kr;
				if (dmunu[comp]) {
					br[RE]+=tab1[ind0]*(kr2-1);
					br[IM]+=tab1[ind0]*kr;
				}
				// Gc0=expval*br
				cMult(expval,br,result[comp]);
				//==== computing Gc1 ======
				if (!inter_avg) {
					// br=(kd*kr/24)*(qa*(delta[mu,nu]*(-2+ikr)-qmunu*(-6+ikr))-qamunu)
					br[RE]=6*qmunu[comp];
					br[IM]=-kr*qmunu[comp];
					if (dmunu[comp]) {
						br[RE]-=2;
						br[IM]+=kr;
					}
					cMultReal(qa,br,br);
					br[RE]-=qamunu[comp];
					cMultReal(2*temp*kd,br,br);
					//  br1=(d/r)*(delta[mu,nu]*t3h*(-1+ikr)-sig*t4h*(-3+3ikr))
					br1[RE]=3*sig*t4a;
					br1[IM]=-kr*br1[RE];
					if (dmunu[comp]) {
						br1[RE]-=t3a;
						br1[IM]+=t3a*kr;
					}
					cMultReal(1/rn,br1,br1);
					// Gc1=expval*i*m*kd*(br1+br)
					cAdd(br,br1,Gc1);
					cMultSelf(Gc1,m);
					cMultReal(kd,Gc1,Gc1);
					cMultSelf(Gc1,expval);
					cMult_i(Gc1);
				}
				//==== computing Gc2 ======
				// br=delta[mu,nu]*t5aa-3*sig*t6aa-(kr/12)*(delta[mu,nu]*(i+kr)-qmunu*(3i+kr))
				br[RE]=-kr*qmunu[comp];
				br[IM]=-3*qmunu[comp];
				if (dmunu[comp]) {
					br[RE]+=kr;
					br[IM]+=1;
				}
				cMultReal(-2*temp,br,br);
				br[RE]-=3*sig*t6aa;
				if (dmunu[comp]) br[RE]+=t5aa;
				// Gc2=expval*(kd^2/2)*m^2*br
				cMult(m2,br,Gc2);
				cMultReal(kd2/2,Gc2,Gc2);
				cMultSelf(Gc2,expval);
				// result = Gc0 + [ Gc1 ] + Gc2
				if (!inter_avg) cAdd(Gc2,Gc1,Gc2);
				cAdd(Gc2,result[comp],result[comp]);
			}
		}
		else {
			//====== Gfar (and part of Gmedian) =======
			// temp=kd^2/24
			temp=kd2/24;
			// br=1-(1+m^2)*kd^2/24
			br[RE]=1-(1+m2[RE])*temp;
			br[IM]=-m2[IM]*temp;
			// Gf0 + Gf2 = Gp*br
			for (comp=0;comp<NDCOMP;comp++) cMultSelf(result[comp],br);
			//==== compute and add Gf1 ===
			if (!inter_avg) for (comp=0;comp<NDCOMP;comp++) {
				/* br = {delta[mu,nu]*(3-3ikr-2kr^2+ikr^3)-qmunu*(15-15ikr-6kr^2+ikr^3)}*qa
				 *    + qamunu*(3-3ikr-kr^2)
				 */
				br[RE]=(6*kr2-15)*qmunu[comp];
				br[IM]=(15*kr-kr3)*qmunu[comp];
				if(dmunu[comp]) {
					br[RE]+=3-2*kr2;
					br[IM]+=kr3-3*kr;
				}
				cMultReal(qa,br,br);
				br[RE]+=(3-kr2)*qamunu[comp];
				br[IM]-=3*kr*qamunu[comp];
				// Gf1=expval*i*m*temp*br
				cMult(m,br,Gf1);
				cMultReal(temp*2/kr,Gf1,Gf1);
				cMultSelf(Gf1,expval);
				cMult_i(Gf1);
				// result = Gf
				cAdd(Gf1,result[comp],result[comp]);
			}
			if (kr < G_BOUND_MEDIAN) {
				//===== G median ========
				vMult(qvec,qvec,q2);
				q4=DotProd(q2,q2);
				invrn=1/rn;
				invrn2=invrn*invrn;
				invrn3=invrn2*invrn;
				invrn4=invrn2*invrn2;
				for (mu=0,comp=0;mu<3;mu++) for (nu=mu;nu<3;nu++,comp++) {
					// Gm0=expval*br*temp; temp is set anew
					temp=qmunu[comp]*(33*q4-7-12*(q2[mu]+q2[nu]));
					if (mu == nu) temp+=(1-3*q4+4*q2[mu]);
					temp*=7*invrn4/64;
					br[RE]=-1;
					br[IM]=kr;
					cMultReal(temp,br,Gm0);
					cMultSelf(Gm0,expval);
					if (!inter_avg) {
						// Gm1=expval*i*m*temp; temp is set anew
						vMult(qvec,prop,qavec);
						temp = 3*qa*(dmunu[comp]-7*qmunu[comp]) + 6*dmunu[comp]*qvec[mu]*prop[mu]
							 - 7*(dmunu[comp]-9*qmunu[comp])*DotProd(qavec,q2)
							 + 3*(prop[mu]*qvec[nu]*(1-7*q2[mu])+prop[nu]*qvec[mu]*(1-7*q2[nu]));
						temp*=kd*invrn3/48;
						cMultReal(temp,m,Gm1);
						cMult_i(Gm1);
						cMultSelf(Gm1,expval);
						// add Gm1 to Gm0
						cAdd(Gm0,Gm1,Gm0);
					}
					// result = Gf + Gm0 + [ Gm1 ]
					cAdd(Gm0,result[comp],result[comp]);
				}
			}
		}
	}
}

//============================================================

void InitDmatrix(void)
/* Initializes the matrix D. D[i][j][k]=A[i1-i2][j1-j2][k1-k2]. Actually D=-FFT(G)/Ngrid.
 * Then -G.x=invFFT(D*FFT(x)) for practical implementation of FFT such that invFFT(FFT(x))=Ngrid*x.
 * G is exactly Green's tensor. The routine is called only once, so needs not to be very fast,
 * however we tried to optimize it.
 */
{
	int i,j,k,kcor,Dcomp;
	size_t x,y,z,indexfrom,indexto,ind,index,Dsize,D2sizeTot;
	double invNgrid,mem;
	int nnn; // multiplier used for reduced_FFT or not reduced; 1 or 2
	int jstart, kstart;
	size_t lengthN;
	TIME_TYPE start,time1;
#ifdef PARALLEL
	size_t bufsize;
#endif
#ifdef PRECISE_TIMING
	// precise timing of the Dmatrix computation
	SYSTEM_TIME tvp[13];
	SYSTEM_TIME Timing_fftX,Timing_fftY,Timing_fftZ,Timing_Gcalc,Timing_ar1,Timing_ar2,Timing_ar3,
	Timing_BT,Timing_TYZ,Timing_beg;
	double t_fftX,t_fftY,t_fftZ,t_ar1,t_ar2,t_ar3,
	t_TYZ,t_beg,t_Gcalc,t_Arithm,t_FFT,t_BT;

	InitTime(&Timing_fftX);
	InitTime(&Timing_fftY);
	InitTime(&Timing_fftZ);
	InitTime(&Timing_ar1);
	InitTime(&Timing_ar2);
	InitTime(&Timing_ar3);
	InitTime(&Timing_BT);
	InitTime(&Timing_TYZ);
	GetTime(tvp);
#endif
	start=GET_TIME();
	// initialize sizes of D and D2 matrices
	D2sizeX=gridX;
	if (reduced_FFT) {
		D2sizeY=gridY/2;
		D2sizeZ=gridZ/2;
		DsizeY=gridY/2+1;
		DsizeZ=gridZ/2+1;
		nnn=1;
		jstart=0;
		kstart=0;
	}
	else {
		D2sizeY=DsizeY=gridY;
		D2sizeZ=DsizeZ=gridZ;
		nnn=2;
		jstart=1-boxY;
		kstart=1-boxZ;
	}
	// auxiliary parameters
	lengthN=nnn*local_Nz;
	DsizeYZ=DsizeY*DsizeZ;
	invNgrid=1.0/(gridX*((double)gridYZ));
	local_Nsmall=(gridX/2)*(gridYZ/(2*nprocs)); // size of X vector (for 1 component)
	/* calculate size of matvec matrices (X,D,slices,slices_tr) and BT buffers (if parallel);
	 * uses complex expression to avoid overflows and enable prognosis for large grids
	 */
	mem = sizeof(doublecomplex)*(3*(2+(gridX/(4.0*nprocs)))*((double)gridYZ)
		+ NDCOMP*local_Nx*((double)DsizeYZ));
#ifdef PARALLEL
	mem+=12*smallY*((double)(local_Nz*local_Nx))*sizeof(double);
#endif
	// printout some information
	/* conversions to (unsigned long) are needed (to remove warnings) because %z printf argument is not
	 * yet supported by all target compiler environmets
	 */
	FPRINTZ(logfile,"The FFT grid is: %lux%lux%lu\n",(unsigned long)gridX,(unsigned long)gridY,
		(unsigned long)gridZ);
#ifdef PARALLEL
	PRINTBOTHZ(logfile,"Memory usage for MatVec matrices (per processor): %.1f Mb\n",mem/MBYTE);
#else
	PRINTBOTHZ(logfile,"Memory usage for MatVec matrices: %.1f Mb\n",mem/MBYTE);
#endif
	FFLUSHZ(logfile);
	memory+=mem;
	if (prognosis) return;
	// allocate memory for Dmatrix
	Dsize=MultOverflow(NDCOMP*local_Nx,DsizeYZ,ONE_POS,"Dmatrix");
	MALLOC_VECTOR(Dmatrix,complex,Dsize,ALL);
	// allocate memory for D2matrix components
	D2sizeTot=nnn*local_Nz*D2sizeY*D2sizeX; // this should be approximately equal to Dsize/NDCOMP
	MALLOC_VECTOR(D2matrix,complex,D2sizeTot,ALL);
	MALLOC_VECTOR(slice,complex,gridYZ,ALL);
	MALLOC_VECTOR(slice_tr,complex,gridYZ,ALL);
	// actually allocation of Xmatrix, slices, slices_tr is below after freeing of Dmatrix and its slice
#ifdef PARALLEL
	// allocate buffer for BlockTranspose_Dm
	bufsize = 2*lengthN*D2sizeY*local_Nx;
	MALLOC_VECTOR(BT_buffer,double,bufsize,ALL);
	MALLOC_VECTOR(BT_rbuffer,double,bufsize,ALL);
#endif
	D("Initialize FFT (1st part)");
	fftInitBeforeD(lengthN);

#ifdef PRECISE_TIMING
	GetTime(tvp+1);
	Elapsed(tvp,tvp+1,&Timing_beg);
#endif
	PRINTZ("Calculating Green's function (Dmatrix)\n");
	FFLUSHZ(stdout);
	/* Interaction matrix values are calculated all at once for performance reasons. They are stored
	 * in Dmatrix with indexing corresponding to D2matrix (to facilitate copying) but NDCOMP
	 * elements instead of one. Afterwards they are replaced by Fourier transforms (with different
	 * indexing) component-wise (in cycle over NDCOMP)
	 */

	/* fill Dmatrix with 0, this if to fill the possible gap between e.g. boxY and gridY/2;
	 * probably faster than using a lot of conditionals
	 */
	for (ind=0;ind<Dsize;ind++) Dmatrix[ind][RE]=Dmatrix[ind][IM]=0;
	// fill Dmatrix with values of Green's tensor
	for(k=nnn*local_z0;k<nnn*local_z1;k++) {
		// correction of k is relevant only if reduced_FFT is not used
		if (k>(int)smallZ) kcor=k-gridZ;
		else kcor=k;
		for (j=jstart;j<boxY;j++) for (i=1-boxX;i<boxX;i++) {
			index=NDCOMP*IndexD2matrix(i,j,k,nnn);
			CalcInterTerm(i,j,kcor,Dmatrix+index);
		}
	} // end of i,j,k loop
#ifdef PRECISE_TIMING
	GetTime(tvp+2);
	Elapsed(tvp+1,tvp+2,&Timing_Gcalc);
#endif
	PRINTZ("Fourier transform of Dmatrix");
	FFLUSHZ(stdout);
	for(Dcomp=0;Dcomp<NDCOMP;Dcomp++) { // main cycle over components of Dmatrix
#ifdef PRECISE_TIMING
		GetTime(tvp+2); // same as the last before cycle
#endif
		// fill D2matrix with precomputed values from D2matrix
		for (ind=0;ind<D2sizeTot;ind++) cEqual(Dmatrix[NDCOMP*ind+Dcomp],D2matrix[ind]);
#ifdef PRECISE_TIMING
		GetTime(tvp+3);
		ElapsedInc(tvp+2,tvp+3,&Timing_ar1);
#endif
		fftX_Dm(lengthN); // fftX D2matrix
#ifdef PRECISE_TIMING
		GetTime(tvp+4);
		ElapsedInc(tvp+3,tvp+4,&Timing_fftX);
#endif
		BlockTranspose_Dm(D2matrix,D2sizeY,lengthN);
#ifdef PRECISE_TIMING
		GetTime(tvp+5);
		ElapsedInc(tvp+4,tvp+5,&Timing_BT);
#endif
		for(x=local_x0;x<local_x1;x++) {
#ifdef PRECISE_TIMING
			GetTime(tvp+6);
#endif
			for (ind=0;ind<gridYZ;ind++) slice[ind][RE]=slice[ind][IM]=0.0; // fill slice with 0.0
			for(j=jstart;j<boxY;j++) for(k=kstart;k<boxZ;k++) {
				indexfrom=IndexGarbledD(x,j,k,lengthN);
				indexto=IndexSliceD2matrix(j,k);
				cEqual(D2matrix[indexfrom],slice[indexto]);
			}
			if (reduced_FFT) {
				for(j=1;j<boxY;j++) for(k=0;k<boxZ;k++) {
					// mirror along y
					indexfrom=IndexSliceD2matrix(j,k);
					indexto=IndexSliceD2matrix(-j,k);
					if (Dcomp==1 || Dcomp==4) cInvSign2(slice[indexfrom],slice[indexto]);
					else cEqual(slice[indexfrom],slice[indexto]);
				}
				for(j=1-boxY;j<boxY;j++) for(k=1;k<boxZ;k++) {
					// mirror along z
					indexfrom=IndexSliceD2matrix(j,k);
					indexto=IndexSliceD2matrix(j,-k);
					if (Dcomp==2 || Dcomp==4) cInvSign2(slice[indexfrom],slice[indexto]);
					else cEqual(slice[indexfrom],slice[indexto]);
				}
			}
#ifdef PRECISE_TIMING
			GetTime(tvp+7);
			ElapsedInc(tvp+6,tvp+7,&Timing_ar2);
#endif
			fftZ_Dm(); // fftZ slice
#ifdef PRECISE_TIMING
			GetTime(tvp+8);
			ElapsedInc(tvp+7,tvp+8,&Timing_fftZ);
#endif
			transposeYZ_Dm(slice,slice_tr);
#ifdef PRECISE_TIMING
			GetTime(tvp+9);
			ElapsedInc(tvp+8,tvp+9,&Timing_TYZ);
#endif
			fftY_Dm(); // fftY slice_tr
#ifdef PRECISE_TIMING
			GetTime(tvp+10);
			ElapsedInc(tvp+9,tvp+10,&Timing_fftY);
#endif
			for(z=0;z<DsizeZ;z++) for(y=0;y<DsizeY;y++) {
				indexto=IndexDmatrix(x-local_x0,y,z)+Dcomp;
				indexfrom=IndexSlice_zyD2matrix(y,z);
				cMultReal(-invNgrid,slice_tr[indexfrom],Dmatrix[indexto]);
			}
#ifdef PRECISE_TIMING
			GetTime(tvp+11);
			ElapsedInc(tvp+10,tvp+11,&Timing_ar3);
#endif
		} // end slice X
		PRINTZ(".");
		FFLUSHZ(stdout);
	} // end of Dcomp
	// free vectors used for computation of Dmatrix
	Free_cVector(D2matrix);
	Free_cVector(slice);
	Free_cVector(slice_tr);
#ifdef PARALLEL
	// deallocate buffers for BlockTranspose_Dm
	Free_general(BT_buffer);
	Free_general(BT_rbuffer);
	// allocate buffers for BlockTranspose
	bufsize = 6*smallY*local_Nz*local_Nx; // in doubles
	MALLOC_VECTOR(BT_buffer,double,bufsize,ALL);
	MALLOC_VECTOR(BT_rbuffer,double,bufsize,ALL);
#endif
	// allocate memory for Xmatrix, slices and slices_tr - used in matvec
	MALLOC_VECTOR(Xmatrix,complex,3*local_Nsmall,ALL);
	MALLOC_VECTOR(slices,complex,3*gridYZ,ALL);
	MALLOC_VECTOR(slices_tr,complex,3*gridYZ,ALL);

	PRINTZ("\n");
	time1=GET_TIME();
	Timing_Dm_Init=time1-start;
#ifdef PRECISE_TIMING
	GetTime(tvp+12);
	// analyze and print precise timing information
	SetTimerFreq();
	t_beg=TimerToSec(&Timing_beg);
	t_Gcalc=TimerToSec(&Timing_Gcalc);
	t_ar1=TimerToSec(&Timing_ar1);
	t_ar2=TimerToSec(&Timing_ar2);
	t_ar3=TimerToSec(&Timing_ar3);
	t_fftX=TimerToSec(&Timing_fftX);
	t_fftY=TimerToSec(&Timing_fftY);
	t_fftZ=TimerToSec(&Timing_fftZ);
	t_TYZ=TimerToSec(&Timing_TYZ);
	t_BT=TimerToSec(&Timing_BT);
	t_Arithm=t_beg+t_Gcalc+t_ar1+t_ar2+t_ar3+t_TYZ;
	t_FFT=t_fftX+t_fftY+t_fftZ;

	PRINTBOTHZ(logfile,
		"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
		"            Init Dmatrix timing            \n"
		"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
		"Begin  = %4.4f    Arithmetics = %4.4f\n"
		"Gcalc  = %4.4f    FFT         = %4.4f\n"
		"Arith1 = %4.4f    Comm        = %4.4f\n"
		"FFTX   = %4.4f\n"
		"BT     = %4.4f          Total = %4.4f\n"
		"Arith2 = %4.4f\n"
		"FFTZ   = %4.4f\n"
		"TYZ    = %4.4f\n"
		"FFTY   = %4.4f\n"
		"Arith3 = %4.4f\n\n",
		t_beg,t_Arithm,t_Gcalc,t_FFT,t_ar1,t_BT,t_fftX,t_BT,
		DiffSec(tvp,tvp+12),t_ar2,t_fftZ,t_TYZ,t_fftY,t_ar3);
#endif

	fftInitAfterD();

	Timing_FFT_Init = GET_TIME()-time1;
}

//============================================================

void Free_FFT_Dmat(void)
// free all vectors that were allocated in fft.c (all used for FFT and MatVec)
{
	Free_cVector(Dmatrix);
	Free_cVector(Xmatrix);
	Free_cVector(slices);
	Free_cVector(slices_tr);
#ifdef PARALLEL
	Free_general(BT_buffer);
	Free_general(BT_rbuffer);
#endif
#ifdef FFTW3
	fftw_destroy_plan(planXf);
	fftw_destroy_plan(planXb);
	fftw_destroy_plan(planYf);
	fftw_destroy_plan(planYb);
	fftw_destroy_plan(planZf);
	fftw_destroy_plan(planZb);
#elif defined(FFT_TEMPERTON)
	Free_general(work);
	Free_general(trigsX);
	Free_general(trigsY);
	Free_general(trigsZ);
#endif
}