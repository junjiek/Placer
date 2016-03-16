/*
 * fpCG.cpp
 *
 *  Created on: Nov 26, 2013
 *      Author: zhouq
 */
/*#include <cmath>
#include <iostream>
#include "cg.h"
#include "bicgstab.h"
#include "ilupre_double.h"

using namespace std;


int fpCG(int * row, int * col, double *val, int numVals, int numRows, double *bb, double *temp)
{
    double tol = 10e-6;
    int result, maxit = 300;


    CompRow_Mat_double A(numRows, numRows, numVals, val, row, col);

    VECTOR_double b(bb, A.dim(1));
    VECTOR_double x(A.dim(1), 0.0);

    // this is experimentally the fastest
    //DiagPreconditioner_double M(A);
    // this one has best result but slower
    CompRow_ILUPreconditioner_double M(A);
    //ICPreconditioner_double M(A);
    result = CG(A, x, b, M, maxit, tol);
    //result = BiCGSTAB(A, x, b, M, maxit, tol);

    for ( int i = 0; i < numRows; i++) {
        temp[i] = x[i];
    }
    return 1;
}
*/
