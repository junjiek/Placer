#include <iostream>
#include <utility>
#include <vector>
#include "util.h"
using namespace std;

#ifndef _HASH_H
#define _HASH_H

template<class KEY_T>
class HashFunc{
	public:
		unsigned  long operator()(KEY_T & key){
			return key;
		}
};


template<> class HashFunc<string>{
	public:
		unsigned  long operator()(string & key){
			unsigned long hash=0;
			const char *str=key.c_str();
			char c;
			while(c=*str++)
				hash=((hash << 5) + hash) +c;
			return hash;
		}
};

template<> class HashFunc<pair<int,int> >{
	public:
		unsigned long operator()(pair<int, int> & key){
			return (key.first * 0x1f1f1f1f) ^ key.second;
		}
};


class IspdHashFunc{
	public:
		unsigned long operator()(string & key){
			return node_name_2_idx(key);
		}

};



template<typename KEY_T, typename VAL_T, typename HASH_FUNC>
struct HashTable{
	enum {MIN_CAP_PER_BUCK=5};
	typedef struct{
		KEY_T key;
		VAL_T value;
	}entry_t;
	int n_bucks;
	entry_t ** bucks;
	int *buck_tails;  //the tail position of each bucket 
	int *buck_caps; // the current capacity of each bucket
	HASH_FUNC hash_func;



	HashTable(int n){
		n_bucks=n;
		bucks=(entry_t**)malloc(sizeof(entry_t*)*n_bucks);
		buck_tails=(int*)malloc(sizeof(int)*n_bucks);
		buck_caps=(int *)malloc(sizeof(int)*n_bucks);
		if(!bucks || !buck_tails || !buck_caps){
			printError("malloc error", __FILE__, __LINE__);
			exit(1);
		}
		for(int i=0; i<n_bucks; ++i){
			bucks[i]=new entry_t[MIN_CAP_PER_BUCK];
			buck_caps[i]=MIN_CAP_PER_BUCK;
			buck_tails[i]=0;
			if(!bucks[i]){
				printError("malloc error", __FILE__, \
						__LINE__);
				exit(1);
			}
		}
	}

	~HashTable(){
		if(buck_tails) free(buck_tails);
		if(buck_caps) free(buck_caps);
		if(bucks){
			for(int i=0; i<n_bucks; ++i){
				delete [] bucks[i];
			}
			free(bucks);
		}
	}

	void insert(KEY_T key, VAL_T val){
		int bucket=hash_func(key) % n_bucks;		
		if(buck_tails[bucket]>=buck_caps[bucket]){
			entry_t* new_buck=new entry_t [2*buck_caps[bucket]];
			if(!new_buck){
				printError("expand memeory error", __FILE__, \
					__LINE__);
				exit(1);
			}
			for(int i=0; i<buck_caps[bucket]; ++i){
				new_buck[i].key=KEY_T(bucks[bucket][i].key);
				new_buck[i].value=VAL_T(bucks[bucket][i].value);
			}
			delete [] bucks[bucket];
			bucks[bucket]=new_buck;
			buck_caps[bucket]*=2;
		}
		bucks[bucket][buck_tails[bucket]].key=KEY_T(key);
		bucks[bucket][buck_tails[bucket]].value=VAL_T(val);
		buck_tails[bucket]++;
	}

	bool getValue(KEY_T key, VAL_T  & val){
		int bucket=hash_func(key) % n_bucks;
		for(size_t i=0; i<buck_tails[bucket]; ++i){
			if(bucks[bucket][i].key==key){
				val=bucks[bucket][i].value;	
				return true;
			}
		}
		return false;
	}

	void print(){
		for(size_t i=0; i<n_bucks; ++i){
			for(size_t j=0; j<buck_tails[i]; ++j){
				cout<<bucks[i][j].key<<"	"<<\
					bucks[i][j].value<<endl;
			}
		}
	}
	
};

#endif
