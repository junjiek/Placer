//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <instTerm.h>
//
// Warp class PinW
//
// Author: Wang Sifei
// History: 2012/02/20 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************


#ifndef _INST_TERM_H_
#define _INST_TERM_H_

#include<string>
#include "point.h"

class InstTerm
{
private:
	char name[32];
	PinType type;
	double xOffset;
	double yOffset;
	long indexNet;
	long indexNode;
	long layer;

public:
	inline InstTerm() {};
	inline ~InstTerm() {};
public:
	static void getOrigin(const InstTerm *term, myPoint &point); 

public:

    inline string getName() {
        return string(name);
    };
	inline PinType getType() {
		return type;
	};
	inline double getOffsetX() {
		return xOffset;
	};
	inline double getOffsetY() {
		return yOffset;
	};
	inline long getIndexInst() {
		return indexNode;
	};
	inline long getIndexNet() {
		return indexNet;
	};
	inline long getLayer(){
		return layer;
	};
public:
private:
	friend class Block;

};

#endif
