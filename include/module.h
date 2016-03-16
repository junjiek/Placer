/*
 * module.h
 *
 *  Created on: Feb 17, 2014
 *      Author: zhouq
 */

#ifndef MODULE_H_
#define MODULE_H_

class Module{
private:
	//char name[128];
	char* name;
	long index;  //both valid and invalid nodes
	bool isRectangular;
	double width;
	double height;
	Orient orient;
	NodeType status;
	long numPins;
	vector<InstTerm> pins;

public:
	inline Module(){name = new char[128];pins.clear();};
	inline ~Module() {delete[] name;};

public:
	inline string getName() {
		return string(name);
	};
	inline long getIndex() {
		return index;
	};
	inline void setWidth(double w){
		width = w;
	}
	inline void setStatus(NodeType t){
		status = t;
	}
	// get orient
	inline Orient getOrient() {
		return orient;
	};
	inline bool isRect() {
		return isRectangular;
	};
	inline double getWidth() {
		return width;
	};
	inline double getHeight() {
		return height;
	};
	inline NodeType getStatus() {
		return status;
	};
	inline long getNumInstTerms() {
		return numPins;
	};
	inline vector<InstTerm> &getInstTerms() {
		return pins;
	};
	inline double getArea() {
		return width * height;
	};

private:
	friend class Block;
};
#endif /* MODULE_H_ */
