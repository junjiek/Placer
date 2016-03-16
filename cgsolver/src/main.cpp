#include "cgsolver.h"
#include <iostream>


void testCG(){
	 QSparseMat M(5, 5, 20);
	 M.insertEnt(0, 0, 24);
	 M.insertEnt(2, 0, 6);
   	 M.insertEnt(1, 1, 8);
         M.insertEnt(2, 1, 2);
         M.insertEnt(2, 2, 8);
         M.insertEnt(3, 2, -6);
         M.insertEnt(4, 2, 2);
         M.insertEnt(3, 3, 24);
         M.insertEnt(4, 4, 8);

	double b[]={1, 2, 3, 4, 5};
	double x[5]={0,0,0,0,0};

	cgsolver(M, b, x, 35, 1e-4);
	cout<<"+++solution+++++"<<endl;
	for(int i=0; i<5; ++i){
		cout<<i<<":"<<x[i]<<endl;
	}
}


int main(int argc, char ** argv){
	testCG();
	return 0;
}



