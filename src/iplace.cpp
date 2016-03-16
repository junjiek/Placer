#include <vector>
#include <cmath>
#include"simPlPlace.h"
#include"enum.h"

using namespace std;

void SimPlPlace::iPlace(int strategy) {
	time_t begin = clock();
	//cout<<"now filter nodes and nets..."<<endl;
	filterInstsAndNets();

	if (strategy == 1) {
		randomDisperse();
	} else if (strategy == 2) {
		purposeDisperse();
	} else {
		;
	}

//	cout<<validNodes[30180]->getName()<<endl;
//	cout<<validNodes[30183]->getName()<<endl;
//	cout<<validNodes[30186]->getName()<<endl;
//	cout<<validNodes[30190]->getName()<<endl;
//	cout<<"pause"<<endl;
//	long x;
//	cin >> x;

	int type = 0; //0-amgpcg or 3-iccg
	long maxSteps = 10;
	double hpwl = getHPWL(plcTopBlock);
	cout << "[INFO] : step = 0 , random hpwl = " << hpwl << endl;
	for (long i = 0; i < maxSteps; ++i) {
		time_t start_ = clock();
		//cout<<"now build B2B matrix..."<<endl;
		buildB2B();

		cout<<"now compress matrix X..."<<endl;
		compressMatrixX(type, numMoveNodes);
		cout<<"now compress matrix Y..."<<endl;
		compressMatrixY(type, numMoveNodes);

		//checkMatrix();

		preValidNodes.resize(validNodes.size());
		preValidNodes = validNodes;

		cout<<"now solve matrix X..."<<endl;
		linearSolveX(type, numMoveNodes);
		cout<<"now solve matrix Y..."<<endl;
		linearSolveY(type, numMoveNodes);

		double newHpwl = getHPWL(plcTopBlock);
		time_t end_ = clock();
		cout << "[INFO] : step = " << i + 1 << " , hpwl = " << newHpwl
				<< ", time = " << double(end_ - start_) / CLOCKS_PER_SEC
				<< " sec." << endl;

		if ((newHpwl >= hpwl && i > 3) || i == maxSteps || newHpwl < 0) {
			//freeB2B();
//			cout << "ok1" << endl;
			hpwl = newHpwl;
			validNodes.resize(preValidNodes.size());
			validNodes = preValidNodes;
			break;
		} else if (abs(hpwl - newHpwl) / newHpwl < 0.05) {
			//freeB2B();
//			cout << "ok2" << endl;
			break;
		} else {
//			cout << "ok3" << endl;
			hpwl = newHpwl;
			freeB2B();
		}
	}
	cout << "[INFO] : Total iplace " << double(clock() - begin)
			/ CLOCKS_PER_SEC << " sec." << endl;
	//for debug
	//printOrder();
	return;
}

void SimPlPlace::randomDisperse() {
	vector<Inst*> &insts = plcTopBlock->getInsts();
	long numInsts = insts.size();
	//srand(time(NULL));
	/*
	 int sx[100] = {0};
	 int sy[100] = {0};
	 int ex = blockW / 100;
	 int ey = blockH / 100;
	 */

	for (long i = 0; i < numInsts; ++i) {
		if (insts[i]->getStatus() == Moved) {
			long x = rand() % blockW + blockX;
			long y = rand() % blockH + blockY;
			insts[i]->setOrigin(x - insts[i]->getWidth() / 2,
					y - insts[i]->getHeight() / 2);
			assert(insts[i]->getCenterX() >= region.left());
			assert(insts[i]->getCenterX() <= region.right());
			assert(insts[i]->getCenterY() >= region.bottom());
			assert(insts[i]->getCenterY() <= region.top());
			//if (i%5000 == 0) cout<<i<<" "<<x<<" "<<y<<endl;
			//sx[(x-blockX)/ex]++;
			//sy[(y-blockY)/ey]++;
		}
	}
	return;
}

void SimPlPlace::purposeDisperse() {
	//buildClique();
	buildHybrid();
	compressMatrixX(0, numMoveNodes + numStarNets);
	compressMatrixY(0, numMoveNodes + numStarNets);
	linearSolveX(0, numMoveNodes + numStarNets);
	linearSolveY(0, numMoveNodes + numStarNets);
	freeB2B();
	return;
}

void SimPlPlace::setBlockXYWH() {
	assert(region.left() == (long)region.left());
	assert(region.bottom() == (long)region.bottom());
	assert(region.width() == (long)region.width());
	assert(region.height() == (long)region.height());

	blockX = (long) region.left();
	blockY = (long) region.bottom();
	blockW = (long) region.width();
	blockH = (long) region.height();

	cout << "[INFO]: LEFT = " << blockX << ", RIGHT = " << blockX + blockW
			<< endl;
	cout << "[INFO]: BOTTOM = " << blockY << ", TOP = " << blockY + blockH
			<< endl;
	//long x;
	//cin>>x;
	return;
}

void SimPlPlace::filterInstsAndNets() {
	//for debug
	int totalPinNum = 0;

	//filter nets
	numStarNets = 0;
	validNets.clear();
	vector<myNet*>& totalNets = plcTopBlock->getNets();
	long netsNum = totalNets.size();
	cout << netsNum << " " << plcTopBlock->getNumNets() << endl;
	assert(netsNum == plcTopBlock->getNumNets());

	for (long id = 0; id < netsNum; ++id) {
		long pinsNum = totalNets[id]->getNumTerms();

		if (pinsNum > 500) {
			//cout<<"large nets: "<<totalNets[id]->getName()<<" "<<pinsNum<<endl;
			//continue;
		}

		if (pinsNum == 0) {
			//cout << "Warning : The net has 0 pins! Check it." << endl;
		} else if (pinsNum == 1) {
			//cout << "Tips : The net has 1 pin. Remove it." << endl;
		} else {
			totalPinNum += pinsNum;
			bool valid = false;
			vector<InstTerm>& terms = totalNets[id]->getTerms();
			long index1 = terms[0].getIndexInst();

			for (long j = 1; j < pinsNum; ++j) {
				long index2 = terms[j].getIndexInst();
				if (index1 != index2) {
					valid = true;
					break;
				}
			}
			if (valid) {
				validNets.push_back(totalNets[id]);
				if (totalNets[id]->getNumTerms() > 3) {
					numStarNets++;
				}
				for (long j = 0; j < pinsNum; ++j) {
					Inst *inst = getInst(terms[j]);
					inst->setIsValid(true);
				}
			} else {
				Inst *inst = getInst(terms[0]);
				//inst->setIsValid(false);
				//cout<<"invalid net# "<<totalNets[id]->getName()<<", nodes #"<<inst->getIndex()<<" "<<inst->getName()<<" "<<inst->getStatus()<<endl;
			}
		}
	}
	cout << "There are " << validNets.size() << "/" << netsNum
			<< " valid nets." << endl;
	cout<<"There are "<<totalPinNum<< " valid pins."<<endl;

	// filter inst:
	const int ISIZE = plcTopBlock->getInsts().size();
	int* numCon = new int[ISIZE];
	for (int i = 0; i < ISIZE; ++i) {
		numCon[i] = 0;
	}
	for (int i = 0; i < (long) validNets.size(); ++i) {
		myNet* ni = validNets[i];
		vector<InstTerm>& terms = ni->getTerms();
		const int TSIZE = terms.size();
		for (int j = 0; j < TSIZE; ++j) {
			int instIndex = terms[j].getIndexInst();
			if (instIndex >= 0 && instIndex < ISIZE) {
				numCon[instIndex]++;
			} else {
				cout << "ERROR: pin inst index out of bound!" << endl;
			}
		}
	}

	validNodes.clear(); //valid movable cells
	inValidNodes.clear();
	vector<Inst*>& totalInsts = plcTopBlock->getInsts();
	long instsNum = totalInsts.size();
	//cout<<"totalInstsize = "<<instsNum<<endl;
	assert(instsNum == plcTopBlock->getNumNodes());
	//restore moved cell
	for (long id = 0; id < instsNum; ++id) {
		if (totalInsts[id]->getStatus() != Moved) {
			continue;
		}
		if (!(totalInsts[id]->getIsValid())) {
			continue;
		}
		// check connectivity
		if (numCon[id] < 1) {
			cout << "invalid movable node " << id << endl;
			inValidNodes.push_back(totalInsts[id]);
			continue;
		}

		totalInsts[id]->setId(validNodes.size());
		//for debug
		//if (totalInsts[id]->getName() == "c_7_5_reg_5_"){
		//	cout<<"[DEBUG]: "<<totalInsts[id]->getName()<<" "<<totalInsts[id]->getNumInstTerms()<<endl;
		//	for (long i = 0; i < 3; ++i){
		//		cout<<validNets[totalInsts[id]->getInstTerms()[i].getIndexNet()]->getNumTerms()<<endl;
		//	}
		//}

		validNodes.push_back(totalInsts[id]);
	}
	numInvalidNodes = inValidNodes.size();
	numMoveNodes = validNodes.size();
	//restore fixed cell
	for (long id = 0; id < instsNum; ++id) {
		if (totalInsts[id]->getName().substr(0, 13) == "FixedRowSpace") {
			totalInsts[id]->setIsValid(true);
			totalInsts[id]->setId(validNodes.size());
			validNodes.push_back(totalInsts[id]);
		}

		if (!(totalInsts[id]->isRect())) { //non-rectangular nodes
#ifdef DEBUG
			cout<<"non rect node : "<<(totalInsts[id]->getName())<<endl;
#endif
			continue;
		}

		if (totalInsts[id]->getStatus() != Moved) {
			totalInsts[id]->setIsValid(true);
			totalInsts[id]->setId(validNodes.size());
			validNodes.push_back(totalInsts[id]);
		}

		if (!(totalInsts[id]->getIsValid())) {
			//cout<<id<<" is not valid"<<endl;
			continue;
		}

		// check connectivity
		if (numCon[id] < 1) {
			//cout<<"invalid fixed node "<<id<<" "<<totalInsts[id]->getName()<<" "<<totalInsts[id]->getStatus()<<endl;
			inValidNodes.push_back(totalInsts[id]);
			continue;
		}
	}
	numValidNodes = validNodes.size();
	//TODO unsure
	for (long i = 0; i < validNodes.size(); ++i) {
		if (validNodes[i]->getStatus() != Moved) {
			if (validNodes[i]->getCoordX() > region.right()) {
				validNodes[i]->setCoordX(region.right() + 10);
			}
			if (validNodes[i]->getCoordX() + validNodes[i]->getWidth()
					< region.left()) {
				validNodes[i]->setCoordX(
						region.left() - validNodes[i]->getWidth() - 10);
			}
			if (validNodes[i]->getCoordY() > region.top()) {
				validNodes[i]->setCoordY(region.top() + 10);
			}
			if (validNodes[i]->getCoordY() + validNodes[i]->getHeight()
					< region.bottom()) {
				validNodes[i]->setCoordY(
						region.bottom() - validNodes[i]->getHeight() - 10);
			}
		}
	}



	cout << "There are " << validNodes.size() << "/" << instsNum
			<< " valid insts." << endl;
	cout << "There are " << numValidNodes - numMoveNodes << " fixed insts."
			<< endl;
	delete[] numCon;
	return;
}

void SimPlPlace::checkMatrix() {
	//ofstream outMatrix;
	//outMatrix.open("matrix.log",ios::out);
	double rate = 0;
	double size = 0;
	double maxDiag = 0;
	double minDiag = INT_MAX;
	double temp = INT_MAX;
	double thisDiag;

	bool matrixX = true;
	//vector<Triple> copy;
	if (matrixX) {
		for (long i = 0; i < numMoveNodes; ++i) {
			double total = 0;
			temp = INT_MAX;
			thisDiag = -1;
			for (long j = Xp[i]; j < Xp[i + 1]; ++j) {
				if (Xx[j] < 0) {
					total += Xx[j];
					temp = min(temp, (abs)(Xx[j]));
				} else if (Xx[j] > 0) {
					maxDiag = max(maxDiag, Xx[j]);
					minDiag = min(minDiag, Xx[j]);
					assert(thisDiag == -1);
					thisDiag = Xx[j];
				} else {
					cout << "error!! zero element" << endl;
				}
			}
			assert(thisDiag > 0);
			if (thisDiag < (abs)(total) - 10e-6) {
				cout << " NOT diagonal dominance!!!" << endl;
				cout << i << " " << thisDiag << " " << (abs)(total) << endl;
			}
		}

		cout << "\tnumMoveNodes = " << numMoveNodes << endl;
		cout << "\tdiag max: " << maxDiag << ", min: " << minDiag
				<< ", diagonal rate: " << maxDiag / minDiag << endl;
		/*for (int i = 0; i < copy.size(); ++i){
		 cout<<copy[i].element<<endl;
		 }*/
		//outMatrix.close();
	} else {
	}

	//for debug
	long maxNetDegree = 0;
	for (long i = 0; i < validNets.size(); ++i) {
		maxNetDegree = max(maxNetDegree, validNets[i]->getNumTerms());
	}
	cout << "\tmaxNetDegree = " << maxNetDegree << endl;

	return;
}

bool lessCX1(Inst* cellA, Inst* cellB) {
	//return (cellA.inst->getCoordX() < cellB.inst->getCoordX());
	return (cellA->getCenterX() < cellB->getCenterX());
}

bool lessCY1(Inst* cellA, Inst* cellB) {
	return (cellA->getCenterY() < cellB->getCenterY());
}

void SimPlPlace::printOrder() {
	/*vector<Inst*> inst;
	 long pos = 0;
	 for (long i = 0; i < numMoveNodes; ++i){
	 inst.push_back(validNodes[i]);
	 }
	 sort(inst.begin(), inst.end(), lessCX1);
	 pos = inst[0]->getCenterX();
	 ofstream cfileX;
	 cfileX.open("instPositionX.info");
	 for (long i = 0; i < numMoveNodes; ++i){
	 if (inst[i]->getCenterX() == pos){
	 cfileX<<" "<<inst[i]->getId();
	 }
	 else{
	 cfileX<<endl<<inst[i]->getId();
	 }
	 }
	 cfileX.close();

	 sort(inst.begin(), inst.end(), lessCY1);
	 pos = inst[0]->getCenterY();
	 ofstream cfileY;
	 cfileY.open("instPositionY.info");
	 for (long i = 0; i < numMoveNodes; ++i){
	 if (inst[i]->getCenterY() == pos){
	 cfileY<<" "<<inst[i]->getId();
	 }
	 else{
	 cfileY<<endl<<inst[i]->getId();
	 }
	 }
	 cfileY.close();*/
	ofstream fout;
	vector<Inst*> insts;
	fout.open("netOrder.info");
	for (long i = 0; i < (long) validNets.size(); ++i) {
		vector<InstTerm>& terms = validNets[i]->getTerms();
		insts.clear();
		fout << "Net #" << validNets[i]->getIndex() << endl;
		for (long j = 0; j < (long) terms.size(); ++j) {
			Inst *inst = getInst(terms[j]);
			insts.push_back(inst);
		}
		sort(insts.begin(), insts.end(), lessCX1);
		fout << insts[0]->getIndex();
		for (long j = 1; j < (long) insts.size(); ++j) {
			if (insts[j]->getCenterX() == insts[j - 1]->getCenterX()) {
				fout << " " << insts[j]->getIndex();
			} else {
				fout << "  x= " << insts[j - 1]->getCenterX() << endl
						<< insts[j]->getIndex();
			}
		}
		fout << "  x= " << insts.back()->getCenterX() << endl << endl;
	}
}

//for debug
void SimPlPlace::loadSol(string xSolFile, string ySolFile) {
	ifstream finx;
	finx.open(xSolFile.c_str());
	ifstream finy;
	finy.open(ySolFile.c_str());
	double temp;
	for (long i = 0; i < numMoveNodes; ++i) {
		finx >> temp;
		validNodes[i]->setCoordX(temp - validNodes[i]->getWidth() / 2);
		finy >> temp;
		validNodes[i]->setCoordY(temp - validNodes[i]->getHeight() / 2);
	}
	finx.close();
	finy.close();
}

