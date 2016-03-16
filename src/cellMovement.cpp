/*
 * cellMovement.cpp
 *
 *  Created on: Mar 26, 2014
 *      Author: thueda
 */
#include "simPlPlace.h"

/*bool lessCongH(myNet* A, myNet* B){
	return (A->congH < B->congH);
}

bool lessCongV(myNet* A, myNet* B){
	return (A->congV < B->congV);
}

void SimPlPlace::setNetCenterAndCong() {
	cout << "Set nets center..." << endl;
	vector<myNet*>& nets = plcTopBlock->getNets();
	vector<Inst*>& insts = plcTopBlock->getInsts();
	for(vector<myNet*>::iterator it = nets.begin(); it != nets.end(); ++it) {
		//set net center
		myNet* net = *it;
		double maxX = 0, maxY = 0, minX = INT_MAX, minY = INT_MAX;
		vector<InstTerm>& terms = net->getTerms();
		for(vector<InstTerm>::iterator iter = terms.begin(); iter != terms.end(); ++iter) {
			InstTerm& term = *iter;
			myPoint termCoord = getOrigin(term);
			maxX = max(maxX, termCoord.coordX());
			maxY = max(maxY, termCoord.coordY());
			minX = min(minX, termCoord.coordX());
			minY = min(minY, termCoord.coordY());
		}
		net->centerX = (minX + maxX) / 2;
		net->centerY = (minY + maxY) / 2;
	}

	//initialization
	NetDB netdb ;
	initInputForRouter(netdb);
	ParamSet par;
	setDefaultParameter(par, false);
	userDefinedParameter(par);
	settingAndChecking(netdb, par, false);
	GlobalRouter gr(netdb.gridX, netdb.gridY, netdb.layer, netdb.horCap, netdb.verCap);

	//Perform Routing
	main_congestion_estimator(netdb, par, gr, NULL) ;



}


void SimPlPlace::netBasedMovementH() {
	cout << "NetBased movement H..." << endl;
	vector<myNet*>& nets = plcTopBlock->getNets();
	sort(nets.begin(), nets.end(), lessCongV);
	long netsSize = long(nets.size());
	long* newCenter = new long[netsSize];
	for(long i = 0; i < netsSize; ++i) {
		myNet* net = nets[i];
		newCenter[i] = net->centerX;
		//if(net->congV > 1) {
		if (net->congV < 0){
			long bestCV = net->congV;
			//find bestCV
			map<long, long>::iterator it = net->getNetShare().begin();
			while(it != net->getNetShare().end()) {
				bestCV = min(nets[it->first]->congV, bestCV);
				++it;
			}
			//if(bestCV != net->congV && bestCV < 1){
			if(bestCV != net->congV && bestCV < 0){
				map<long, long>::iterator iter = net->getNetShare().begin();
				long instSize = net->getNetInsts().size();
				while(iter != net->getNetShare().end()) {
					if(nets[iter->first]->congV < net->congV) {
						double factor1 = (net->congV - nets[iter->first]->congV) / (net->congV - bestCV);
						double factor2 = double(iter->second) / instSize;
						newCenter[i] += long((nets[iter->first]->centerX - net->centerX) * factor1 * factor2);
					}
					++iter;
				}
			}
		}
	}
	vector<Inst*>& insts =  plcTopBlock->getInsts();
	for(vector<Inst*>::iterator it = insts.begin(); it != insts.end(); ++it) {
		Inst* inst = *it;
		if(inst->getStatus() != Moved) {
			continue;
		}
		vector<InstTerm>& terms = inst->getInstTerms();
		vector<long> position;
		long maxX = 0, minX = numeric_limits<long>::max();
		for(vector<InstTerm>::iterator ite = terms.begin(); ite != terms.end(); ++ite) {
			InstTerm& term = *ite;
			long offsetX = inst->getCoordX() - nets[term.getIndexNet()]->centerX;
			long newX = offsetX + newCenter[(*ite).getIndexNet()];
			minX = min(minX, newX);
			maxX = max(maxX, newX);
			position.push_back(newX);
		}
		long midX = (minX + maxX) / 2;
    long chooseX; //= numeric_limits<long>::max();
    if(midX > inst->getCoordX()) {
      chooseX = 0;
      for(vector<long>::iterator iter = position.begin(); iter != position.end(); ++iter) {
        if(midX >= *iter)chooseX = max(chooseX, *iter);
      }
    }else {
      chooseX = numeric_limits<long>::max();
      for(vector<long>::iterator iter = position.begin(); iter != position.end(); ++iter) {
        if(*iter >= midX)chooseX = min(chooseX, *iter);
      }
    }
    chooseX = max(region.left(), chooseX);
    chooseX = min(region.right(),chooseX);

    inst->setCoordX(chooseX);
  }
  delete [] newCenter;
}

void SimPlPlace::netBasedMovementV() {
  cout << "NetBased movement V..." << endl;
  //cout << "Placement region:" << region.top() << " " << region.bottom() << " "
      // << region.left() << " "<< region.right() << endl;
  vector<myNet*>& nets = plcTopBlock->getNets();
  sort(nets.begin(), nets.end(), lessCongH);
  long netsSize = long(nets.size());
  long* newCenter = new long[netsSize];
  for(long i = 0; i < netsSize; ++i) {
    myNet* net = nets[i];
    newCenter[i] = net->centerY;
    if(net->congH > 0) {
      long bestCH = net->congH;
      map<long, long>::iterator it = net->getNetShare().begin();
      while(it != net->getNetShare().end()) {
        bestCH = min(nets[it->first]->congH, bestCH);
        ++it;
      }
      if(bestCH != net->congH && bestCH < 1){
        map<long, long>::iterator iter = net->getNetShare().begin();
        long instSize = net->getNetInsts().size();
        while(iter != net->getNetShare().end()) {
          if(nets[iter->first]->congH < net->congH) {
            double factor1 = (net->congH - nets[iter->first]->congH) / (net->congH - bestCH);
            double factor2 = double(iter->second) / instSize;
            newCenter[i] += long((nets[iter->first]->centerY - net->centerY) * factor1 * factor2);

            if(factor1 > 1)cout << factor1 << " 1 " << i << endl;
            if(factor2 > 1)cout << factor2 << " 2 " << i << endl;
          }
          ++iter;
        }
      }
    }
    //cout << "net:" << i << " " << newCenter[i] << " " << net->centerY << endl;
  }
  vector<Inst*>& insts =  plcTopBlock->getInsts();
  for(vector<Inst*>::iterator it = insts.begin(); it != insts.end(); ++it) {
    Inst* inst = *it;
    if(inst->getStatus() != Moved) {
      continue;
    }
    vector<InstTerm>& terms = inst->getInstTerms();
    vector<long> position;
    long maxY = 0, minY = numeric_limits<long>::max();
    for(vector<InstTerm>::iterator ite = terms.begin(); ite != terms.end(); ++ite) {
      InstTerm& term = *ite;
      long offsetY = inst->getCoordY() - nets[term.getIndexNet()]->centerY;
      long newY = offsetY + newCenter[(*ite).getIndexNet()];
      minY = min(minY, newY);
      maxY = max(maxY, newY);
      position.push_back(newY);
      //cout << offsetY << endl;
    }
    long midY = (minY + maxY) / 2;
    long chooseY; //= numeric_limits<long>::max();
    if(midY > inst->getCoordY()) {
      chooseY = 0;
      for(vector<long>::iterator iter = position.begin(); iter != position.end(); ++iter) {
        if(midY >= *iter)chooseY = max(chooseY, *iter);
      }
    }else {
      chooseY = numeric_limits<long>::max();
      for(vector<long>::iterator iter = position.begin(); iter != position.end(); ++iter) {
        if(*iter >= midY)chooseY = min(chooseY, *iter);
      }
    }

    chooseY = max(region.bottom(), chooseY);
    chooseY = min(region.top(), chooseY);

    inst->setCoordY(chooseY);

  }
  delete [] newCenter;

}*/

