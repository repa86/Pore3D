#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>
#include <limits.h>

#include "p3dFilt.h"
#include "p3dTime.h"

#define _USE_MATH_DEFINES
#include <math.h>

double _p3dKapurThresholding_entropy(double *h, int a, double p) {
	if (h[a] > 0.0 && p > 0.0)
		return -(h[a] / p * log((double) (h[a]) / p));
	return 0.0;
}


int p3dKapurThresholding_8(
	unsigned char* in_im,
	unsigned char* out_im,
	const int dimx,
	const int dimy,
	const int dimz,
	unsigned char* thresh,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) 
{

	double* prob;
	double *Pt, *F;
	double Hb, Hw;

	int i, j, ct, t;

	// Start tracking computational time:
	if (wr_log != NULL) {
		p3dResetStartTime();
		wr_log("Pore3D - Thresholding image according to Kapur's method...");
	}


	/* Allocate and initialize to zero kernel histogram: */
	P3D_MEM_TRY(Pt = (double *) malloc(sizeof (double) *(UCHAR_MAX + 1)));
	P3D_MEM_TRY(F = (double *) malloc(sizeof (double) *(UCHAR_MAX + 1)));
	P3D_MEM_TRY(prob = (double*) calloc((UCHAR_MAX + 1), sizeof (double)));

	/* Compute image histogram: */
	for (ct = 0; ct < (dimx * dimy * dimz); ct++)
		prob[in_im[ ct ]] = prob[in_im[ ct ]] + 1.0;

	/* Compute probabilities: */
	for (ct = 0; ct <= UCHAR_MAX; ct++)
		prob[ct] = prob[ct] / (double) (dimx * dimy * dimz);

	/* Compute the factors */
	Pt[0] = prob[0];
	for (i = 1; i <= UCHAR_MAX; i++)
		Pt[i] = Pt[i - 1] + prob[i];

	/* Calculate the function to be maximized at all levels */
	t = 0;
	for (i = 0; i <= UCHAR_MAX; i++) {
		Hb = Hw = 0.0;
		for (j = 0; j <= UCHAR_MAX; j++)
			if (j <= i)
				Hb += _p3dKapurThresholding_entropy(prob, j, Pt[i]);
			else
				Hw += _p3dKapurThresholding_entropy(prob, j, 1.0 - Pt[i]);

		F[i] = Hb + Hw;
		if (i > 0 && F[i] > F[t]) t = i;
	}

	*thresh = (unsigned char) t;



#pragma omp parallel for
	for (ct = 0; ct < (dimx * dimy * dimz); ct++)
		out_im[ ct ] = (in_im[ ct ] > (*thresh)) ? OBJECT : BACKGROUND;

	// Free memory:
	if (prob != NULL) free(prob);
	if (Pt != NULL) free(Pt);
	if (F != NULL) free(F);

	// Print elapsed time (if required):
	if (wr_log != NULL) {
		wr_log("\tDetermined threshold: %d.", *thresh);
		wr_log("Pore3D - Image thresholded successfully in %dm%0.3fs.", p3dGetElapsedTime_min(), p3dGetElapsedTime_sec());
	}

	// Return success:
	return P3D_SUCCESS;


MEM_ERROR:

	if (wr_log != NULL) {
		wr_log("Pore3D - Not enough (contiguous) memory. Program will exit.");
	}

	// Free memory:
	if (prob != NULL) free(prob);
	if (Pt != NULL) free(Pt);
	if (F != NULL) free(F);

	// Return error:
	return P3D_ERROR;

}

int p3dKapurThresholding_16(
	unsigned short* in_im,
	unsigned char* out_im,
	const int dimx,
	const int dimy,
	const int dimz,
	unsigned short* thresh,
	int (*wr_log)(const char*, ...),
	int (*wr_progress)(const int, ...)
	) 
{

	double* prob;
	double *Pt, *F;
	double Hb, Hw;

	int i, j, ct, t;


	// Start tracking computational time:
	if (wr_log != NULL) {
		p3dResetStartTime();
		wr_log("Pore3D - Thresholding image according to Kapur's method...");
	}


	/* Allocate and initialize to zero kernel histogram: */
	P3D_MEM_TRY(Pt = (double *) malloc(sizeof (double) *(USHRT_MAX + 1)));
	P3D_MEM_TRY(F = (double *) malloc(sizeof (double) *(USHRT_MAX + 1)));
	P3D_MEM_TRY(prob = (double*) calloc((USHRT_MAX + 1), sizeof (double)));

	/* Compute image histogram: */
	for (ct = 0; ct < (dimx * dimy * dimz); ct++)
		prob[in_im[ ct ]] = prob[in_im[ ct ]] + 1.0;

	/* Compute probabilities: */
	for (ct = 0; ct <= USHRT_MAX; ct++)
		prob[ct] = prob[ct] / (double) (dimx * dimy * dimz);

	/* Compute the factors */
	Pt[0] = prob[0];
	for (i = 1; i <= USHRT_MAX; i++)
		Pt[i] = Pt[i - 1] + prob[i];

	/* Calculate the function to be maximized at all levels */
	t = 0;
	for (i = 0; i <= USHRT_MAX; i++) {
		Hb = Hw = 0.0;
		for (j = 0; j <= USHRT_MAX; j++)
			if (j <= i)
				Hb += _p3dKapurThresholding_entropy(prob, j, Pt[i]);
			else
				Hw += _p3dKapurThresholding_entropy(prob, j, 1.0 - Pt[i]);

		F[i] = Hb + Hw;
		if (i > 0 && F[i] > F[t]) t = i;
	}



	*thresh = (unsigned short) t;



#pragma omp parallel for
	for (ct = 0; ct < (dimx * dimy * dimz); ct++)
		out_im[ ct ] = (in_im[ ct ] > (*thresh)) ? OBJECT : BACKGROUND;

	// Free memory:
	if (prob != NULL) free(prob);
	if (Pt != NULL) free(Pt);
	if (F != NULL) free(F);

	// Print elapsed time (if required):
	if (wr_log != NULL) {
		wr_log("\tDetermined threshold: %d.", *thresh);
		wr_log("Pore3D - Image thresholded successfully in %dm%0.3fs.", p3dGetElapsedTime_min(), p3dGetElapsedTime_sec());
	}

	// Return success:
	return P3D_SUCCESS;


MEM_ERROR:

	if (wr_log != NULL) {
		wr_log("Pore3D - Not enough (contiguous) memory. Program will exit.");
	}

	// Free memory:
	if (prob != NULL) free(prob);
	if (Pt != NULL) free(Pt);
	if (F != NULL) free(F);

	// Return error:
	return P3D_ERROR;

}
