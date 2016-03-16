#include "cgsolver.h"

/*The algorithm comes from <<matrix computation>> version 3*/
#define OPENMP_ON

double* aux;
double* copy_r;
double *r, *z, *p, *q;

// for the upper triangular part
int * n_e_of_cols; // the number of element in each col 
pair<int, double> * upper_tria;
int * begin_of_cols; // the begin pos of each col

int* upper_tria_cur_pos; //current position of each col


double *temp_y;

void cgsolver_init(const QSparseMat & A){
	int tot_elem=0;
	int n_cols=A.n_cols;
	aux=(double *)malloc(sizeof(double)*n_cols);
	copy_r=(double *)malloc(sizeof(double)*n_cols);
	r=(double *)malloc(sizeof(double)*n_cols);
	z=(double *)malloc(sizeof(double)*n_cols);
	p=(double *)malloc(sizeof(double)*n_cols);
	q=(double *)malloc(sizeof(double)*n_cols);

	temp_y=(double *)malloc(sizeof(double)*n_cols);
	
	if(!aux || !copy_r || !r || !z || !p || !q || !temp_y){
		printError("malloc error", __FILE__, __LINE__);
		exit(1);
	}
	
	//for incomplete cholesky decomposition	
	n_e_of_cols=(int*)malloc(sizeof(int)*n_cols);
	begin_of_cols=(int*)malloc(sizeof(int)*n_cols);
	upper_tria_cur_pos=(int*)malloc(sizeof(int)*n_cols);
	if(!n_e_of_cols || ! upper_tria_cur_pos || !begin_of_cols){
		printError("malloc error", __FILE__, __LINE__);
		exit(1);
	}
	memset(upper_tria_cur_pos, 0, sizeof(int)*n_cols);
	memset(n_e_of_cols, 0, sizeof(int)*n_cols);
	for(int i=0; i<n_cols; ++i){
		int col_start=A.col_ptr[i];
		int col_end=A.col_ptr[i+1];
		for(int j=col_start; j<col_end; ++j){
			assert(A.row_idx[j]<n_cols);
			n_e_of_cols[A.row_idx[j]]++;
			tot_elem++;
		}
	}
	//generate the begin_of_cols;
	begin_of_cols[0]=0;
	for(int i=1; i<n_cols; ++i){
		begin_of_cols[i]=begin_of_cols[i-1]+n_e_of_cols[i-1];
	}
	
	upper_tria=(pair<int, double>*)malloc(sizeof(pair<int, double>)\
			*tot_elem);

	if(!upper_tria){
		printError("malloc error", __FILE__, __LINE__);
		exit(1);
	}
}

void cgsolver_destroy(const QSparseMat & A){
	int n_cols=A.n_cols;
	if(aux) free(aux);
	if(copy_r) free(copy_r);
	if(r) free(r);
	if(z) free(z);
	if(p) free(p);
	if(q) free(q);
	if(n_e_of_cols) free(n_e_of_cols);
	if(upper_tria_cur_pos) free(upper_tria_cur_pos);
	if(upper_tria) free(upper_tria);
	if(begin_of_cols) free(begin_of_cols);
	if(temp_y) free(temp_y);
}



void QSparseMat::insertEnt(int row, int col, double val){
//	cout<<"insert: "<<row<<" "<<col<<" "<<val<<", [0] is "<<row_idx[0]<<endl;
//	long x;
//	cin >> x;

	assert(row>=col);
	if(col!=cur_col){//	
//		cout<<"[0_-5] : "<<row_idx[0]<<endl;
		assert(col=cur_col+1);
//		cout<<"[0_-4] : "<<row_idx[0]<<endl;
		col_ptr[++cur_col]=n_values;
	}
//	cout<<"[0_-3] : "<<row_idx[0]<<endl;

	if(n_values>=capacity){
		capacity*=1.2;
//		cout<<"[0_-2] : "<<row_idx[0]<<endl;
		row_idx=(int*)realloc(row_idx, sizeof(int*)*capacity);
//		cout<<"[0_-1] : "<<row_idx[0]<<endl;
		values=(double *)realloc(values, sizeof(double*)*capacity);
	}
//	cout<<"[0_00] : "<<row_idx[0]<<endl;
	assert(n_values<capacity);
	row_idx[n_values]=row;
//	cout<<"[0_0] : "<<row_idx[0]<<endl;
	//cout<<"row_idx "<<n_values<<" is set to "<<row<<endl;
//	cout<<"[0_1] : "<<row_idx[0]<<endl;
//	if (row > 210904){
//		cout<<"!!!! 210904 "<<row<<endl;
//	}
	values[n_values]=val;
//	cout<<"[0_2] : "<<row_idx[0]<<endl;
	n_values++;
//	cout<<"[0_3] : "<<row_idx[0]<<endl;
	col_ptr[n_cols]=n_values;
//	cout<<"[0_4] : "<<row_idx[0]<<endl;
}


double QSparseMat::getValue(int row, int col, int &pos) const{
	if(row<col) return getValue(col, row, pos);
	int low=col_ptr[col];
	int high=col_ptr[col+1]-1;
	while(low<=high){
		int mid=(low+high)/2;
		if(row_idx[mid]>row){
			high=mid-1;
		}
		else if(row_idx[mid]<row){
			low=mid+1;
		}
		else{
			pos=mid;
			return values[mid];
		}	
	}
	pos=-1;
	return 0.0;
}


//y=0 before using this function
void matMultVec(const QSparseMat & A, const double * x,  double * y){
	int n=A.n_cols;
	memset(y, 0, sizeof(double)*n);
	for(int col=0; col<n; ++col){
		for(int i=A.col_ptr[col]; i<A.col_ptr[col+1]; ++i){
			y[A.row_idx[i]]+=x[col]*A.values[i];		
			if(A.row_idx[i]!=col)
				y[col]+=x[A.row_idx[i]]*A.values[i];
		}
	}

}

void residual(const QSparseMat & A, const double *x, \
	const double * b, double * r){
	int n=A.n_cols;
	matMultVec(A, x, aux);  //aux=A*x
#ifdef OPENMP_ON
	#pragma omp parallel for 
#endif
	for(int col=0; col<n; ++col){
		r[col]=b[col]-aux[col];
	}
}


double innerProduct(const double *a, const double *b, int n){
	double sum=0;
#ifdef OPENMP_ON
	#pragma omp parallel for reduction(+: sum)
#endif
	for(int i=0; i<n; ++i){
		sum+=a[i]*b[i];
	}
	return sum;
}

double norm2(const double * v, int n){
	return sqrt(innerProduct(v,v,n));
}

void incompCholesky(QSparseMat & A){
	int n_cols=A.n_cols;
	memset(upper_tria_cur_pos, 0, sizeof(int)*n_cols);	
	for(int j=0; j<n_cols; ++j){
		int col_j_start=A.col_ptr[j];
		double sum=A.values[col_j_start];
		for(size_t z=0; z<upper_tria_cur_pos[j]; ++z){
			sum-=pow(upper_tria[begin_of_cols[j]+z].second,2);
		}
		A.values[col_j_start]=sqrt(sum);
		double div=1.0/A.values[col_j_start];
		int pos=begin_of_cols[j]+upper_tria_cur_pos[j];
		upper_tria[pos].first=j;	
		upper_tria[pos].second=A.values[col_j_start];
		upper_tria_cur_pos[j]++;
		for(int e=col_j_start+1; e<A.col_ptr[j+1]; ++e){
			int i=A.row_idx[e];
			size_t z1_idx=0, z2_idx=0;
			double sum=A.values[e];
			while(z1_idx<upper_tria_cur_pos[i] &&\
				 z2_idx<upper_tria_cur_pos[j]){
				int pos1=begin_of_cols[i]+z1_idx;
				int pos2=begin_of_cols[j]+z2_idx;

				int k1=upper_tria[pos1].first;
				int k2=upper_tria[pos2].first;
				if(k1 == k2){
					sum-=upper_tria[pos1].second *\
						 upper_tria[pos2].second;
					z1_idx++;
					z2_idx++;
				}
				else if(k1 < k2){
					z1_idx++;
				}
				else{
					z2_idx++;
				}
			}
			A.values[e]=sum*div;
			pos=begin_of_cols[i]+upper_tria_cur_pos[i];
			upper_tria[pos].first=j;
			upper_tria[pos].second=A.values[e];
			upper_tria_cur_pos[i]++;

		}
	}
}


void linearSolver(const QSparseMat & M, const double * r, double *z){
	triangSolver_lower(M, r, temp_y);
	triangSolver_upper(M, temp_y, z);

}

void triangSolver_lower(const QSparseMat & M,  const double * r, double* z){
	int n=M.n_cols;
	memcpy(copy_r, r, sizeof(double)*n);
	for(int col=0; col<n; ++col){
		z[col]=copy_r[col]/M.values[M.col_ptr[col]];
		for(int i=M.col_ptr[col]+1; i<M.col_ptr[col+1]; ++i){
			copy_r[M.row_idx[i]]-=z[col]*M.values[i];	
		}
	}
}




void triangSolver_upper(const QSparseMat &M, double *r, double *z){
	int n=M.n_cols;
	int i;
	int pos;
	for(int col=n-1; col>=0; --col){
		pos=begin_of_cols[col]+upper_tria_cur_pos[col]-1;	
		z[col]=r[col]/upper_tria[pos].second;
		for(int i=0; i<upper_tria_cur_pos[col]; ++i){
			int cur_row=upper_tria[begin_of_cols[col]+i].first;
			double cur_val=upper_tria[begin_of_cols[col]+i].second;
			r[cur_row]-=z[col]*cur_val;
		}
	}
}

void cgsolver(const QSparseMat & A, const QSparseMat & pre, const double * b, \
		double * x, int maxIter, double err){
	int n=A.n_cols;	
	double alpha,beta;
	double c_old,c_new;
	double norm_b, norm_r;
	norm_b=norm2(b, n);
	int k=0;
	residual(A, x, b, r);  //r=b-Ax
	linearSolver(pre, r, z); //z=pre(-1)*r
	c_old=innerProduct(r, z, n);//c=rz
	//p=z
	memcpy(p, z, sizeof(double)*n);
	double time_matMultVec=0.0;
	double time_linearSolver=0.0;
	double time_copy=0.0;
	double time_inner=0.0;
	do{
		matMultVec(A, p, q); //q=Ap
		alpha=c_old/innerProduct(p,q, n); //alpha=c/pq

		for(int i=0; i<n; ++i){
			x[i]+=alpha*p[i];
			r[i]-=alpha*q[i];
		}
		//the stop condition
		norm_r=norm2(r, n);
		if(k>maxIter || norm_r<err*norm_b) break;
		linearSolver(pre, r, z);

		c_new=innerProduct(r,z, n); //c=rz
		beta=c_new/c_old;
		c_old=c_new;
		for(int i=0; i<n; ++i){
			p[i]=z[i]+beta*p[i];
		}
		k++;
	}while(true);
}



void cgsolver(const QSparseMat &A, const double *b, double *x, \
		int maxIter, double err){
	//the data structures
	double* aux;
	double* copy_r;
	double *r, *z, *p, *q;

	// for the upper triangular part
	int * n_e_of_cols; // the number of element in each col 
	pair<int, double> * upper_tria;
	int * begin_of_cols; // the begin pos of each col

	int* upper_tria_cur_pos; //current position of each col
	double *temp_y;

	//init the memory: just copy the function of cgsolver_init
	int tot_elem=0;
	int n_cols=A.n_cols;
	aux=(double *)malloc(sizeof(double)*n_cols);
	copy_r=(double *)malloc(sizeof(double)*n_cols);
	r=(double *)malloc(sizeof(double)*n_cols);
	z=(double *)malloc(sizeof(double)*n_cols);
	p=(double *)malloc(sizeof(double)*n_cols);
	q=(double *)malloc(sizeof(double)*n_cols);

	temp_y=(double *)malloc(sizeof(double)*n_cols);
	
	if(!aux || !copy_r || !r || !z || !p || !q || !temp_y){
		printError("malloc error", __FILE__, __LINE__);
		exit(1);
	}
	
	//for incomplete cholesky decomposition	
	n_e_of_cols=(int*)malloc(sizeof(int)*n_cols);
	begin_of_cols=(int*)malloc(sizeof(int)*n_cols);
	upper_tria_cur_pos=(int*)malloc(sizeof(int)*n_cols);
	if(!n_e_of_cols || ! upper_tria_cur_pos || !begin_of_cols){
		printError("malloc error", __FILE__, __LINE__);
		exit(1);
	}
	memset(upper_tria_cur_pos, 0, sizeof(int)*n_cols);
	memset(n_e_of_cols, 0, sizeof(int)*n_cols);
	for(int i=0; i<n_cols; ++i){
		int col_start=A.col_ptr[i];
		int col_end=A.col_ptr[i+1];
		for(int j=col_start; j<col_end; ++j){
			if (A.row_idx[j] >= n_cols){
				cout<<j<<" "<<A.row_idx[j]<<" "<<n_cols<<endl;
			}
			assert(A.row_idx[j]<n_cols);
			n_e_of_cols[A.row_idx[j]]++;
			tot_elem++;
		}
	}
	//generate the begin_of_cols;
	begin_of_cols[0]=0;
	for(int i=1; i<n_cols; ++i){
		begin_of_cols[i]=begin_of_cols[i-1]+n_e_of_cols[i-1];
	}
	
	upper_tria=(pair<int, double>*)malloc(sizeof(pair<int, double>)\
			*tot_elem);

	if(!upper_tria){
		printError("malloc error", __FILE__, __LINE__);
		exit(1);
	}


	//incompCholesky 
	QSparseMat pre(A);
	memset(upper_tria_cur_pos, 0, sizeof(int)*n_cols);	
	for(int j=0; j<n_cols; ++j){
		int col_j_start=pre.col_ptr[j];
		double sum=pre.values[col_j_start];
		for(size_t z=0; z<upper_tria_cur_pos[j]; ++z){
			sum-=pow(upper_tria[begin_of_cols[j]+z].second,2);
		}
		pre.values[col_j_start]=sqrt(sum);
		double div=1.0/pre.values[col_j_start];
		int pos=begin_of_cols[j]+upper_tria_cur_pos[j];
		upper_tria[pos].first=j;	
		upper_tria[pos].second=pre.values[col_j_start];
		upper_tria_cur_pos[j]++;
		for(int e=col_j_start+1; e<pre.col_ptr[j+1]; ++e){
			int i=pre.row_idx[e];
			size_t z1_idx=0, z2_idx=0;
			double sum=pre.values[e];
			while(z1_idx<upper_tria_cur_pos[i] &&\
				 z2_idx<upper_tria_cur_pos[j]){
				int pos1=begin_of_cols[i]+z1_idx;
				int pos2=begin_of_cols[j]+z2_idx;

				int k1=upper_tria[pos1].first;
				int k2=upper_tria[pos2].first;
				if(k1 == k2){
					sum-=upper_tria[pos1].second *\
						 upper_tria[pos2].second;
					z1_idx++;
					z2_idx++;
				}
				else if(k1 < k2){
					z1_idx++;
				}
				else{
					z2_idx++;
				}
			}
			pre.values[e]=sum*div;
			pos=begin_of_cols[i]+upper_tria_cur_pos[i];
			upper_tria[pos].first=j;
			upper_tria[pos].second=pre.values[e];
			upper_tria_cur_pos[i]++;

		}
	}



	//run cg iteration
	int n=A.n_cols;	
	double alpha,beta;
	double c_old,c_new;
	double norm_b, norm_r;
	norm_b=norm2(b, n);
	int k=0;



	matMultVec(A, x, aux);  //aux=A*x, this func is thread-safe
	for(int col=0; col<n; ++col){
		r[col]=b[col]-aux[col];
	}

	memcpy(copy_r, r, sizeof(double)*n);
	for(int col=0; col<n; ++col){
		temp_y[col]=copy_r[col]/pre.values[pre.col_ptr[col]];
		for(int i=pre.col_ptr[col]+1; i<pre.col_ptr[col+1]; ++i){
			copy_r[pre.row_idx[i]]-=temp_y[col]*pre.values[i];	
		}
	}
	for(int col=n-1; col>=0; --col){
		int pos=begin_of_cols[col]+upper_tria_cur_pos[col]-1;	
		z[col]=temp_y[col]/upper_tria[pos].second;
		for(int i=0; i<upper_tria_cur_pos[col]; ++i){
			int cur_row=upper_tria[begin_of_cols[col]+i].first;
			double cur_val=upper_tria[begin_of_cols[col]+i].second;
			temp_y[cur_row]-=z[col]*cur_val;
		}
	}


	c_old=innerProduct(r, z, n);//c=rz
	//p=z
	memcpy(p, z, sizeof(double)*n);
	double time_matMultVec=0.0;
	double time_linearSolver=0.0;
	double time_copy=0.0;
	double time_inner=0.0;
	do{
		matMultVec(A, p, q); //q=Ap

		alpha=c_old/innerProduct(p,q, n); //alpha=c/pq
#ifdef OPENMP_ON
		#pragma omp parallel for
#endif
		for(int i=0; i<n; ++i){
			x[i]+=alpha*p[i];
			r[i]-=alpha*q[i];
		}
		//the stop condition
		norm_r=norm2(r, n);
		if(k>maxIter){
			cout<<"max iter break, error = "<<norm_r<<" "<<norm_b<<" "<<norm_r / norm_b<<endl;
			break;
		}
		else if (norm_r<err*norm_b){
			cout<<"norm error break, error = "<<norm_r<<" "<<norm_b<<" "<<norm_r / norm_b<<endl;
			break;
		}


		//linear solver
		memcpy(copy_r, r, sizeof(double)*n);
		for(int col=0; col<n; ++col){
			temp_y[col]=copy_r[col]/pre.values[pre.col_ptr[col]];
			for(int i=pre.col_ptr[col]+1; i<pre.col_ptr[col+1]; ++i){
				copy_r[pre.row_idx[i]]-=temp_y[col]*pre.values[i];	
			}
		}
		for(int col=n-1; col>=0; --col){
			int pos=begin_of_cols[col]+upper_tria_cur_pos[col]-1;	
			z[col]=temp_y[col]/upper_tria[pos].second;
			for(int i=0; i<upper_tria_cur_pos[col]; ++i){
				int cur_row=upper_tria[begin_of_cols[col]+i].first;
				double cur_val=upper_tria[begin_of_cols[col]+i].second;
				temp_y[cur_row]-=z[col]*cur_val;
			}
		}



		c_new=innerProduct(r,z, n); //c=rz
		beta=c_new/c_old;
		c_old=c_new;
#ifdef OPENMP_ON
		#pragma omp parallel for
#endif
		for(int i=0; i<n; ++i){
			p[i]=z[i]+beta*p[i];
		}
		k++;
	}while(true);

	//free the memory
	if(aux) free(aux);
	if(copy_r) free(copy_r);
	if(r) free(r);
	if(z) free(z);
	if(p) free(p);
	if(q) free(q);
	if(n_e_of_cols) free(n_e_of_cols);
	if(upper_tria_cur_pos) free(upper_tria_cur_pos);
	if(upper_tria) free(upper_tria);
	if(begin_of_cols) free(begin_of_cols);
	if(temp_y) free(temp_y);
}


