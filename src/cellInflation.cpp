/*
 * cellInflation.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: zhouq
 */

#include "simPlPlace.h"

void SimPlPlace::restore(){
	long origW, origH;
	for (long i = 0; i < numMoveNodes; ++i){
		origW = validNodes[i]->getOrigWidth();
		origH = validNodes[i]->getOrigHeight();
		validNodes[i]->setWidth(origW);
		validNodes[i]->setHeight(origH);
	}
}

void SimPlPlace::bloating(myNet* net, long dem, long cap, bool hori){
	if (cap == 0){
		cap = 1;
	}
	vector<InstTerm> terms = net->getTerms();
	long orig;
	if (hori){
		for (long i = 0; i < terms.size(); ++i){
			Inst* inst = getInst(terms[i]);
			if (inst->getStatus() == Moved && inst->getWidth() <= 5 * inst->getOrigWidth()){
				orig = inst->getWidth();
				inst->setWidth(1.0 * orig * dem / cap);
				assert(inst->getWidth() > 0);
			}
		}
	}
	else{
		for (long i = 0; i < terms.size(); ++i){
			Inst* inst = getInst(terms[i]);
			if (inst->getStatus() == Moved && inst->getHeight() <= 5 * inst->getOrigHeight()){
				orig = inst->getHeight();
				inst->setHeight(1.0 * orig * dem / cap);
				assert(inst->getHeight() > 0);
			}
		}
	}
}


//add by wxy
void SimPlPlace::pinDenBasedInflation(double alpha){
	//pin number,original area, whitespace
	long totalPinNum=getTotalPinNum();
	double totalCellArea=getTotalCellArea();
	vector<double> pin_density;
	vector<double> index;
	pin_density.resize(numMoveNodes);
	double numInstTerms;
	double area;
	for (long i = 0; i < numMoveNodes; i++) {
		//assert(validNodes[i]->getStatus() == Moved);
		numInstTerms=(double) validNodes[i]->getNumInstTerms();
		area=(double) validNodes[i]->getArea();
		pin_density[i]= numInstTerms/area;
	}

	double whitespace=alpha*getAvailableArea();
	assert(whitespace>=0);
	long inflatedNum=0;
	double criticalPinDen=totalPinNum/(whitespace+totalCellArea);
	//select sort of pi/Ai
	int max_index=0;

	for(int i=0;i<numMoveNodes;i++)
	{
		double largest=pin_density[i];
		max_index=i;
		for(int j=i;j<numMoveNodes;j++)
		{
			if(pin_density[j]>largest)
			{
				largest=pin_density[j];
				max_index=j;
			}

		}
		double temp=pin_density[i];
		pin_density[i]=pin_density[max_index];
		pin_density[max_index]=temp;
		index.push_back(max_index);
		inflatedNum++;
		if(largest<=criticalPinDen)
		{
			break;
		}
	}
	//find the largest k
//	for(int k=0;k<numMoveNodes;k++)
//	{
//		double Ik=totalPinNum/pin_density[k]-totalCellArea;
//		if(whitespace<=Ik)
//		{
//			inflatedNum=k;
//			break;
//		}
//	}
	//compute new area
	double dmax=totalPinNum/(whitespace+totalCellArea);
	//update new area of inflated areas
	cout<<"numMoveNodes:"<<numMoveNodes<<endl;
	cout<<"inflatedNum: "<<inflatedNum<<endl;
	for(int i=0;i<inflatedNum;i++)
	{
		int j=index[i];
		double newArea=validNodes[j]->getNumInstTerms()/dmax;
		double areaRatio=newArea/validNodes[j]->getArea();
		long newWidth=(long) validNodes[j]->getWidth()*sqrt(areaRatio);
		//cout<<"validNode["<<j<<"] oldWidth: "<<validNodes[j]->getWidth()<<" "<<validNodes[j]->getName()<<endl;
		//cout<<"validNode["<<j<<"] newWidth: "<<newWidth<<" "<<validNodes[j]->getName()<<endl;
		long newHeight=(long) validNodes[j]->getHeight()*sqrt(areaRatio);
		//cout<<"validNode["<<j<<"] oldHeight: "<<validNodes[j]->getHeight()<<" "<<validNodes[j]->getName()<<endl;
		//cout<<"validNode["<<j<<"] newHeight: "<<newHeight<<" "<<validNodes[j]->getName()<<endl;
		validNodes[j]->setWidth(newWidth);
		validNodes[j]->setHeight(newHeight);

		validNodes[j]->setOWidth(newWidth);
		validNodes[j]->setOHeight(newHeight);
	}

}
//add by wxy
double SimPlPlace::getTotalCellArea(){
	double cellArea = 0;
	for (long i = 0; i < numMoveNodes; i++) {
		assert(validNodes[i]->getStatus() == Moved);
		cellArea += (double) validNodes[i]->getArea();
	}
	return cellArea;
}
//add by wxy
long SimPlPlace::getTotalPinNum(){
	long PinNum=0;
	for (long i = 0; i < numMoveNodes; i++) {
		assert(validNodes[i]->getStatus() == Moved);
		PinNum += validNodes[i]->getNumInstTerms();
	}
	return PinNum;
}
//add by wxy
double SimPlPlace::getAvailableArea(){
	double cellArea = getTotalCellArea();
	double availableArea = 0;
	double obstacleArea = 0;
	double totalArea = (double) blockW * (double) blockH;
	for (long i = numMoveNodes; i < numValidNodes; i++) {
		if (validNodes[i]->getStatus() == Fixed) {
			if ((long)validNodes[i]->getCoordX() >= (long)blockX + blockW ||
					(long)validNodes[i]->getCoordX() + validNodes[i]->getWidth() <= (long)blockX ||
					(long)validNodes[i]->getCoordY() >= blockY + blockH ||
					(long)validNodes[i]->getCoordY() + validNodes[i]->getHeight() <= (long)blockY){
				continue;
			}
			obstacleArea += validNodes[i]->getArea();
		}
	}
	availableArea = totalArea - obstacleArea - cellArea;
	if (availableArea < 0){
		availableArea = 0;
	}
	return availableArea;
}

void SimPlPlace::blockInflation(double ratio){
	vector<Inst*> insts = plcTopBlock->getInsts();
	long origWidth, origHeight;
	for (long i = numMoveNodes; i < insts.size(); ++i){
		if (insts[i]->getStatus() == Fixed){
			origWidth = insts[i]->getOrigWidth();
			origHeight = insts[i]->getHeight();
			insts[i]->setWidth(origWidth * ratio);
			insts[i]->setHeight(origHeight * ratio);
		}
	}
}
