/*
 * RLRegion.h
 *
 *  Created on: Sep 12, 2013
 *      Author: zhouq
 */

#ifndef RLRegion_H_
#define RLRegion_H_

#include "inst.h"


class RLRegion{

public:
//constructor and destructor
	RLRegion(){};
	RLRegion(long l, long r, long b, long t){
		left = l;
		right = r;
		bottom = b;
		top = t;

		cellArea = 0;
		availableArea = 1.0 * (top - bottom) * (right - left);//the whitespace that be lasted

		clustered = false;
		covered = false;
		moveInstsX.clear();
		moveInstsY.clear();
		fixInsts.clear();
	}
	~RLRegion(){
		moveInstsX.clear();
		moveInstsY.clear();
//		fixInsts.clear();
	};

//getters
	inline long getWidth(){
		return right - left;
	}

	inline long getHeight(){
		return top - bottom;
	}

	inline bool isClustered(){
		return clustered;
	}

	inline double getArea(){
		return 1.0 * (top - bottom) * (right - left);
	}

	inline double getCellArea(){
		return cellArea;
	}

	inline double getAvailableArea(){
		return availableArea;
	}

	inline double getDensity(){
		if (availableArea == 0){
			return 0;
		}
		else{
			return (cellArea / availableArea);
		}
	}

	inline long getGridLeft(){
		return gridLeft;
	}

	inline long getGridRight(){
		return gridRight;
	}

	inline long getGridBottom(){
		return gridBottom;
	}

	inline long getGridTop(){
		return gridTop;
	}

//setters
	inline void setClustered(bool b){
		clustered = b;
	}

	inline void setCellArea(double d){
		cellArea = d;
	}

	inline void setAvailableArea(double d){
		availableArea = d;
	}

	vector<Inst*> moveInstsX;
	vector<Inst*> moveInstsY;
	vector<Inst*> fixInsts;
	double cellArea;
	double availableArea;
	long left;
	long right;
	long bottom;
	long top;
	long gridLeft;
	long gridRight;
	long gridBottom;
	long gridTop;

	// Used for dynamic programming: calculate the space utilization ratio quickly.
	double O; // total area of cells in region (0, 0, i, j)
	double A; // available area of region (0, 0, i, j)
	bool covered;

//functions
	void clearData(){
		cellArea = 0;
		availableArea = 1.0 * (top - bottom) * (right - left);
		clustered = false;
		moveInstsX.clear();
		moveInstsY.clear();
		fixInsts.clear();
	}

private:
	bool clustered;
};

#endif /* RLRegion_H_ */
