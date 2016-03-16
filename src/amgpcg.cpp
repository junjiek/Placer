/* The AMG preconditioned FCG algorithm
 * Created by lizuowei, 2011.2.25
 * lizuoweirain@163.com
 * Copyright to EDA lab, tsinghua university
 * 
 * All the work is referred to "AN AGGREGATION-BASED ALGEBRAIC 
 * MULTIGRID METHOD" written by YVAN NOTAY, 2010
 */

#include <iostream>
#include <cmath>
#include "define.h"

//the max level we can construct
#define MAXLEV 40

//we can directly solve it when the size n is less than 400
int maxcoarsesize = 400;

//
double resi = 0.25;

//
double checkdd = 5.0;

//
double trswc = 0.25;

//
double trspos = 0.45;

//
double scalcg = 1.0;

//indicate that the memory alloc in reclusive function
int nwrkcum = 0;

//the important data we use for the multi-level matrix
typedef struct InnerData_
{
	double *a;
	int *ja;
	int *ia;

	double *p;  //we use for the inverse of diagonal element
	int *idiag; //we use for the index of diagonal element 
	int *ind;   //we use for the index of aggregate G
} InnerData;

InnerData dt[MAXLEV];

//nn record the size in each level
int nn[MAXLEV]; 

//controls whether we use V-cycle or K-cycle in precondition.
int innermax[MAXLEV];

//nlev record how many levels we have
int nlev; 


void test(int n, double *p)
{
	for(int i = 0; i < n; i++) {
		printf("%f\n", p[i]);
	}
}

void test(int n, int *p)
{
	for(int i = 0; i < n; i++) {
		printf("%d\n", p[i]);
	}
}

//------------------------------------------------------------------------
//some vector functions
double dnorm2(int n, double *p)
{
	int i;
	double sum = 0;
	for(i=0; i<n; i++) {
		sum += p[i] * p[i];
	}
	return sum;
}

double ddot(int n, double *p, double *q)
{
	int i;
	double sum = 0;
	for(i=0; i<n; i++) {
		sum += p[i] * q[i];
	}
	return sum;
}

//y  <--  alpha*x + y
void daxpy(int n, double alpha, double *x, double *y)
{
	int i;
	for(i=0; i<n; i++) {
		y[i] += alpha * x[i];
	}
}

//x <-- alpha * x
void dscal(int n, double alpha, double *x)
{
	int i;
	for(i=0; i<n; i++) {
		x[i] *= alpha;
	}
}

void init()
{
	int l;
	for(l = 0; l < MAXLEV; l++) {
		dt[l].a = NULL;
		dt[l].ja = NULL;
		dt[l].ia = NULL;
		dt[l].idiag = NULL;
		dt[l].p = NULL;
		dt[l].ind = NULL;
	}
}

//This routine releases the memory we alloc in each level
void release()
{
	int l;
	for(l = 0; l <= nlev; l++) {
	//	if(l > 0) {
	//		if(nn[l-1] > 0) {
				FREE(dt[l].a);
				FREE(dt[l].ja);
				FREE(dt[l].ia);
	//		}
	//	}
	//	if(l < nlev-1 && nn[l] > 0) {
			FREE(dt[l].p);
			FREE(dt[l].ind);
			FREE(dt[l].idiag);
	//	}
	}
}

//This routine calculates the matrix vector product y = Ax
void mxvs(int n, double *a, int *ja, int *ia, double *x, double *y)
{
	int i, kk, k1, k2;
	register double t;
	for(i = 0; i < n; i++) {
		k1 = ia[i];
		k2 = ia[i+1];
		t = a[k1] * x[ja[k1]];
		for(kk = k1+1; kk < k2; kk++) {
			t = t + a[kk] * x[ja[kk]];
		}
		y[i] = t;
	}
}

//This routine calculates the residual r = b - Ax
void rescalc(int n, double *a, int *ja, int *ia, double *b, double *x, double *r)
{
	int i, kk, k1, k2;
	register double t;

	for(i = 0; i < n; i++) {
		k1 = ia[i];
		k2 = ia[i+1];
		t = b[i] - a[k1] * x[ja[k1]];
		for(kk = k1 + 1; kk < k2; kk++) {
			t = t - a[kk] * x[ja[kk]];
		}
		r[i] = t;
	}
}

//This routine reconstruct the matrix A
//w : the work vector
//after call this routine, p record the inverse of the diagonal entry, 
//  idiag record the index the diagonal entry in a
void setsgs(int n, double *a, int *ja, int *ia,
			double *p, int *idiag, double *w, int *iw)
{
	int i, j, k, ipos, nzu;
	register double t;
	for(i = 0; i < n; i++) {
		//find diagonal, save upper part, concatenate lower part
		ipos = ia[i];
		nzu = 0;
		for(k = ia[i]; k < ia[i+1]; k++) {
			j = ja[k];
			if(j > i) {
				w[nzu] = a[k];
				iw[nzu] = j;
				nzu++;
			}
			else if(j < i) {
				a[ipos] = a[k];
				ja[ipos] = j;
				ipos++;
			}
			else {
				p[i] = a[k];
			}
		}

		//copy back diagonal entry
		idiag[i] = ipos;
		a[ipos] = p[i];
		ja[ipos] = i;

		//copy back upper part
		for(k = 0; k < nzu; k++) {
			ipos++;
			a[ipos] = w[k];
			ja[ipos] = iw[k];
		}

		//save inverse in vector p
		t = p[i];
		p[i] = 1 / t;
	}
}

//This routine performs a direct solver Ax = b using cholesky factory
//the size of A is relatively small, such as smaller than maxcoarsesize(=400)
//x contains the right hand values initially and return the solution
void cholmod(int n, double *a, int *ja, int *ia, double *x, int job)
{
	int i, j, k;
	static double **ac = NULL;
	static double *y = NULL;

	if(job == 0) {  //prepare data
		ac = ALLOC(double*, n);
		for(i = 0; i < n; i++) {  //must be sure that n is small
			ac[i] = ALLOC(double, n);
			ZERO(ac[i], double, n);
		}
		y = ALLOC(double, n);

		//reconstruct matrix A
		for(i = 0; i < n; i++) {
			for(k = ia[i]; k < ia[i+1]; k++) {
				ac[i][ja[k]] = a[k];
			}
		}

		//cholesky factory
		for(k = 0; k < n; k++) {
			ac[k][k] = sqrt(ac[k][k]);
			for(i = k+1; i < n; i++) {
				ac[i][k] = ac[i][k] / ac[k][k];
			}
			for(j = k+1; j < n; j++) {
				for(i = j; i < n; i++) {
					ac[i][j] = ac[i][j] - ac[i][k] * ac[j][k];
				}
			}
		}
	}
	else if(job > 0) { //solve
		//solve Ly = x;
		for(j = 0; j < n; j++) {
			y[j] = x[j] / ac[j][j];
			for(i = j+1; i < n; i++) {
				x[i] -= ac[i][j] * y[j];
			}
		}
		//solve L'x = y
		for(j = n-1; j > -1; j--) {
			x[j] = y[j] / ac[j][j];
			for(i = 0; i < j; i++) {
				y[i] -= ac[j][i] * x[j];
			}
		}
	}
	else { //release data
		for(i = 0; i < nn[nlev]; i++) {
			FREE(ac[i]);
		}
		FREE(ac);
		FREE(y);
	}
}

//This routine performs the prolongation step in standard AMG
//v = P(f x c) * b
void prolongate(int n, int nc, double *v, double *b, int *ind)
{
	int i, k;
	for(i = 0; i < n; i++) {
		k = ind[i];
		if(k < nc) {
			v[i] = b[k];
		}
		else {
			v[i] = 0;
		}
	}
}

//This routine performs the restriction step in standard AMG
//b = P(c x f) * v
void restrict(int n, int nc, double *v, double *b, int *ind)
{
	int i, k;
	ZERO(b, double, nc);
	for(i = 0; i < n; i++) {
		k = ind[i];
		if(k < nc) {
			b[k] = b[k] + v[i];
		}
	}
}

//This routine performs the pre-smooth step 
//and compute new residual in standard AMG
void pre_smooth(int n, double *b, double *t1, double *r, double *v, double *a,
				int *ja, int *ia, double *p, int *idiag)
{
	int j, kk, k2;
	register double t;
	
	//v = b - low(A) * t1; Not include diag elements;
	//t1 = p * v; p is the inverse of diag elements;
	t1[0] = p[0] * b[0];
	v[0] = b[0];
	for(kk = 1; kk < n; kk++) {
		t = b[kk];
		for(j = ia[kk]; j < idiag[kk]; j++) {
			t = t - a[j] * t1[ja[j]];
		}
		v[kk] = t;
		t1[kk] = p[kk] * t;
	}

	//printf("%.50lf\n", dnorm2(n, t1));exit(0);
	//t = -upp(A) * t1;
	//t1 = t1 + p * t;
	for(kk = n-2; kk > -1; kk--) {
		t = 0;
		k2 = ia[kk+1];
		for(j = idiag[kk] + 1; j < k2; j++) {
			t = t - a[j] * t1[ja[j]];
		}
		t1[kk] = t1[kk] + p[kk] * t;
	}

	//r = b-v-low(A)*t1;
	r[0] = b[0] - v[0];
	for(kk = 1; kk < n; kk++) {
		t = b[kk] - v[kk];
		for(j = ia[kk]; j < idiag[kk]; j++) {
			t = t - a[j] * t1[ja[j]];
		}
		r[kk] = t;
	}
}

//This routine performs the post-smooth step in standard AMG
void post_smooth(int n, double *x, double *t1, double *t3, double *a,
				 int *ja, int *ia, double *p, int *idiag)
{
	int kk, j, k2;
	register double t;

	//input : x = t2; converted to t1+t2
	//output : x = t1 + t2 + t3

	x[n-1] = x[n-1] + t1[n-1];
	t3[n-1] = 0;
	for(kk = n - 2; kk > -1; kk--) {
		x[kk] = x[kk] + t1[kk];
		t = 0;
		for(j = idiag[kk] + 1; j < ia[kk + 1]; j++) {
			t = t + a[j] * x[ja[j]];
		}
		t3[kk] = t;
	}

	//
	t3[0] = p[0] * t3[0];
	for(kk = 1; kk < n; kk++) {
		t = t3[kk];
		for(j = ia[kk]; j < idiag[kk]; j++) {
			t = t - a[j] * t3[ja[j]];
		}
		t3[kk] = p[kk] * t;
	}

	//
	for(kk = 0; kk < n; kk++) {
		t3[kk] += x[kk];
	}

	//
	for(kk = n - 2; kk > -1; kk--) {
		t = 0;
		k2 = ia[kk+1];
		for(j = idiag[kk] + 1; j < k2; j++) {
			t = t - a[j] * t3[ja[j]];
		}
		t3[kk] += p[kk] * t;
	}

	//
	for(kk = 0; kk < n; kk++) {
		x[kk] = x[kk] + t1[kk] - t3[kk];
	}
}


void precond(int n, double *x, double *b, double *a, int *ja, int *ia, int l, double *t);

//This routine performs the fcg inner op. of precondition
void fcg_inner(int n, double *x, double *r, int l)
{
	double resid, bnorm;
	double alpha1, alpha2, bet0, rho1, rho2, gamm0;

	//at most 2 iterations
	//the first iteration
	bnorm = sqrt(dnorm2(n, r));

	precond(n, x, r, dt[l].a, dt[l].ja, dt[l].ia, l, r+n);
	mxvs(n, dt[l].a, dt[l].ja, dt[l].ia, x, r+n);
	
	rho1 = ddot(n, x, r+n);
	alpha1 = ddot(n, x, r);

	bet0 = alpha1 / rho1;
	daxpy(n, -bet0, r+n, r);
	resid = dnorm2(n, r);
	resid = sqrt(resid) / bnorm;
	
	if(resid <= resi) {
		dscal(n, bet0, x);
		return;
	}

	//the second iteration
	precond(n, r+2*n, r, dt[l].a, dt[l].ja, dt[l].ia, l, r+3*n);  //r+3*n or r
	mxvs(n, dt[l].a, dt[l].ja, dt[l].ia, r+2*n, r+3*n);

	gamm0 = ddot(n, r+n, r+2*n);
	alpha2 = ddot(n, r+2*n, r);
	rho2 = ddot(n, r+2*n, r+3*n);
	rho2 = rho2 - gamm0 * gamm0 / rho1;
	bet0 = (alpha1 - alpha2*gamm0/rho2) / rho1;

	dscal(n, bet0, x);
	bet0 = alpha2 / rho2;
	daxpy(n, bet0, r+2*n, x);
}

//This routine performs the coarse-grid correction step in the AMG
//include : restriction, compute an approximate solution of the residual function,
//prolongate coarse-grid correction
void cg_correct(int n, double *x, double *r, int l, double *w)
{
	int i;
	int l1 = l + 1;
	int nnext = nn[l1];

	if(nnext > 0) {
		restrict(n, nnext, r, w+nnext, dt[l].ind);    //r  -->  w
		if(l1 >= nlev) {
			for(i = 0; i < nnext; i++) {
				w[i] = w[i+nnext];
			}
			cholmod(nnext, dt[l1].a, dt[l1].ja, dt[l1].ia, w, 1);
		}
		else if(innermax[l1] <= 1) {  //V-cycle
			precond(nnext, w, w+nnext, dt[l1].a, dt[l1].ja, dt[l1].ia, l1, w+nnext+nnext);
		}
		else { //K-cycle
			fcg_inner(nnext, w, w+nnext, l1); //get w
		}
		prolongate(n, nnext, x, w, dt[l].ind);  //w  -->  x
	}
	else {
		ZERO(x, double, n);
	}
}

//This routine performs a AMG preconditioner
void precond(int n, double *x, double *b, double *a, int *ja, int *ia, int l, double *t)
{
	pre_smooth(n, b, t+n, t, x, a, ja, ia, dt[l].p, dt[l].idiag);
	cg_correct(n, x, t, l, t+2*n);
	post_smooth(n, x, t+n, t, a, ja, ia, dt[l].p, dt[l].idiag);
}

//This routine performs simple pairwise aggregation
void sp_aggre(int n, int &nc, double *a, int *ja, int *ia, int *lcg,
			  int *ind, int *lpair, int *deg, int *next, int *prev, 
			  int *first, double *odmax, double trs, double checkdd,
			  double trspos, int &maxdg)
{
	double vald = 0.0;
	double val, tent, odm, valp;
	int mindg, i, j, jj, k, ipair, dg, isel, nmark, nm1, nm2;

	nmark = 0;
	nm1 = -n - 1;
	nm2 = -n - 2;
	maxdg = 0;
	nc = 0;

	if(checkdd > 0) {
		for(i = 0; i < n; i++) {
			j = ia[i];
			jj = ia[i+1] - 1;
			dg = jj - j;
			val = 0;
			odm = 0;
			valp = 0;
			for(k = j; k <= jj; k++) {
				if(ja[k] == i) {
					vald = a[k];
				}
				else {
					odm = MAX(odm, ABS(a[k]));
					valp = MAX(valp, a[k]);
					val += ABS(a[k]);
				}
			}
			if(dg == 0 || ABS(vald) > checkdd * val) {  // checkdd = 5.0
				ind[i] = nm1;
				nmark ++;
				odmax[i] = 0;
				deg[i] = nm1;
			}
			else if(valp > trspos * vald) {
				ind[i] = nm2;
				deg[i] = 0;
				odmax[i] = - trs * odm;
			}
			else {
				ind[i] = -1;
				deg[i] = 0;
				odmax[i] = - trs * odm;
			}
		}
	}
	else {
		for(i = 0; i < n; i++) {
			j = ia[i];
			jj = ia[i+1] - 1;
			dg = jj - j;
			odm = 0;
			valp = 0;
			for(k = j; k <= jj; k++) {
				if(ja[k] == i) {
					vald = a[k];
				}
				else {
					odm = MAX(odm, ABS(a[k]));
					valp = MAX(valp, a[k]);
				}
			}
			if(dg == 0) {
				ind[i] = nm1;
				nmark ++;
				odmax[i] = 0;
				deg[i] = nm1;
			}
			else if(valp > trspos * vald) {
				ind[i] = nm2;
				deg[i] = 0;
			}
			else {
				ind[i] = -1;
				deg[i] = 0;
				odmax[i] = - trs * odm;
			}
		}
	}

	for(i = 0; i < n; i++) {
		for(k = ia[i]; k < ia[i+1]; k++) {
			j = ja[k];
			if((ind[i] == -1 || ind[i] == nm2) && a[k] < odmax[i] && j != i) {
				dg = deg[j] + 1;
				deg[j] = dg;
				maxdg = MAX(maxdg, dg);
			}
		}
	}

	for(i = 0; i <= maxdg; i++) {
		first[i] = -1;
	}

	for(i = n-1; i > -1; i--) {
		if(ind[i] == -1 || ind[i] == nm2) {
			dg = deg[i];
			if(first[dg] > -1) {
				prev[first[dg]] = i;
			}
			next[i] = first[dg];
			prev[i] = -1;
			first[dg] = i;
		}
	}

	while(nmark < n) {
		mindg = -1;
		ipair = -1;
		while(ipair == -1) {
			mindg++;
			if(first[mindg] > -1) {
				ipair = first[mindg];
				first[mindg] = next[ipair];
				if(next[ipair] > -1) {
					prev[next[ipair]] = -1;
				}
			}
		}

		ind[ipair] = nc;
		isel = -1;

		if(ind[ipair] == nm2) {
			goto aaa;
		}

		val = 0;
		for(i = ia[ipair]; i < ia[ipair+1]; i++) {
			j = ja[i];
			if(ind[j] == -1 && a[i] < odmax[ipair]) {
				tent = a[i];
				if(tent < 1.0001 * val) {
					isel = j;
					val = tent;
				}
			}
		}

aaa:    
		if(isel == -1) {
			lcg[nc] = ipair;
			lpair[nc] = -1; 
			nmark++;
		}
		else {
			ind[ipair] = nc; 
			ind[isel] = nc;  
			lcg[nc] = isel;
			lpair[nc] = ipair;
			nmark = nmark + 2;
			dg = deg[isel];

			if(prev[isel] > -1) {
				next[prev[isel]] = next[isel];
			}
			else {
				first[dg] = next[isel];
			}

			if(next[isel] > -1) {
				prev[next[isel]] = prev[isel];
			}

			for(i = ia[isel]; i < ia[isel+1]; i++) {
				j = ja[i];
				if((ind[j] == -1 || ind[j] == nm2) && a[i] < odmax[isel]) {
					dg = deg[j];
					dg--;
					deg[j] = dg;
					if(prev[j] > -1) {
						next[prev[j]] = next[j];
					}
					else {
						first[dg+1] = next[j];
					}
					if(next[j] > -1) {
						prev[next[j]] = prev[j];
					}
					if(first[dg] > -1) {
						prev[first[dg]] = j;
					}
					next[j] = first[dg];
					prev[j] = -1;
					first[dg] = j;
				}
			}
		}

		for(i = ia[ipair]; i < ia[ipair+1]; i++) {
			j = ja[i];
			if((ind[j] == -1 || ind[j] == nm2) && a[i] < odmax[ipair]) {
				dg = deg[j];
				dg--;
				deg[j] = dg;
				if(prev[j] > -1) {
					next[prev[j]] = next[j];
				}
				else {
					first[dg+1] = next[j];
				}
				if(next[j] > -1) {
					prev[next[j]] = prev[j];
				}
				if(first[dg] > -1) {
					prev[first[dg]] = j;
				}
				next[j] = first[dg];
				prev[j] = -1;
				first[dg] = j;
			}
		}
		nc++;
	}
}

//This routine performs combination of lcg1 and lcg2
void setlcg(int n, int nc, int *lcg, int *lcg1, int *lpair1, int *lcg2, int *lpair2)
{
	int i, j1, j2;

	for(i = 0; i < nc; i++) {
		j1 = lcg2[i];
		j2 = lpair2[i];
		lcg[i] = lcg1[j1];
		lcg[n+i] = lpair1[j1];
		if(j2 > -1) {
			lcg[2*n+i] = lcg1[j2];
			lcg[3*n+i] = lpair1[j2];
		}
		else {
			lcg[2*n+i] = -1;
			lcg[3*n+i] = -1;
		}
	}
}

//This routine performs double pairwise aggregation
int dp_aggre(int n, int &nc, double *a, int *ja, int *ia, int *lcg, int *ind, int *lcg1,
				int *lpair1, int *lcg2, int *lpair2, double *odmax, double trs, double *a2,
				int *ja2, int* ia2, double checkdd, double trspos)
{
	int nc1 = 0;
	int maxdg = 0;
	int *ifirst = NULL;
	int i, jj, jc, jcol, kb, jpos, nz;

	sp_aggre(n, nc1, a, ja, ia, lcg1, ind, lpair1, lcg2, 
		lpair2, ia2, lcg, odmax, trs, checkdd, trspos, maxdg);

	//printf("%d\n", nc1);
	//exit(0);

	nz = 0;
	ia2[0] = 0;
	for(i = 0; i < nc1; i++) {
		lcg[i] = -1;
	}
	for(i = 0; i < nc1; i++) {
		jj = lcg1[i];
		for(kb = ia[jj]; kb < ia[jj+1]; kb++) {
			jcol = ja[kb];  
			jcol = ABS(ind[jcol]);
			if(jcol < nc1) {
				jpos = lcg[jcol];
				if(jpos == -1) {
					ja2[nz] = jcol;
					lcg[jcol] = nz;
					a2[nz] = a[kb];
					nz++;
				}
				else {
					a2[jpos] += a[kb];
				}
			}
		}

		jj = lpair1[i];
		if(jj > -1) {
			for(kb = ia[jj]; kb < ia[jj+1]; kb++) {
				jcol = ja[kb];
				jcol = ABS(ind[jcol]);
				if(jcol < nc1) {
					jpos = lcg[jcol];
					if(jpos == -1) {
						ja2[nz] = jcol;
						lcg[jcol] = nz;
						a2[nz] = a[kb];
						nz++;
					}
					else {
						a2[jpos] = a2[jpos] + a[kb];
					}
				}
			}
		}
		for(kb = ia2[i]; kb < nz; kb++) {
			lcg[ja2[kb]] = -1;
		}
		ia2[i+1] = nz;
	}

	if(maxdg + 1 <= ia[n] - nz - 1) {
		sp_aggre(nc1, nc, a2, ja2, ia2, lcg2, lcg, lpair2, lcg+n, lcg+2*n, lcg+3*n, 
			ja2+nz, odmax, trs, -1.0, trspos, maxdg);
	}
	else {
		ifirst = ALLOC(int, maxdg+1);
		sp_aggre(nc1, nc, a2, ja2, ia2, lcg2, lcg, lpair2, lcg+n, lcg+2*n, lcg+3*n, 
			ifirst, odmax, trs, -1.0, trspos, maxdg);
		FREE(ifirst);
	}
	for(i = 0; i < n; i++) {
		jc = ind[i];
		jcol = ABS(jc);
		if(jcol < nc1) {
			if(jc > -1) {
				ind[i] = lcg[jcol];
			}
			else {
				ind[i] = -ABS(lcg[jcol]);
			}
		}
	}
	setlcg(n, nc, lcg, lcg1, lpair1, lcg2, lpair2);
	
	return 0;
}

//This routine fill the information with the matrix of the coarse level
void setacg(int n, int nc, int *lcg, double *a, int *ja, int *ia, double *ac,
			int *jac, int *iac, int &nzac, int *ind, int *iw)
{
	int i, kk, jj, jc, kb, jcol, jpos;
	
	for(i = 0; i < nc; i++) {
		iw[i] = -1;
	}

	nzac = 0;
	iac[0] = 0;
	for(i = 0; i < nc; i++) {
		for(kk = 0; kk < 4; kk++) {
			jj = lcg[kk*n + i];
			if(jj != -1) {
				for(kb = ia[jj]; kb < ia[jj+1]; kb++) {
					jc = ja[kb];
					jcol = ABS(ind[jc]);
					if(jcol < nc) {
						jpos = iw[jcol];
						if(jpos == -1) {
							jac[nzac] = jcol;
							iw[jcol] = nzac;
							ac[nzac] = a[kb];
							nzac++;
						}
						else {
							ac[jpos] = ac[jpos] + a[kb];
						}
					}
				}
			}
		}

		for(kb = iac[i]; kb < nzac; kb++) {
			iw[jac[kb]] = -1;
		}
		iac[i+1] = nzac;
	}
}

//This routine setups all the level we need
void setup(int l, int n, double *a, int *ja, int *ia)
{
	int nnext, nzan, i, j, lw, lw0, nwprev;
	int *iw, *iw0, *ja2, *lcg;
	double *a2;
	double ff, xsi, eta, checkddl;
	int nnz;
	static int last_nnz, init_nnz, last_size, init_size;
	static int icum;
	static int slowcoarse = 0;

	nn[l] = n;
	nnz = ia[n] - ia[0];

	if(l > 0) {
		xsi = 0.6f;
		eta = 0.5f;
		ff = (init_nnz / nnz) * pow(xsi, l) / icum;
		if(ff < 2.0 - eta) {
			innermax[l] = 1;
		}
		else {
			innermax[l] = 2;
		}
		icum = icum * innermax[l];
	}
	else {
		init_size = n;
		init_nnz = nnz;
		icum = 1;
		innermax[0] = 1;
		slowcoarse = 0;
	}

	if(l >= MAXLEV || n <= maxcoarsesize || 
		(slowcoarse && 3*last_size < 5*n) || 
		last_size == n) {
		nlev = l;
		innermax[l] = 0;
	}

	slowcoarse = 0;

	if(l > 0 && 3*last_size < 5*n) {
		slowcoarse = 1;
	}

	last_size = n;
	last_nnz = nnz;

	lw = 0;
	lw0 = 0;

	if(l == nlev) {
		goto G1;
	}

	lw = 3*n + 1;
	lw0 = n;

	dt[l].p = ALLOC(double, n);
	dt[l].ind = ALLOC(int, n);
	dt[l].idiag = ALLOC(int, n+1);
	lcg = ALLOC(int, 4*n);
	a2 = ALLOC(double, nnz);
	ja2 = ALLOC(int, nnz);
	iw = ALLOC(int, lw);
	iw0 = ALLOC(int, lw0);

	if(dt[l].p == NULL || dt[l].ind == NULL || dt[l].idiag == NULL 
		|| lcg == NULL || a2 == NULL || ja2 == NULL || iw == NULL || iw0 == NULL) {
		printf("[ERROR] : Cann't alloc enough memory in setup!\n");
		return;
	}
	
	if(l > 0) {
		checkddl = -1.0;
	}
	else {
		checkddl = checkdd; //5.0
	}

	//here we use dt[l].idiag, iw0, iw as the 
	//auxiliary memory in the function below
	dp_aggre(n, nnext, a, ja, ia, lcg, dt[l].ind, dt[l].idiag, iw0, 
		iw, iw+n, dt[l].p, trswc, a2, ja2, iw+n+n, checkddl, trspos);

G1:	
	if(lw > 0) {
		FREE(iw);
	}
	lw = 0;

	if(l == nlev) {
		goto G2;
	}

	setsgs(n, a, ja, ia, dt[l].p, dt[l].idiag, a2, ja2);

	dt[l+1].ia = ALLOC(int, nnext+1);
	j = nnext;

	if(j > lw0) {
		if(lw0 > 0) {
			FREE(iw0);
		}
		iw0 = ALLOC(int, j);
		lw0 = j;
	}

	setacg(n, nnext, lcg, a, ja, ia, a2, ja2, dt[l+1].ia, nzan, dt[l].ind, iw0);
	FREE(iw0);
	FREE(lcg);

	dt[l+1].a = ALLOC(double, nzan);
	dt[l+1].ja = ALLOC(int, nzan);

	if(dt[l+1].a == NULL || dt[l+1].ja == NULL) {
		printf("[ERROR] : Cann't alloc enough memory in setup!\n");
		return;
	}

	COPY(dt[l+1].a, a2, double, nzan);
	COPY(dt[l+1].ja, ja2, int, nzan);

	FREE(a2);
	FREE(ja2);

	//printf("[INFO] : level %d, size %d, nonzeros %d.\n", l+1, nnext, nzan);

	setup(l+1, nnext, dt[l+1].a, dt[l+1].ja, dt[l+1].ia);
	goto G3;

G2:
	//prepare data for directly solving the coarsest level
	cholmod(n, a, ja, ia, NULL, 0);

G3:
	//important for memory in recursive functions
	if(l == 0) {
		nwrkcum = 0;
		nwprev = 0;
		for(i = 1; i < nlev; i++) {
			if(innermax[i] > 1) {
				nwrkcum = nwrkcum + MAX(nwprev, 6*nn[i]);
				nwprev = nn[i];
			}
			else {
				nwrkcum = nwrkcum + MAX(nwprev, 4*nn[i]);
				nwprev = 0;
			}
		}
		nwrkcum = nwrkcum + MAX(nwprev, 2*nn[nlev]);
	}
}

//This routine performs the flexible cg algorithm with amg preconditioner
int fcg(int n, double *a, int *ja, int *ia, double *x,
		double *b, int init, int maxIter, double tol)
{
	//init = 0 : with no initial guess in x[1:n], x[1:n] = 0
	//init = 1 : we must identify x[1:n] before enter amgpcg

	int iter;
	double resid, bnorm;
	double alpha, beta, rho = 0;
	double *temp;

	//the space fcg would need 
	temp = ALLOC(double, nwrkcum+5*n);
	if(temp == NULL) {
		printf("[ERROR] : Cann't alloc enough memory in fcg!");
		return -1;
	}
	
	ZERO(temp, double, nwrkcum+5*n);

	resid = 1000.0;
	iter = 0;

	bnorm = sqrt(dnorm2(n, b));

	if(init == 1) { 
		//initial guess in x[1:n]
		rescalc(n, a, ja, ia, b, x, b);  //b = b - Ax
		resid = sqrt(dnorm2(n, b));
		resid = resid / bnorm;
		//printf("[INFO] : iter = %d, resid = %lf, resid/bnorm = %lf\n",
		//	iter, resid*bnorm, resid);
	}

	while(iter < maxIter && resid > tol) {
		
		iter++;

		//get temp+3*n, and temp+4*n is a auxiliary space
		precond(n, temp+2*n, b, a, ja, ia, 0, temp+3*n);
		//printf("------- %lf\n", dnorm2(n, temp+2*n)); exit(0);

		//compute direction vector
		if(iter > 1) {
			beta = -ddot(n, temp, temp+2*n) / rho;
			daxpy(n, beta, temp+n, temp+2*n);
		}	
		
		COPY(temp+n, temp+2*n, double, n);
		mxvs(n, a, ja, ia, temp+n, temp);

		rho = ddot(n, temp+n, temp);
		alpha = ddot(n, temp+n, b) / rho;

		if(iter == 1 && init == 0) {
			COPY(x, temp+n, double, n);
			dscal(n, alpha, x);
		}
		else {
			daxpy(n, alpha, temp+n, x);
		}

		daxpy(n, -alpha, temp, b);
		resid = sqrt(dnorm2(n, b));
		resid = resid / bnorm;

		//printf("[INFO] : iter = %d, resid = %lf, resid/bnorm = %lf\n",
		//	iter, resid*bnorm, resid);
	}

	FREE(temp);

	return 0;
}

//This main routine of the amg_pcg algorithm
void amgpcg(int n, double *a, int *ja, int *ia, double *b,
		   double *x, int maxIter, double tol, int initValue)
{
	init();

	//first we setup the levels
	nlev = -1;
	setup(0, n, a, ja, ia);

	if(nlev == 0) {
		//direct solve if only one level
		COPY(x, b, double, n);
		cholmod(n, a, ja, ia, x, 1);
	}
	else {
		//amg preconditioned fcg solve
		fcg(n, a, ja, ia, x, b, initValue, maxIter, tol);
	}

	cholmod(n, a, ja, ia, x, -1);
	release();

}

void testMatrix(int n, double *a, int *ja, int *ia)
{
	//test matrix
	FILE *fp = fopen("mm.txt", "w");
	for(int i = 0; i < n; i++) {
		for(int k = ia[i]; k < ia[i+1]; k++) {
			fprintf(fp, "%d %d %f\n", i, ja[k], a[k]);
		}
	}
	fclose(fp);
	//test end
}

