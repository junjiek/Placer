/*
 * test.h
 *
 *  Created on: Nov 22, 2015
 *      Author: wuwy
 */

#ifndef TEST_H_
#define TEST_H_

#include <vector>

class CnNodes{
private:
	long index;
	long xGridCoord;
	long yGridCoord;
	long xAnchor;
	long yAnchor;
	double xDeta;
	double yDeta;
	vector<long> indexConFixed;

public:
	inline CnNodes() {
		indexConFixed.clear();
	};

	inline void setIndex(long in) {
		index = in;
	};
	inline void setXGrid(long xG) {
		xGridCoord = xG;
	};
	inline void setYGrid(long yG) {
		yGridCoord = yG;
	};
	inline void setXAnchor(long x) {
		xAnchor = x;
	};
	inline void setYAnchor(long y) {
		yAnchor = y;
	};
	inline void setXDeta(double x) {
		 xDeta = x;
	};
	inline void setYDeta(double y) {
		 yDeta = y;
	};
	inline long getIndex() {
		return index;
	};
	inline long getxGridCoord() {
		return xGridCoord;
	};
	inline long getyGridCoord() {
		return yGridCoord;
	};
	inline long getxAnchor() {
		return xAnchor;
	};
	inline long getyAnchor() {
		return yAnchor;
	};
	inline long getXDeta() {
		 return xDeta;
	};
	inline long getYDeta() {
		 return yDeta;
	};
	inline vector<long>& getConFixed() {
		return indexConFixed;
	}
	inline void addConFixed(long x) {
		indexConFixed.push_back(x);
	}
	inline void clearConFixed() {
		indexConFixed.clear();
	}
};

#endif /* TEST_H_ */
