//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <shape.h>
//
// Warp class ShapeW
//
// Author: Wang Sifei
// History: 2012/02/12 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************


#ifndef _SHAPE_H_
#define _SHAPE_H_

using namespace std;

class Shape
{
private:
	char* name;
	long index;
	long numRectangular;
	vector<Inst> shapes;
public:
	inline Shape() {name = new char[128];};
	inline ~Shape() {delete[] name;};
private:
	friend class Block;
};
	

#endif
