//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <row.h>
//
// Warp class RowW
//
// Author: Wang Sifei
// History: 2012/02/21 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************

#ifndef _ROW_H_
#define _ROW_H_

#include"point.h"

using namespace std;

class myRow
{
private:
	char* name;
	long height;
    long xCoord;
    long yCoord;
    long yCoordOrig;
	long siteWidth;
	long siteSpacing;
    Orient siteOrient;
    Orient rowOrient;
    long numSites;
	char symmetry;

public:
	inline myRow() {name = new char[128];};
	inline ~myRow() {delete[] name;};
public:
	static void getOrigin(const myRow *row, myPoint &origin);
public:
  inline string getName() {
    return name;
  }
	inline myPoint getOrigin(){
                myPoint p(xCoord, yCoord);
                return p;
        };
        inline long getCoordX() {
                return xCoord;
        };
        inline void setCoordY(long y){
        	yCoord = y;
        }
        inline long getCoordY() {
                return yCoord;
        };
        inline long getCoordYOrigin(){
        	return yCoordOrig;
        }
        inline void getOrigin(long& x, long& y) {
                x = xCoord;
                y = yCoord;
        };
        inline void setOrigin(myPoint& p) {
                xCoord = p.coordX();
                yCoord = p.coordY();
        };
        inline void setOrigin(long x, long y) {
                xCoord = x;
                yCoord = y;
        };
        // get orient
        inline Orient getRowOrient() {
                return rowOrient;
	}
	inline Orient getSiteOrient() {
		return siteOrient;
	}
	inline long getHeight() {
		return height;
	}
	inline long getWidth() {
		return numSites * siteSpacing;
	}
	inline long getSiteWidth() {
		return siteWidth;
	}
	inline long getSiteSpace() {
		return siteSpacing;
	}
	inline long getNumSites() {
		return numSites;
	}
	inline char getSymmetry() {
		return symmetry;
	}

private:
	friend class Block;
};

class Site
{
public:
	char name[128];
};

#endif
