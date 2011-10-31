/* File: matvec.c
 * $Date::                            $
 * Descr: calculate local matrix vector product of decomposed interaction matrix with r_k or p_k,
 *        using a FFT based convolution algorithm
 *
 * Copyright (C) 2006-2011 ADDA contributors
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
#include <stdio.h>
#include <string.h>
#include "vars.h"
#include "cmplx.h"
#include "const.h"
#include "comm.h"
#include "fft.h"
#include "prec_time.h"
#include "linalg.h"
#include "function.h"
#include "io.h"
#include "interaction.h"
#include "debug.h"

#ifdef OPENCL
#	include "oclcore.h"
#endif

// SEMI-GLOBAL VARIABLES

// defined and initialized in fft.c
extern const doublecomplex * restrict Dmatrix;
extern doublecomplex * restrict slices,* restrict slices_tr;
extern const size_t DsizeY,DsizeZ,DsizeYZ;

// defined and initialized in timing.c
extern size_t TotalMatVec;

#ifndef ADDA_SPARSE
#ifndef OPENCL // the following inline functions are not used in OCL or sparse mode
//============================================================

INLINE size_t IndexSliceZY(const size_t y,const size_t z)
{
	return (z*gridY+y);
}

//============================================================

INLINE size_t IndexSliceYZ(const size_t y,const size_t z)
{
	return(y*gridZ+z);
}

//============================================================

INLINE size_t IndexGarbledX(const size_t x,const size_t y,const size_t z)
{
#ifdef PARALLEL
	return(((z%local_Nz)*smallY+y)*gridX+(z/local_Nz)*local_Nx+x%local_Nx);
#else
	return((z*smallY+y)*gridX+x);
#endif
}

//============================================================

INLINE size_t IndexXmatrix(const size_t x,const size_t y,const size_t z)
{
	return((z*smallY+y)*gridX+x);
}

//============================================================

INLINE size_t IndexDmatrix_mv(size_t x,size_t y,size_t z,const bool transposed)
{
	if (transposed) { // used only for G_SO
		if (x>0) x=gridX-x;
		if (y>0) y=gridY-y;
		if (z>0) z=gridZ-z;
	}
	else {
		if (y>=DsizeY) y=gridY-y;
		if (z>=DsizeZ) z=gridZ-z;
	}

	return(NDCOMP*(x*DsizeYZ+z*DsizeY+y));
}
#endif //OCL
#endif //ADDA_SPARSE

//============================================================

#ifndef ADDA_SPARSE
void MatVec (doublecomplex * restrict argvec,    // the argument vector
             doublecomplex * restrict resultvec, // the result vector
             double *inprod,         // the resulting inner product
             const bool her,         // whether Hermitian transpose of the matrix is used
             TIME_TYPE *comm_timing) // this variable is incremented by communication time
/* This function implements both MatVec_nim and MatVecAndInp_nim. The difference is that when we
 * want to calculate the inner product as well, we pass 'inprod' as a non-NULL pointer. if 'inprod'
 * is NULL, we don't calculate it. 'argvec' always remains unchanged afterwards, however it is not
 * strictly const - some manipulations may occur during the execution. comm_timing can be NULL, then
 * it is ignored.
 */
{
	size_t j,x;
	bool ipr,transposed;
	size_t boxY_st=boxY,boxZ_st=boxZ; // copies with different type
#ifndef OPENCL // these variables are not needed for OpenCL
	size_t i;
	doublecomplex fmat[6],xv[3],yv[3];
	doublecomplex temp;
	size_t index,y,z,Xcomp;
	unsigned char mat;
#endif
#ifdef PRECISE_TIMING
	SYSTEM_TIME tvp[18];
	SYSTEM_TIME Timing_FFTXf,Timing_FFTYf,Timing_FFTZf,Timing_FFTXb,Timing_FFTYb,Timing_FFTZb,
	Timing_Mult1,Timing_Mult2,Timing_Mult3,Timing_Mult4,Timing_Mult5,
	Timing_BTf,Timing_BTb,Timing_TYZf,Timing_TYZb,Timing_ipr;
	double t_FFTXf,t_FFTYf,t_FFTZf,t_FFTXb,t_FFTYb,t_FFTZb,
	t_Mult1,t_Mult2,t_Mult3,t_Mult4,t_Mult5,t_ipr,
	t_BTf,t_BTb,t_TYZf,t_TYZb,t_Arithm,t_FFT,t_Comm;

#endif

/* A = I + S.D.S
 * S = sqrt(C)
 * A.x = x + S.D.(S.x)
 * A(H).x = x + (S(T).D(T).S(T).x(*))(*)
 * C,S - diagonal => symmetric
 * (!! will change if tensor (non-diagonal) polarizability is used !!)
 * D - symmetric (except for G_SO)
 *
 * D.x=F(-1)(F(D).F(X))
 * F(D) is just a vector
 *
 * G_SO: F(D(T)) (k) =  F(D) (-k)
 *       k - vector index
 *
 *   For (her) three additional operations of nConj are used. Should not be a problem,
 *     but can be avoided by a more complex code.
 */

	transposed=(!reduced_FFT) && her;
	ipr=(inprod!=NULL);
	if (ipr && !ipr_required) LogError(ONE_POS,"Incompatibility error in MatVec");
#ifdef PRECISE_TIMING
	InitTime(&Timing_FFTYf);
	InitTime(&Timing_FFTZf);
	InitTime(&Timing_FFTYb);
	InitTime(&Timing_FFTZb);
	InitTime(&Timing_Mult2);
	InitTime(&Timing_Mult3);
	InitTime(&Timing_Mult4);
	InitTime(&Timing_TYZf);
	InitTime(&Timing_TYZb);
	GetTime(tvp);
#endif
	// FFT_matvec code
	if (ipr) *inprod = 0.0;
#ifdef OPENCL
	// needed for Arith3 but declared here since Arith3 is called inside a loop
	const size_t gwsclarith3[2]={gridZ,gridY};
	const cl_long ndcomp=NDCOMP;
	const cl_char transp=(cl_char)transposed;
	const cl_char redfft=(cl_char)reduced_FFT; //little workaround for kernel cannot take bool arguments
	cl_int err; // error code

	/* following two calls to clSetKernelArg can be moved to fft.c, since the arguments are
	 * constant. However, this requires setting auxiliary variables redfft and ndcomp as globals,
	 * since the kernel is called below.
	 */
	err=clSetKernelArg(clarith3,8,sizeof(cl_long),&ndcomp);
	checkErr(err,"set kernelargs at 8 of arith3");
	err=clSetKernelArg(clarith3,9,sizeof(cl_char),&redfft);
	checkErr(err,"set kernelargs at 9 of arith3");
	err=clSetKernelArg(clarith3,10,sizeof(cl_char),&transp);
	checkErr(err,"set kernelargs at 10 of arith3");
	// for arith2 and arith4
	const size_t gwsarith24[2]={boxY_st,boxZ_st};
	const size_t slicesize=gridYZ*3;
	// write into buffers eg upload to device
	err=clEnqueueWriteBuffer(command_queue,bufcc_sqrt,CL_TRUE,0,MAX_NMAT*3*2*sizeof(cl_double),
		cc_sqrt,0,NULL,NULL);
	checkErr(err,"writing cc_sqrt to device memory");
	err=clEnqueueWriteBuffer(command_queue,bufargvec,CL_TRUE,0,
		local_nvoid_Ndip*3*2*sizeof(cl_double),argvec,0,NULL,NULL);
	checkErr(err,"writing argvec to device memory");

	size_t xmsize=local_Nsmall*3;
	if (her) {
		err=clSetKernelArg(clnConj,0,sizeof(cl_mem),&bufargvec);
		checkErr(err,"set kernelargs at 0 of clnConj");
		err=clEnqueueNDRangeKernel(command_queue,clnConj,1,NULL,&local_Nsmall,NULL,0,NULL,NULL);
		checkErr(err,"Enqueueing kernel clnConj");
	}
	// setting (buf)Xmatrix with zeros (on device)
	err=clSetKernelArg(clzero,0,sizeof(cl_mem),&bufXmatrix);
	checkErr(err,"set kernelargs at 0 of clzero");
	err=clEnqueueNDRangeKernel(command_queue,clzero,1,NULL,&xmsize,NULL,0,NULL,NULL);
	checkErr(err,"Enqueueing kernel clzero");
	err=clEnqueueNDRangeKernel(command_queue,clarith1,1,NULL,&local_nvoid_Ndip,NULL,0,NULL,NULL);
	checkErr(err,"Enqueueing kernel clarith1");
	clFinish(command_queue); //wait till kernel executions are finished
#else
	// fill Xmatrix with 0.0
	for (i=0;i<3*local_Nsmall;i++) Xmatrix[i][RE]=Xmatrix[i][IM]=0.0;

	// transform from coordinates to grid and multiply with coupling constant
	if (her) nConj(argvec); // conjugated back afterwards

	for (i=0;i<local_nvoid_Ndip;i++) {
		// fill grid with argvec*sqrt_cc
		j=3*i;
		mat=material[i];
		index=IndexXmatrix(position[j],position[j+1],position[j+2]);
		for (Xcomp=0;Xcomp<3;Xcomp++) // Xmat=cc_sqrt*argvec
			cMult(cc_sqrt[mat][Xcomp],argvec[j+Xcomp],Xmatrix[index+Xcomp*local_Nsmall]);
	}
#endif
#ifdef PRECISE_TIMING
	GetTime(tvp+1);
	Elapsed(tvp,tvp+1,&Timing_Mult1);
#endif
	// FFT X
	fftX(FFT_FORWARD); // fftX Xmatrix
#ifdef PRECISE_TIMING
	GetTime(tvp+2);
	Elapsed(tvp+1,tvp+2,&Timing_FFTXf);
#endif
	BlockTranspose(Xmatrix,comm_timing);
#ifdef PRECISE_TIMING
	GetTime(tvp+3);
	Elapsed(tvp+2,tvp+3,&Timing_BTf);
#endif
	// following is done by slices
	for(x=local_x0;x<local_x1;x++) {
#ifdef PRECISE_TIMING
		GetTime(tvp+4);
#endif
#ifdef OPENCL
		err=clSetKernelArg(clarith2,7,sizeof(cl_long),&x);
		checkErr(err,"set kernelargs at 7 of clarith2");
		err=clSetKernelArg(clzero,0,sizeof(cl_mem),&bufslices);
		checkErr(err,"set kernelargs at 0 of clzero");
		err=clEnqueueNDRangeKernel(command_queue,clzero,1,NULL,&slicesize,NULL,0,NULL,NULL);
		checkErr(err,"Enqueueing kernel clzero");
		err=clEnqueueNDRangeKernel(command_queue,clarith2,2,NULL,gwsarith24,NULL,0,NULL,NULL);
		checkErr(err,"Enqueueing kernel clarith2");
		clFinish(command_queue);
#else
		// clear slice
		for(i=0;i<3*gridYZ;i++) slices[i][RE]=slices[i][IM]=0.0;

		// fill slices with values from Xmatrix
		for(y=0;y<boxY_st;y++) for(z=0;z<boxZ_st;z++) {
			i=IndexSliceYZ(y,z);
			j=IndexGarbledX(x,y,z);
			for (Xcomp=0;Xcomp<3;Xcomp++)
				cEqual(Xmatrix[j+Xcomp*local_Nsmall],slices[i+Xcomp*gridYZ]);
		}
#endif
#ifdef PRECISE_TIMING
		GetTime(tvp+5);
		ElapsedInc(tvp+4,tvp+5,&Timing_Mult2);
#endif
		// FFT z&y
		fftZ(FFT_FORWARD); // fftZ slices
#ifdef PRECISE_TIMING
		GetTime(tvp+6);
		ElapsedInc(tvp+5,tvp+6,&Timing_FFTZf);
#endif
		TransposeYZ(FFT_FORWARD);
#ifdef PRECISE_TIMING
		GetTime(tvp+7);
		ElapsedInc(tvp+6,tvp+7,&Timing_TYZf);
#endif
		fftY(FFT_FORWARD); // fftY slices_tr
#ifdef PRECISE_TIMING//
		GetTime(tvp+8);
		ElapsedInc(tvp+7,tvp+8,&Timing_FFTYf);
#endif//
#ifdef OPENCL
		// arith3 on Device
		err=clSetKernelArg(clarith3,11,sizeof(cl_long),&x);
		checkErr(err,"set kernelargs at 11 of arith3");
		// enqueueing kernel for arith3
		err=clEnqueueNDRangeKernel(command_queue,clarith3,2,NULL,gwsclarith3,NULL,0,NULL,NULL);
		checkErr(err,"Enqueueing kernel clarith3");
		clFinish(command_queue); //wait till kernel executions are finished
#else
		// arith3 on host
		// do the product D~*X~ 
		for(z=0;z<gridZ;z++) for(y=0;y<gridY;y++) {
			i=IndexSliceZY(y,z);
			for (Xcomp=0;Xcomp<3;Xcomp++)
				cEqual(slices_tr[i+Xcomp*gridYZ],xv[Xcomp]);

			j=IndexDmatrix_mv(x-local_x0,y,z,transposed);
			memcpy(fmat,Dmatrix[j],6*sizeof(doublecomplex));
			if (reduced_FFT) {
				if (y>smallY) {
					cInvSign(fmat[1]);               // fmat[1]*=-1
					if (z>smallZ) cInvSign(fmat[2]); // fmat[2]*=-1
					else cInvSign(fmat[4]);          // fmat[4]*=-1
				}
				else if (z>smallZ) {
					cInvSign(fmat[2]); // fmat[2]*=-1
					cInvSign(fmat[4]); // fmat[4]*=-1
				}
			}
			cSymMatrVec(fmat,xv,yv); // yv=fmat*xv
			for (Xcomp=0;Xcomp<3;Xcomp++)
				cEqual(yv[Xcomp],slices_tr[i+Xcomp*gridYZ]);
		}
#endif
#ifdef PRECISE_TIMING
		GetTime(tvp+9);
		ElapsedInc(tvp+8,tvp+9,&Timing_Mult3);
#endif
		// inverse FFT y&z
		fftY(FFT_BACKWARD); // fftY slices_tr
#ifdef PRECISE_TIMING //       
		GetTime(tvp+10);
		ElapsedInc(tvp+9,tvp+10,&Timing_FFTYb);
#endif
		TransposeYZ(FFT_BACKWARD);
#ifdef PRECISE_TIMING//
		GetTime(tvp+11);
		ElapsedInc(tvp+10,tvp+11,&Timing_TYZb);
#endif
		fftZ(FFT_BACKWARD); // fftZ slices
#ifdef PRECISE_TIMING
		GetTime(tvp+12);
		ElapsedInc(tvp+11,tvp+12,&Timing_FFTZb);
#endif
#ifdef OPENCL
		err=clSetKernelArg(clarith4,7,sizeof(cl_long),&x);
		checkErr(err,"set kernelargs at 7 of arith4");
		err=clEnqueueNDRangeKernel(command_queue,clarith4,2,NULL,gwsarith24,NULL,0,NULL,NULL);
		checkErr(err,"Enqueueing kernel clarith4");
		clFinish(command_queue);
#else
		//arith4 on host
		// copy slice back to Xmatrix
		for(y=0;y<boxY_st;y++) for(z=0;z<boxZ_st;z++) {
			i=IndexSliceYZ(y,z);
			j=IndexGarbledX(x,y,z);
			for (Xcomp=0;Xcomp<3;Xcomp++)
				cEqual(slices[i+Xcomp*gridYZ],Xmatrix[j+Xcomp*local_Nsmall]);
		}
#endif
#ifdef PRECISE_TIMING
		GetTime(tvp+13);
		ElapsedInc(tvp+12,tvp+13,&Timing_Mult4);
#endif
	} // end of loop over slices
	// FFT-X back the result
	BlockTranspose(Xmatrix,comm_timing);
#ifdef PRECISE_TIMING
	GetTime(tvp+14);
	Elapsed(tvp+13,tvp+14,&Timing_BTb);
#endif
	fftX(FFT_BACKWARD); // fftX Xmatrix
#ifdef PRECISE_TIMING
	GetTime(tvp+15);
	Elapsed(tvp+14,tvp+15,&Timing_FFTXb);
#endif
#ifdef OPENCL
	err=clEnqueueWriteBuffer(command_queue,bufresultvec,CL_TRUE,0,
		local_nvoid_Ndip*3*2*sizeof(cl_double),resultvec,0,NULL,NULL);
	checkErr(err,"writing resultvec to device memory");
	err=clEnqueueNDRangeKernel(command_queue,clarith5,1,NULL,&local_nvoid_Ndip,NULL,0,NULL,NULL);
	checkErr(err,"Enqueueing kernel clarith5");
	clFinish(command_queue);
	if (ipr) {
		/* calculating inner product in OpenCL is more complicated than usually. The norm for each
		 * element is calculated inside GPU, but the sum is taken by CPU afterwards. Hence,
		 * additional large buffers are required. Potentially, this can be optimized.
		 */
		err=clEnqueueNDRangeKernel(command_queue,clinprod,1,NULL,&local_nvoid_Ndip,NULL,0,NULL,
			NULL);
		checkErr(err,"Enqueueing kernel clinprod");
		err=clEnqueueReadBuffer(command_queue,bufinproduct,CL_TRUE,0,
			sizeof(cl_double)*local_nvoid_Ndip,inprodhlp,0,NULL,NULL);
		checkErr(err,"reading inprodhlp from device memory");
		// sum up on the CPU after calculating the norm on GPU
		for (j=0;j<local_nvoid_Ndip;j++) *inprod+=inprodhlp[j];
	}
	if (her) {
		err=clSetKernelArg(clnConj,0,sizeof(cl_mem),&bufresultvec);
		checkErr(err,"set kernelargs at 0 of clnConj");
		err=clEnqueueNDRangeKernel(command_queue,clnConj,1,NULL,&local_Nsmall,NULL,0,NULL,NULL);
		checkErr(err,"Enqueueing kernel clnConj");
	}
	clFinish(command_queue);
	err=clEnqueueReadBuffer(command_queue,bufresultvec,CL_TRUE,0,
		local_nvoid_Ndip*3*2*sizeof(cl_double),resultvec,0,NULL,NULL);
	checkErr(err,"reading resultvec from device memory");
#else
	// fill resultvec
	for (i=0;i<local_nvoid_Ndip;i++) {
		j=3*i;
		mat=material[i];
		index=IndexXmatrix(position[j],position[j+1],position[j+2]);
		for (Xcomp=0;Xcomp<3;Xcomp++) {
			cMult(cc_sqrt[mat][Xcomp],Xmatrix[index+Xcomp*local_Nsmall],temp);
			cAdd(argvec[j+Xcomp],temp,resultvec[j+Xcomp]); // result=argvec+cc_sqrt*Xmat
		}
		// norm is unaffected by conjugation, hence can be computed here
		if (ipr) *inprod+=cvNorm2(resultvec+j);
	}
	if (her) {
		nConj(resultvec);
		nConj(argvec); // conjugate back argvec, so it remains unchanged after MatVec
	}
#endif
#ifdef PRECISE_TIMING
	GetTime(tvp+16);
	Elapsed(tvp+15,tvp+16,&Timing_Mult5);
#endif
	if (ipr) MyInnerProduct(inprod,double_type,1,comm_timing);
#ifdef PRECISE_TIMING
	GetTime(tvp+17);
	Elapsed(tvp+16,tvp+17,&Timing_ipr);

	t_Mult1=TimerToSec(&Timing_Mult1);
	t_Mult2=TimerToSec(&Timing_Mult2);
	t_Mult3=TimerToSec(&Timing_Mult3);
	t_Mult4=TimerToSec(&Timing_Mult4);
	t_Mult5=TimerToSec(&Timing_Mult5);
	t_TYZf=TimerToSec(&Timing_TYZf);
	t_TYZb=TimerToSec(&Timing_TYZb);
	t_BTf=TimerToSec(&Timing_BTf);
	t_BTb=TimerToSec(&Timing_BTb);
	t_FFTXf=TimerToSec(&Timing_FFTXf);
	t_FFTXb=TimerToSec(&Timing_FFTXb);
	t_FFTYf=TimerToSec(&Timing_FFTYf);
	t_FFTYb=TimerToSec(&Timing_FFTYb);
	t_FFTZf=TimerToSec(&Timing_FFTZf);
	t_FFTZb=TimerToSec(&Timing_FFTZb);
	t_ipr=TimerToSec(&Timing_ipr);

	t_Arithm=t_Mult1+t_Mult2+t_Mult3+t_Mult4+t_Mult5+t_TYZf+t_TYZb;
	t_FFT=t_FFTXf+t_FFTYf+t_FFTZf+t_FFTXb+t_FFTYb+t_FFTZb;
	t_Comm=t_BTf+t_BTb+t_ipr;

	if (IFROOT) {
		PrintBoth(logfile,
			"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
			"                MatVec timing              \n"
			"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
			"Arith1 = "FFORMPT"    Arithmetics = "FFORMPT"\n"
			"FFTXf  = "FFORMPT"    FFT         = "FFORMPT"\n"
			"BTf    = "FFORMPT"    Comm        = "FFORMPT"\n"
			"Arith2 = "FFORMPT"\n"
			"FFTZf  = "FFORMPT"          Total = "FFORMPT"\n"
			"TYZf   = "FFORMPT"\n"
			"FFTYf  = "FFORMPT"\n"
			"Arith3 = "FFORMPT"\n"
			"FFTYb  = "FFORMPT"\n"
			"TYZb   = "FFORMPT"\n"
			"FFTZb  = "FFORMPT"\n"
			"Arith4 = "FFORMPT"\n"
			"BTb    = "FFORMPT"\n"
			"FFTXb  = "FFORMPT"\n"
			"Arith5 = "FFORMPT"\n"
			"InProd = "FFORMPT"\n\n",
			t_Mult1,t_Arithm,t_FFTXf,t_FFT,t_BTf,t_Comm,t_Mult2,
			t_FFTZf,DiffSec(tvp,tvp+16),t_TYZf,t_FFTYf,t_Mult3,t_FFTYb,t_TYZb,t_FFTZb,
			t_Mult4,t_BTb,t_FFTXb,t_Mult5,t_ipr);
		printf("\nPrecise timing is complete. Finishing execution.\n");
	}
	Stop(0);
#endif
	TotalMatVec++;
}

#else //ADDA_SPARSE is defined

inline static void AijProd(doublecomplex * restrict argvec, 
                           doublecomplex * restrict resultvec,
                           const int i, const int j)
{	
	static doublecomplex tmp1, argX, argY, argZ, resX, resY, resZ;
	static doublecomplex iterm[6];
	const unsigned int i3 = 3*i, j3 = 3*j;

	cMult(argvec[j3],cc_sqrt[material_full[j]][0],argX);
	cMult(argvec[j3+1],cc_sqrt[material_full[j]][1],argY);
	cMult(argvec[j3+2],cc_sqrt[material_full[j]][2],argZ);
	
	//D("%d %d %d %d %d %d %d %d", i, j, position[3*i], position[3*i+1], position[3*i+2], position[3*j], position[3*j+1], position[3*j+2]);
	CalcInterTerm(position[i3]-position_full[j3], position[i3+1]-position_full[j3+1], 
	              position[i3+2]-position_full[j3+2], iterm);
	
	cMult(argX,iterm[0],resX);
	cMult(argY,iterm[1],tmp1);
	cAdd(resX,tmp1,resX);
	cMult(argZ,iterm[2],tmp1);
	cAdd(resX,tmp1,resX);
	
	cMult(argX,iterm[1],resY);
	cMult(argY,iterm[3],tmp1);
	cAdd(resY,tmp1,resY);
	cMult(argZ,iterm[4],tmp1);
	cAdd(resY,tmp1,resY);
	
	cMult(argX,iterm[2],resZ);
	cMult(argY,iterm[4],tmp1);
	cAdd(resZ,tmp1,resZ);
	cMult(argZ,iterm[5],tmp1);
	cAdd(resZ,tmp1,resZ);
	
	cMult(resX,cc_sqrt[material[i]][0],argX);
	cAdd(argX,resultvec[i3],resultvec[i3]);
	cMult(resY,cc_sqrt[material[i]][1],argY);
	cAdd(argY,resultvec[i3+1],resultvec[i3+1]);
	cMult(resZ,cc_sqrt[material[i]][2],argZ);
	cAdd(argZ,resultvec[i3+2],resultvec[i3+2]);
}

/* 
The sparse MatVec is implemented completely separately from the non-sparse version.
Although there is some code duplication, this probably makes the both versions easier
to maintain.
*/

void MatVec (doublecomplex * restrict argvec,    // the argument vector
             doublecomplex * restrict resultvec, // the result vector
             double *inprod,         // the resulting inner product
             const bool her,         // whether Hermitian transpose of the matrix is used
             TIME_TYPE *comm_timing) // this variable is incremented by communication time
{	
	const bool ipr = (inprod != NULL);

	if (her) {
		for (unsigned int j=0; j<nlocalRows; j++) {
			argvec[j][IM] = -argvec[j][IM];		 
		}
	}	
#ifdef PARALLEL	
	SyncArgvec(argvec);
#else
	arg_full = argvec;	
#endif	

	/*
	printf("%u: nvoid_Ndip: %d\n", ringid, (int)nvoid_Ndip);	
	for (unsigned int i=0; i<3*(int)nvoid_Ndip; i++)
		printf("%u: %.4f+%.4fi ", i, arg_full[i][RE], arg_full[i][IM]);
	printf("\n");
	*/	
	
	for (unsigned int i=0; i<local_nvoid_Ndip; i++) {
		const unsigned int i3 = 3*i;		
		resultvec[i3][RE]=resultvec[i3][IM]=0.0;
		resultvec[i3+1][RE]=resultvec[i3+1][IM]=0.0;
		resultvec[i3+2][RE]=resultvec[i3+2][IM]=0.0;
		for (unsigned int j=0; j<local_d0+i; j++) {			
			AijProd(arg_full, resultvec, i, j);
		}		
		for (unsigned int j=local_d0+i+1; j<(unsigned int)nvoid_Ndip; j++) {
			AijProd(arg_full, resultvec, i, j);
		}
	}
	
	/*
	printf("%u: local_nvoid_Ndip: %d\n", ringid, (int)local_nvoid_Ndip);	
	for (unsigned int i=0; i<3*(int)local_nvoid_Ndip; i++)
		printf("%u: %.4f+%.4fi ", i, resultvec[i][RE], resultvec[i][IM]);
	printf("\n");
	*/
	
	const int local_c0 = 3*local_d0;
	for (unsigned int i=0; i<nlocalRows; i++) {
		cSubtr(arg_full[local_c0+i], resultvec[i], resultvec[i]);		 
	}
	
	if (her) {
		for (unsigned int i=0; i<nlocalRows; i++) {
			resultvec[i][IM] = -resultvec[i][IM];
			argvec[i][IM] = -argvec[i][IM];		
		}
	}	
		
	if (ipr) {
		*inprod = 0.0;
		for (unsigned int i=0; i<nlocalRows; i++) {
			*inprod += resultvec[i][RE]*resultvec[i][RE] + resultvec[i][IM]*resultvec[i][IM];		 
		}
		MyInnerProduct(inprod,double_type,1,comm_timing);
	}
	
	TotalMatVec++;
	
}

#endif //ADDA_SPARSE