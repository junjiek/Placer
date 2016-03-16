//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <util.h>
//
// Purpose : define utility function
//
// Author: Wang Sifei
// History: 2012/02/20 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************

#ifndef _UTIL_H_WSF_
#define _UTIL_H_WSF_

#include <iostream>
#include <string>
#include <sstream>

inline std::string Itostr(long i) {
	std::string s;
	std::stringstream ss(s);
	ss << i;
	return ss.str();
}

#endif


#include <string>
#include <cstring>
#include <ctime>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <iostream>
#include <fstream>
#include <cctype>
//#include <omp.h>
#include <cassert>
using namespace std;
#ifndef _UTIL_H
#define _UTIL_H

#ifndef _IDX_TYPE
#define _IDX_TYPE
typedef int idx_t;
#endif


typedef enum{LEFT=0, RIGHT=1, BOTTOM=2, TOP=3}  drc_t;

void printError(const char *msg, const char *file, int line);

idx_t node_name_2_idx(string node_name);


#define MAX_FILE_NAME 256
#define MAX_DELIMITERS 10
#define MAX_LINE_LENGTH 2048
#define DEFAULT_DELIMITERS "/#,: \t"

class Reader{
	private:
		string line;
		vector<string> words;
		char delimiters[MAX_DELIMITERS];
		ifstream in;
		bool finished;
	public:
		Reader(const char * f_name, const char *delimiters){
			strcpy(this->delimiters, delimiters);
			in.open(f_name);
			if(!in){
				printError("Reader: input file error", \
						__FILE__, __LINE__);
				exit(1);
			}
			finished=false;
		}


		bool isFinished(){
			return finished;
		}


		void processLine(){
			getline(in, line);
			if(in.eof()){
				finished=true;
				in.close();
				return;
			}
			words.clear();
			char temp[MAX_LINE_LENGTH];
			strcpy(temp, line.c_str());
			char *pch;
			pch = strtok(temp, delimiters);
			while(pch != NULL){
				words.push_back(string(pch));
				pch = strtok(NULL, delimiters);
			}
		}

		vector<string> & getWords(){
			return words;
		}


};




/*
struct Timer{
	double s, e;
	Timer(){
	}

	void start(){
		s=omp_get_wtime();
	}

	void end(){
		e=omp_get_wtime();
	}

	double interval(){
		return e-s;
	}
};
*/



int primer(int seed); //find the least primer which is greater than seed

void swapIdx(idx_t & a, idx_t & b);


double max(double a, double b);

double min(double a, double b);

int max(int a, int b);

int min(int a, int b);

template<class T>
void releaseVecMem(vector<T> & v){
	vector<T> tmp;
	tmp.swap(v);
}

string int2str(int a);


//wrap the realloc function
template<class T>
T   *  reallocMem(T * ptr, int cap){
	ptr=(T*)realloc(ptr, sizeof(T)*cap);
	if(!ptr){
		printError("realloc error", __FILE__, __LINE__);
		exit(1);
	}
	return ptr;
}


//wrap the malloc function
template<class T>
T *  mallocMem(T * ptr, int cap){
	ptr=(T*)malloc(sizeof(T)*cap);
	assert(ptr);
	if(!ptr){
		printError("malloc error", __FILE__, __LINE__);
		exit(1);
	}
	return ptr;
}



#endif
