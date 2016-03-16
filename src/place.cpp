//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of
// Tsinghua University and may not be reproduced in any form by any
// other persons or organizations without prior written permissions
// of EDA Lab. of Tsinghua University.
//
// <place.cpp>
//
// myPlacement main entry
//
// Author: Wang Sifei
// History: 2012/02/15 created by Sifei
//*****************************************************************************
//*****************************************************************************
#include "place.h"

unsigned long
myPlacement::getRowHeight(Block *block)
{
    return 0;
}

// check if the cell is valid cell instance
bool
myPlacement::isValidCell(Inst *inst)
{
    return true;
}

// is valid for global placement, such as isolated cells
bool
myPlacement::isConnected(Inst *inst)
{
    return true;
}

// *****************************************************************************
// myPlacement methods
// *****************************************************************************

myPlacement::myPlacement(Block *topBlock)
        : plcTopBlock(topBlock)
{
	setRegion();
}

void myPlacement::setRegion() {
	vector<myRow*>& rowss = plcTopBlock->getRows();
	for(unsigned long i=0; i<plcTopBlock->getNumRows(); ++i) {
		myRow* row = rowss[i];
		long x = row->getCoordX();
		long y = row->getCoordY();
		long w = row->getWidth();
		long h = row->getHeight();
		//cout<<x<<"  "<<y<<"  "<<w<<"  "<<h<<endl;
		if (i == 0) {
			region.setCoord(x, y, x+w, y+h);
		} else {
			Rect rect(x, y, x+w, y+h);
			region.stretch(rect);
		}
	}
	return;
}

unsigned long
myPlacement::getInstsArea() {
	unsigned long area = 0;
	vector<Inst*>& insts = plcTopBlock->getInsts();
	for(unsigned long i = 0; i < insts.size(); ++i) {
		if (insts[i]->isRect()) {
			area += insts[i]->getArea();
		}
	}
	return area;
}

unsigned long
myPlacement::getHPWL(const vector<myNet*> &netlist)
{
 	unsigned long total = 0;
	for (unsigned long netId = 0 ; netId < netlist.size(); netId++) {
                unsigned long pinsNum = netlist[netId] -> getNumTerms();
                vector<InstTerm>& instTerm = netlist[netId] -> getTerms();
                //assert(instTerm.size() > 1);
                if (instTerm.size() < 2) {
                  continue;
                }
                myPoint origin1 = getOrigin(instTerm[0]);
                long minX = origin1.coordX();
                long minY = origin1.coordY();
                long maxX = minX;
                long maxY = minY;

                for (unsigned long tId = 1 ; tId < pinsNum ; tId++) {
                        myPoint origin = getOrigin(instTerm[tId]);
                        if (origin.coordX() > maxX) {
                                maxX = origin.coordX();
                        }
                        if (origin.coordX() <= minX) {
                                minX = origin.coordX();
                        }
                        if (origin.coordY() > maxY) {
                                maxY = origin.coordY();
                        }
                        if (origin.coordY() <= minY) {
                                minY = origin.coordY();
                        }
                }
                total += (maxX + maxY - minX -minY);
        }

    	return total;
}

unsigned long myPlacement::getHPWL(myNet* net) {
	unsigned long total = 0.0;
	unsigned long pinsNum = net->getNumTerms();
	vector<InstTerm>& instTerm = net->getTerms();
	if (instTerm.size() < 2) {
		return 0;
	}

	myPoint origin1 = getOrigin(instTerm[0]);
	long minX = origin1.coordX();
	long minY = origin1.coordY();
	long maxX = minX;
	long maxY = minY;

	for (unsigned long tId = 1; tId < pinsNum; tId++) {
		myPoint origin = getOrigin(instTerm[tId]);
		if (origin.coordX() > maxX) {
			maxX = origin.coordX();
		}
		if (origin.coordX() <= minX) {
			minX = origin.coordX();
		}
		if (origin.coordY() > maxY) {
			maxY = origin.coordY();
		}
		if (origin.coordY() <= minY) {
			minY = origin.coordY();
		}
	}
	total = (maxX + maxY - minX - minY);

	return total;
}


double
myPlacement::getHPWL(const Block *block)
{
    double total = 0;

    //for debug
    double totalX = 0;
    double totalY = 0;

	vector<myNet*>& validNets = plcTopBlock->getNets();
	for (unsigned long netId = 0 ; netId < validNets.size(); netId++) {
		unsigned long pinsNum = validNets[netId] -> getNumTerms();
		vector<InstTerm>& instTerm = validNets[netId] -> getTerms();
		//assert(instTerm.size() > 1);
		if (instTerm.size() < 2) {
			continue;
		}
		myPoint origin1 = getOrigin(instTerm[0]);
		long minX = origin1.coordX();
		long minY = origin1.coordY();
		long maxX = minX;
		long maxY = minY;

		for (unsigned long tId = 1 ; tId < pinsNum ; tId++) {
			myPoint origin = getOrigin(instTerm[tId]);
			if (origin.coordX() > maxX) {
				maxX = origin.coordX();
			} else if (origin.coordX() < minX) {
				minX = origin.coordX();
			}
			if (origin.coordY() > maxY) {
				maxY = origin.coordY();
			} else if (origin.coordY() < minY) {
				minY = origin.coordY();
			}
		}
		total += (maxX + maxY - minX -minY);
		totalX += (maxX - minX);
		totalY += (maxY - minY);
	}
	//cout<<totalX<<" "<<totalY<<" ";
	return total / plcTopBlock->getBloatSize();
}

void
myPlacement::getHPWL(const Block *block, unsigned long &total) {
	return ;
}

Inst* myPlacement::getInst(InstTerm& it) {
	unsigned long index = it.getIndexInst();
	return (plcTopBlock->getInsts())[index];
}

myNet* myPlacement::getNet(InstTerm& it) {
	unsigned long index = it.getIndexNet();
	return (plcTopBlock->getNets())[index];
}
Inst* myPlacement::getInst(InstTerm *it) {
	unsigned long index = it->getIndexInst();
	return (plcTopBlock->getInsts())[index];
}

myNet* myPlacement::getNet(InstTerm *it) {
	unsigned long index = it->getIndexNet();
	return (plcTopBlock->getNets())[index];
}

myPoint myPlacement::getOrigin(InstTerm& it) {
	Inst* inst = getInst(it);
	long cx = inst->getCenterX();
	long cy = inst->getCenterY();
	long ox = it.getOffsetX();
	long oy = it.getOffsetY();

	switch(inst->getOrient()){
	case kR0:
		return myPoint(cx + ox, cy + oy);
	case kR90:
		return myPoint(cx + oy, cy - ox);
	case kR180:
		return myPoint(cx - ox, cy - oy);
	case kR270:
		return myPoint(cx - oy, cy + ox);
	case kMY:
		return myPoint(cx - ox, cy + oy);
	case kMYR90:
		return myPoint(cx + oy, cy + ox);
	case kMX:
		return myPoint(cx + ox, cy - oy);
	case kMXR90:
		return myPoint(cx - oy, cy - ox);
	default:
		cout<<"unknown orientation!!!"<<endl;
		return myPoint(0,0);
	}

	return myPoint(cx, cy);
}
void myPlacement::getInstsOfNet(myNet* net, vector<Inst*>& insts){
	vector<InstTerm> terms = net->getTerms();
	insts.clear();
	insts.push_back(getInst(terms[0]));
	long index1, index2;
	for (long i = 1; i < terms.size(); ++i){
		index1 = terms[i-1].getIndexInst();
		index2 = terms[i].getIndexInst();
		if (index1 != index2){
			insts.push_back(getInst(terms[i]));
		}
	}
	long numPins = net->getNumTerms();
	return;
}

myPoint myPlacement::getOrigin(InstTerm* it) {
	Inst* inst = getInst(it);
	long cx = inst->getCenterX();
	long cy = inst->getCenterY();
	long ox = it->getOffsetX();
	long oy = it->getOffsetY();
	return myPoint(cx + ox, cy + oy);
}

