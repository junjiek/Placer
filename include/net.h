//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <net.h>
//
// wrap class NetW
//
// Author: Wang Sifei
// History: 2012/02/20 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************

#ifndef _NET_H_
#define _NET_H_

#include<string>
#include"instTerm.h"
#include"rect.h"
#include<vector>
#include<map>

using namespace std;

class myNet
{
private:
	char* name;
	long index;
	long numPins;
	vector<InstTerm> pins;
	long weight;
	map<long, long> netShare;
	map<long, long> netInsts;

public:
	inline myNet() {name = new char[128]; weight = 1;};
	inline ~myNet() {delete[] name;};
public:
	static void getBBox(const myNet* net, Rect &bbox);
public:
	inline string getName() {
		return string(name);
    };

	inline long getIndex() {
		return index;
	};
	inline long getNumTerms() {
        return numPins;
    };
    inline vector<InstTerm> &getTerms() {
        return pins;
    };
    inline long getWeight(){
    	return weight;
    }
    inline map<long, long>& getNetShare() {
        return netShare;
    }
    inline map<long, long>& getNetInsts() {
        return netInsts;
    }
    double centerX, centerY;
    long congH,congV;
private:
        friend class Block;
};

#endif
