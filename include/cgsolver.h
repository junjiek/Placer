#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <fstream>
#include <cstring>
#include "util.h"
#include "hashTable.h"
using namespace std;


#ifndef _CGSOLVER_H
#define _CGSOLVER_H


/*QSparseMat is sparse symmetry positive definite matrix*/
class QSparseMat{
public:
	int n_rows, n_cols;
	int* row_idx; 
	double* values;
	int* col_ptr; 
	int n_values;

	//aux field for func insertEnt
	int cur_col;
	long capacity;

	QSparseMat(){}
	~QSparseMat();

	QSparseMat(int r, int c, long cap);
	void reset(int r, int c, long cap);
	


	QSparseMat(const QSparseMat & mat);

	//note that: the order of insertEnt is  from column 1 to n_cols
	//besides, row>=col
	void insertEnt(int row, int col, double val);


	double getValue(int row, int col, int & pos) const;


	void print() const ;
	void print2() const ;

	//save the matrix to a file
	void save(const char * file);
};


//member function
inline
QSparseMat::~QSparseMat(){
	if(col_ptr) free(col_ptr);
	if(row_idx) free(row_idx);
	if(values) free(values);
}

inline
QSparseMat::QSparseMat(int r, int c, long cap):n_rows(r), n_cols(c), \
	capacity(cap), n_values(0){
	assert(r>=0 && c>=0);
	col_ptr=(int *)malloc(sizeof(int)*(n_cols+1));	
	row_idx=(int *)malloc(sizeof(int)*capacity);
	values=(double*)malloc(sizeof(double)*capacity);
	if(!col_ptr || !row_idx || !values){
		//printError("malloc error", __FILE__, __LINE__);
		exit(1);
	}
	col_ptr[0]=0;
	cur_col=0;
}

inline
void QSparseMat::reset(int r, int c, long cap){
	n_rows=r;
	n_cols=c;
	n_values=0;
	capacity=cap;
	if(col_ptr) free(col_ptr);
	if(row_idx) free(row_idx);
	if(values) free(values);
	col_ptr=(int *)malloc(sizeof(int)*(n_cols+1));	
	row_idx=(int *)malloc(sizeof(int)*capacity);
	values=(double*)malloc(sizeof(double)*capacity);
	if(!col_ptr || !row_idx || !values){
		//printError("malloc error", __FILE__, __LINE__);
		exit(1);
	}
	col_ptr[0]=0;
	cur_col=0;
}
		

inline
QSparseMat::QSparseMat(const QSparseMat & mat){
	n_rows=mat.n_rows;
	n_cols=mat.n_cols;
	n_values=mat.n_values;
	cur_col=mat.cur_col;
	capacity=mat.capacity;
		
	col_ptr=(int *)malloc(sizeof(int)*(n_cols+1));	
	row_idx=(int *)malloc(sizeof(int)*capacity);
	values=(double*)malloc(sizeof(double)*capacity);
	if(!col_ptr || !row_idx || !values){
		//printError("malloc error", __FILE__, __LINE__);
		exit(1);
	}
	memcpy(row_idx, mat.row_idx, sizeof(int)*n_values);
	memcpy(values, mat.values, sizeof(double)*n_values);
	memcpy(col_ptr, mat.col_ptr, sizeof(int)*(n_cols+1));
}


inline
void QSparseMat::print() const {
	for(int col=0; col<n_cols; ++col){
		cout<<"col:"<<col<<"; col_ptr:"<<col_ptr[col]<<endl;
		for(int i=col_ptr[col]; i<col_ptr[col+1]; ++i){
			cout<<"col:"<<col<<endl;
			cout<<"row:"<<row_idx[i]<<endl;
			cout<<"val:"<<values[i]<<endl;
			cout<<endl;
		}
	}
}

inline
void QSparseMat::save(const char * file){
	ofstream out(file);
	for(int col=0; col<n_cols; ++col){
		for(int i=col_ptr[col]; i<col_ptr[col+1]; ++i){
			if(row_idx[i]+1==col+1){
				out<<row_idx[i]+1<<" "<<col+1<<" "<<values[i]/2.0<<endl;
			}
			else{
				out<<row_idx[i]+1<<" "<<col+1<<" "<<values[i]<<endl;
			}
			/*
			if(row_idx[i]!=col){
				out<<col+1<<" "<<row_idx[i]+1<<" "<<values[i]<<endl;
			}
			*/
		}
	}
	out.close();
}

inline
void QSparseMat::print2() const {
	for(int i=0; i<n_rows; ++i){
		for(int j=0; j<n_cols; ++j){
			int pos;
			cout<<getValue(i, j, pos)<<"  	";
		}
		cout<<endl;
	}
}	


//non member function
double norm2(const double * v, int n);

//y=A*x	
void matMultVec(const QSparseMat & A, const double * x, double * y);

void residual(const QSparseMat & A, const double * x, \
	const double * b, double * r);



//solve the linear system M*M'z=r, M is lower triangular sparse matrix
//note that: here M is considered as a lower triangular sparse matrix
//rather than symmetry matrix
void linearSolver(const QSparseMat &M, const double * r, double *z);


//solve the linear system Mz=r,  M is lower triangular sparse matrix, 
//note that: here M is considered as a lower triangular sparse matrix,
//rather than symmetry matrix 
void triangSolver_lower(const QSparseMat & M, const double* r, double* z);


//solve the linear system Mz=r, M is upper triangular sparse matrix
//note that: here M is considered as a upper triangular sparse matrix
//rather than symmetry matrix
void triangSolver_upper(const QSparseMat &M, double *r, double *z);

//a*b
double innerProduct(const double * a, const double * b, int n);

//incomplete Cholesky decomposition
void incompCholesky(QSparseMat & A);


//preconditioned conjugate gradient using incomplete cholesky
void cgsolver_init(const QSparseMat & A);
void cgsolver_destroy(const QSparseMat & A);


void cgsolver(const QSparseMat & A, const QSparseMat & pre, \
	const double* b, double * x, int maxIter, double err);

void cgsolver(const QSparseMat & A, const double *b, double *x, \
	int maxIter, double err);		

#endif
