#include "util.h"

void printError(const char *msg, const char *file, int line){
	cout<<msg<<", in "<<file<<",  line "<<line<<endl;
}

idx_t node_name_2_idx(string node_name){
	idx_t index=0;
	string::iterator iter=node_name.begin();
	for(; iter!=node_name.end(); ++iter){
		if(isdigit(*iter)){
			index=10*index+(*iter)-'0';	
		}
	}
	return index;
}


int primer(int seed){
	for(int i=seed; i<INT_MAX; ++i){
		int sqr=(int)floor(sqrt(i))+1;
		bool divided=false;
		for(int j=2; j<sqr;++j){
			if(i%j==0){
				divided=true;
				break;
			}
		}
		if(!divided) return i;
	}
}


void swapIdx(idx_t & a, idx_t & b){
	idx_t tmp=a;
	a=b;
	b=tmp;
}
	
double max(double a, double b){
	return a>b?a:b;
}

double min(double a, double b){
	return a<b?a:b;
}

int max(int a, int b){
	return a>b?a:b;
}

int min(int a, int b){
	return a<b?a:b;
}

string int2str(int a){
	int i=0;
	char s[100];
	while(true){
		s[i++]=(char)(a%10+'0');
		a=a/10;
		if(a==0) break;
	}
	s[i]='\0';
	return string(s);
}



