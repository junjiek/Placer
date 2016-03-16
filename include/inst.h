//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <inst.h>
//
// wrap class NodeW
//
// Author: Wang Sifei
// History: 2012/02/20 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************


#ifndef _INST_H_
#define _INST_H_

#include<string>
#include"enum.h"
#include"rect.h"
#include"point.h"
#include"instTerm.h"
#include <vector>

using namespace std;

class Inst
{
private:
	//char name[128];
	char* name;
	long index;  //both valid and invalid nodes
	bool isRectangular;
	long width;
	long height;
	double xCoord;
	double yCoord;
	double origCoordX;
	double origCoordY;
	long origWidth;
	long origHeight;
	Orient orient;
	NodeType status;
	long numPins;
	vector<InstTerm> pins;

	double gcx;
	double gcy;

	bool included;

	//for debug



  //added by QiZhongdong
	int layer; //some NI_terminals are on higher layers than M1
    vector<int> blockingLayers; //some FIXED nodes blocks routing layers

public:
	//inline Inst() : id(0), isValid(true) {};
	inline Inst() : id(0), isValid(false) {name = new char[128]; pins.clear();}; //Quan add
	inline ~Inst() {delete[] name;};
public:
	// get area
    	static inline double getArea(const Inst *inst) {
        	Rect bbox;
		getBBox(inst, bbox);
        	return (bbox.height()) * (bbox.width());
	}
    	// get the bounding box of the inst, with the real cooridinates on chip
    	static void getBBox(const Inst *inst, Rect &bbox);
    	// get lower-left origin
    	static void getOrigin(const Inst *inst, myPoint &origin);
    	// set lower-left origin
    	static void setOrigin(Inst *inst, const myPoint &origin);
    	static inline Orient getOrient(const Inst *inst) {
        	return inst->orient;
   	};
    	// set orientation, whenever this invoked, the inst will transformed 
    	// according to the orientation
    	static inline void setOrient(Inst *inst, const Orient &orient) {
		inst->orient = orient;
	}

public:
	inline string getName() {
		return string(name);
	};
	inline long getId() {
		return id;
	};
	inline void setId(long i) {
		id = i;
	};
	inline bool getIsValid() {
		return isValid;
	};
	inline void setIsValid(bool iv) {
		isValid = iv;
	};
	inline long getIndex() {
		return index;
	};
	inline myPoint getOrigin(){
		myPoint p(xCoord, yCoord);
		return p;
	};
	inline double getCoordX() {
		return xCoord;
	};
	inline double getCoordY() {
		return yCoord;
	};
	inline void getOrigin(double& x, double& y) {
		x = xCoord;
		y = yCoord;
	};
	inline void setOrigin(myPoint& p) {
		xCoord = p.coordX();
		yCoord = p.coordY();
	};
	inline void setOrigin(double x, double y) {
		xCoord = x;
		yCoord = y;
	};
	inline void setCoordX(double x) {
		xCoord = x;
	};
	inline void setCoordY(double y) {
		yCoord = y;
	};
	inline void setWidth(long w){
		width = w;
	}
	inline void setHeight(long h){
		height = h;
	}
	inline void setOWidth(long w){
		origWidth = w;
	}
	inline void setOHeight(long h){
		origHeight = h;
	}
	inline void setGCX(double d){
		gcx = d;
	}
	inline void setGCY(double d){
		gcy = d;
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
	inline long getWidth() {
		return width;
	};
	inline long getHeight() {
		return height;
	};
	inline double getCenterX() {
		return (xCoord + width / 2);
	};
	inline double getCenterY() {
		return (yCoord + height / 2);
	};
	inline myPoint getCenter() {
		return myPoint(getCenterX(), getCenterY());
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
	inline long getOrigWidth(){
		return origWidth;
	}
	inline long getOrigHeight(){
		return origHeight;
	}
	inline double getOrigCoordX(){
		return origCoordX;
	}
	inline double getOrigCoordY(){
		return origCoordY;
	}
	inline double getGCX(){
		return gcx;
	}
	inline double getGCY(){
		return gcy;
	}

	//added by QiZhongdong
	inline void setLayer(int l) {
		layer = l;
	}
	inline int getLayer() {
		return layer;
	}
	inline vector<int>* getBlockingLayers() {
		return &blockingLayers;
	}

	inline void initIncluded(){
		included = false;
	}
	inline void setIncluded(){
		included = true;
	}
	inline bool isIncluded(){
		return included;
	}
private:
	long id; //only valid nodes
	bool isValid;
	friend class Block;

//for debug
public:
	bool moved;
};

#endif
