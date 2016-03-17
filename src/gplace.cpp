#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
//#include "block.h"
#include <functional>
#include "simPlPlace.h"
#include "test.h"

using namespace std;

bool myMax(double a, double b) {
	return a > b;
}

bool myMin(double a, double b) {
	return a < b;
}

bool sameInst(Inst* A, Inst* B) {
	return (A->getId() == B->getId());
}

bool morePin(Inst* A, Inst* B) {
	return (A->getNumInstTerms() > B->getNumInstTerms());
}

bool lessID(Inst* A, Inst* B) {
	return (A->getId() < B->getId());
}

bool lessCX(const InstPos& cellA, const InstPos& cellB) {
	//return (cellA.inst->getCoordX() < cellB.inst->getCoordX());
	return (cellA.inst->getCenterX() < cellB.inst->getCenterX());
}

bool lessCenterX(Inst* cellA, Inst* cellB) {
	return (cellA->getCenterX() < cellB->getCenterX());
}

bool lessCenterY(Inst* cellA, Inst* cellB) {
	return (cellA->getCenterY() < cellB->getCenterY());
}

bool lessCoordX(Inst* cellA, Inst* cellB) {
	//return (cellA.inst->getCoordX() < cellB.inst->getCoordX());
	return (cellA->getCoordX() < cellB->getCoordX());
}

bool lessCoordY(Inst* cellA, Inst* cellB) {
	//return (cellA.inst->getCoordX() < cellB.inst->getCoordX());
	return (cellA->getCoordY() < cellB->getCoordY());
}

bool lessY(const InstPos& cellA, const InstPos& cellB) {
	return (cellA.oldPos.coordY()) < (cellB.oldPos.coordY());
}

bool lessCY(const InstPos& cellA, const InstPos& cellB) {
	return (cellA.inst->getCenterY()) < (cellB.inst->getCenterY());
}

bool lessXY(Inst* cellA, Inst* cellB) {
	return (cellA->getCoordY() < cellB->getCoordY());
}
bool greaterV1(const Triple& A, const Triple& B) {
	return (A.element > B.element);
}

bool lessCenterX_net(Inst* A, Inst* B) {
	if ((long) A->getCenterX() < (long) B->getCenterX()) {
		return true;
	} else if ((long) A->getCenterX() > (long) B->getCenterX()) {
		return false;
	} else {
		return (A->getGCX() < B->getGCX());
	}
}

bool lessCenterY_net(Inst* A, Inst* B) {
	if ((long) A->getCenterY() < (long) B->getCenterY()) {
		return true;
	} else if ((long) A->getCenterY() > (long) B->getCenterY()) {
		return false;
	} else {
		return (A->getGCY() < B->getGCY());
	}
}

bool lessRandX(Inst*A, Inst* B) {
	if ((long) A->getCenterX() < (long) B->getCenterX()) {
		return true;
	} else if ((long) A->getCenterX() > (long) B->getCenterX()) {
		return false;
	} else {
		return (A->getIndex() % 2 == 0);
	}
}

bool lessRandY(Inst* A, Inst* B) {
	if ((long) A->getCenterY() < (long) B->getCenterY()) {
		return true;
	} else if ((long) A->getCenterY() > (long) B->getCenterY()) {
		return false;
	} else {
		return (A->getIndex() % 2 == 0);
	}
}

void SimPlPlace::gPlace() {
	long TEST = 0;
	// set density overfilled factor
	//time_t begin = clock();
	setDensFactor();//overfilledFactor = cellArea / whiteSpace
	setAverageNodeArea();
	long minGSize = (long) (4 * averageNodeWidth);//the minimum grid size
	//cout<<"whole circut density : "<<getDensFactor()<<endl;
	//gridSize = plcTopBlock->getRows()[0]->getHeight() * 4;
	gridSize = blockW / 100;//initial grid size
	//	cout << "gridsize: " << gridSize << endl;
	//	while(1);
	const long MAXITER = 100;
	//const long MAXITER = 1;
	cout << "[INFO] : Begin gPlace HPWL = " << getHPWL(plcTopBlock) << endl;
	//double gap10 = 0;
	double ubHPWL = 0;
	double lbHPWL = 0;
	double minHPWL = 10e20;
	long negCount = 0;

	vector<double> coordX;
	vector<double> coordY;
	coordX.resize(numMoveNodes);
	coordY.resize(numMoveNodes);

	//choose ls solver
	long type = 0; //0-amgpcg or 3-iccg

	//DEBUG
//	cout << blockX << ' ' << blockX + (long) blockW << endl;
//	cout << blockY << ' ' << blockY + (long) blockH << endl;


	vector<CnNodes> CN;
	CN.clear();
	vector<long> edgeFixedNodes;
	edgeFixedNodes.clear();

	long tmp = 0;
	for (long i = 0; i < validNodes.size(); ++i){
		if (validNodes[i]->getStatus() == Moved){
			continue;
		}
		if (validNodes[i]->getCenterX() < 460 || validNodes[i]->getCenterX() > 11000
				|| validNodes[i]->getCenterY() < 460 || validNodes[i]->getCenterY() > 11000){
			edgeFixedNodes.push_back(validNodes[i]->getIndex());
//			cout << validNodes[i]->getIndex() << ' ' << i << endl;
		}
		tmp ++;
	}
//	cout << tmp << ' ' << edgeFixedNodes.size() << endl;
//	cout << "OK!" << endl;
//	cin >> TEST;

//	ofstream center_fixed("center_fixed",ios_base::out | ios_base::app);
//	for (long i = 0; i < validNodes.size(); ++i){
//		if (validNodes[i]->getStatus() == Moved){
//			continue;
//		}
//		center_fixed << validNodes[i]->getCenterX() << ' ' << validNodes[i]->getCenterY() << endl;
//	}

	//	Block block;
	//	SimPlPlace placement(&block);
//	ofstream f("force.out", ios_base::out | ios_base::app);


	ofstream up("Upper-Bound-Place_HPWL");
	ofstream low("Lower-Bound-Place_HPWL");
	for (long i = 1; i <= MAXITER; ++i) {

//		cout << "[INFO] : step = " << i << ", Input-Bound-Place-Before HPWL = "
//				<< getHPWL(plcTopBlock) << endl;
//		if (i >= 1) {
//			guiFile("inputBound_before.gnu");
//		}
		//Calculate F_hold

		vector<double> Fh_x(numMoveNodes, 0.0);
		vector<double> Fh_y(numMoveNodes, 0.0);
//		Fh_x.clear();
//		Fh_y.clear();

//		for (long k = 0; k < numMoveNodes; ++k) {
//			sort(B2B_MatrixX[k].begin(), B2B_MatrixX[k].end(), greater<Triple> ());
//		}
//		for (long k = 0; k < B2B_MatrixX.size(); ++k){
//			for(long t = 0; t < B2B_MatrixX[k].size(); ++t){
//				cout << B2B_MatrixX[k][t].row << ' ' << B2B_MatrixX[k][t].column
//						<< ' ' << B2B_MatrixX[k][t].element << endl;
//			}
//			cin >> TEST;
//		}
		for (long k = 0; k < numMoveNodes; ++k){
			double tmp_sum = 0.0;
			for(long t = 0; t < B2B_MatrixX[k].size(); ++t){
				tmp_sum += B2B_MatrixX[k][t].element
						* validNodes[B2B_MatrixX[k][t].column]->getCenterX();
			}
			Bx[k] = tmp_sum;
		}

		for (long k = 0; k < numMoveNodes; ++k){
			double tmp_sum = 0.0;
			for(long t = 0; t < B2B_MatrixY[k].size(); ++t){
				tmp_sum += B2B_MatrixY[k][t].element
						* validNodes[B2B_MatrixY[k][t].column]->getCenterY();
			}
			By[k] = tmp_sum;
		}

//		ofstream F_hold("F_hold",ios_base::out | ios_base::app);
//		for(long k = 0; k < Fh_x.size(); ++k){
//			F_hold << Fh_x[k] << endl;
//		}
//		cout << "OK" << endl;
//		cin >> TEST;


//		for (long k = 0; k < numMoveNodes; ++k) {
//			Bx[k] += Fh_x[k];
//			By[k] += Fh_y[k];
//		}
//		linearSolveX(type, numMoveNodes);
//		linearSolveY(type, numMoveNodes);
//		cout << "[INFO] : step = " << i << ", Input-Bound-Place-After HPWL = "
//				<< getHPWL(plcTopBlock) << endl;
//		if (i >= 1) {
//			guiFile("inputBound_after.gnu");
//			cout << "OK" << endl;
//			cin >> TEST;
//		}





		//TODO
		double targetT = 1.0;
		double overfiT = 1.0;
		//double targetT = getDensFactor();
		//double overfiT = getDensFactor();
		setTol(targetT, overfiT);

		//save old positions of cells
		saveOldPos();

		//setNetCenterOfInst();


		//cout<<"roughLegalization..."<<endl;

//		vector<myNet*>& nets = plcTopBlock->getNets();
//		ofstream hpwl_fixed("hpwl_fixed",ios_base::out | ios_base::app);
//
//		for(long k = 0; k < validNodes.size(); ++k){
//			if(validNodes[k]->getStatus() == Moved){
//				continue;
//			}
//			vector<InstTerm> pinsOnNode = validNodes[k]->getInstTerms();
//			long hpwl_net_be = 0;
//			for(long t = 0; t < pinsOnNode.size(); ++t){
//				hpwl_net_be += getHPWL(nets[pinsOnNode[t].getIndexNet()]);
//			}
//			hpwl_fixed << hpwl_net_be << endl;
//		}
//		hpwl_fixed << endl;

//		ofstream hpwl_l1("hpwl_l1",ios_base::out | ios_base::app);
//		vector<myNet*>& nets = plcTopBlock->getNets();
//		for (long k = 0; k < CN.size(); ++i){
//			vector<InstTerm> pinsOnNode = CN[k]->getInstTerms();
//			long hpwl_net = 0;
//			for(long t = 0; t < pinsOnNode.size(); ++t){
//				hpwl_net += getHPWL(nets[pinsOnNode[t].getIndexNet()]);
//			}
//			hpwl_l1 << hpwl_net << endl;
//		}
//		hpwl_l1 << endl;




		roughLegalization(1, 1, i);





//		vector<double> force;
//		for (long t = 0; t < validNodes.size(); ++t) {
//			double offX = validNodes[t]->getCenterX() - oldPos[t].coordX();
//			double offY = validNodes[t]->getCenterY() - oldPos[t].coordY();
//			double off = abs(offX) + abs(offY);
//			force.push_back(off);
//		}
//		sort(force.begin(), force.end(), myMin);
//		long num = 0;
//		for (long t = 0; t < force.size(); ++t) {
//			if(force[t] > 5000){
//				num ++;
//			}
//			f << force[t] << endl;
//		}
//		cout << "num = " << num << endl;
//
//		if(i == 10){
//			cout << "OK!" << endl;
//			cin >> TEST;
//		}





//		for(long k = 0; k < validNodes.size(); ++k){
//			if(validNodes[k]->getStatus() == Moved){
//				continue;
//			}
//			vector<InstTerm> pinsOnNode = validNodes[k]->getInstTerms();
//			long hpwl_net_af = 0;
//			for(long t = 0; t < pinsOnNode.size(); ++t){
//				hpwl_net_af += getHPWL(nets[pinsOnNode[t].getIndexNet()]);
//			}
//			hpwl_fixed << hpwl_net_af << endl;
//		}
//		hpwl_fixed << endl;




//		for (long k = 0; k < CN.size(); ++i){
//			vector<InstTerm> pinsOnNode = CN[k]->getInstTerms();
//			long hpwl_net = 0;
//			for(long t = 0; t < pinsOnNode.size(); ++t){
//				hpwl_net += getHPWL(nets[pinsOnNode[t].getIndexNet()]);
//			}
//			hpwl_l1 << hpwl_net << endl;
//		}
//		hpwl_l1 << endl;






		//globalRefine();


		/*if (i % 2 == 0 && i > 5){
		 for (long s = 0; s < 4; ++s){
		 binBasedCellInflation();
		 //congestionEstimate("");
		 }
		 }*/

		//adjust gridSize
		gridSize = (long) (gridSize / 1.06);
		if (gridSize < minGSize) {
			cout << "minGSize = " << minGSize << ", gridSize = " << gridSize
					<< ", row height = "
					<< plcTopBlock->getRows()[0]->getHeight() << endl;
			gridSize = minGSize;
		}

		//update B2B model
		double weight;
		//if (i <= 20){
		weight = 0.002 * (1 + i);
		//}
		//else{
		//	weight = 0.1 + 0.01 * (i - 20);
		//}
		//weight = 10;

		updateB2B(type, weight);

		ubHPWL = getHPWL(plcTopBlock);

		//		if (ubHPWL < minHPWL) {
		//			minHPWL = ubHPWL;
		//			for (long k = 0; k < numMoveNodes; ++k) {
		//				assert(validNodes[k]->getStatus() == Moved);
		//				coordX[k] = validNodes[k]->getCoordX();
		//				coordY[k] = validNodes[k]->getCoordY();
		//			}
		//			negCount = 0;
		//		} else {
		//			negCount++;
		//		}


		cout << "[INFO] : step = " << i << ", Upper-Bound-Place HPWL = "
				<< getHPWL(plcTopBlock) << endl;
		// ofstream up("Upper-Bound-Place_HPWL",ios_base::out | ios_base::app);
		up << i << ", " << getHPWL(plcTopBlock) << endl;
		//<< ",  time = " <<double(clock() - start1) / CLOCKS_PER_SEC<<" sec."<<endl;
		//time_t start2 = clock();

		// control the convergance critiria
		/*if (i == 10){
		 gap10 = ubHPWL - lbHPWL;
		 }
		 if (i > 10) {
		 if ((ubHPWL - lbHPWL) / gap10 - 0.1 < 0) {
		 break;
		 }
		 }*/

		if (i == MAXITER) {
			break;
		}

		//if (negCount >= 10){
		//	break;
		//}

		//constraintMove(0.017 * i);

//		if (i >= 1) {
//			guiFile("upperBound.gnu");
//		}


		//cout<<"upper bound..."<<endl;

		//DEBUG
//		saveOldPos();

		linearSolveX(type, numMoveNodes);
		//		if(i == 19){
		//			int a;
		//	     	cin >> a;
		//		}
		linearSolveY(type, numMoveNodes);
		//		if(i == 19){
		//			while(1);
		//		}



		//TODO unsure
		freeB2B();
		buildB2B();
		compressMatrixX(type, numMoveNodes);
		compressMatrixY(type, numMoveNodes);

		cout << "[INFO] : step = " << i << ", Lower-Bound-Place HPWL = "
				<< getHPWL(plcTopBlock) << endl;
		// ofstream low("Lower-Bound-Place_HPWL",ios_base::out | ios_base::app);
		low << i << ", " << getHPWL(plcTopBlock) << endl;
		//<<",  time = "<<double(clock() - start2) / CLOCKS_PER_SEC<<" sec."<<endl<<endl;
		lbHPWL = getHPWL(plcTopBlock);

//		if (i >= 1) {
//			guiFile("lowerBound.gnu");
//			cout << "OK!" << endl;
//			cin >> TEST;
//		}


		//DEBUG
		//		if(i >= 36){
		//			guiFile("lowerBound.gnu");
		//			long TEST;
		//			cout << "Input: " << endl;
		//			cin >> TEST;
		//		}
	}
	//TODO
	/*for (long i = 0; i < numMoveNodes; ++i){
	 validNodes[i]->setCoordX(coordX[i]);
	 validNodes[i]->setCoordY(coordY[i]);
 	 }*/

	//	for (long i = 0; i < insts.size(); ++i) {
	//insts[i]->setOWidth(owidth[i]);
	//		insts[i]->setOHeight(oheight[i]);
	//		insts[i]->setHeight(oheight[i]);
	//	}

	cout << "[INFO] :End gPlace HPWL = " << getHPWL(plcTopBlock) << endl;
	//double gtime = double(clock() - begin) / CLOCKS_PER_SEC;

	//cout<<"[INFO] : Total gplace(including NCTUgr) "<< gtime <<" sec."<<endl;
}


void SimPlPlace::flattenRL() {
	long left = region.left();
	long right = region.right();
	long bottom = region.bottom();
	long top = region.top();
	double stepX, stepY;

	vector<Inst*> instsX;
	vector<Inst*> instsY;
	instsX.clear();
	instsY.clear();

	stepX = 1.0 * (right - left) / numMoveNodes;
	stepY = 1.0 * (top - bottom) / numMoveNodes;

	for (long i = 0; i < numMoveNodes; ++i) {
		assert(validNodes[i]->getStatus() == Moved);
		instsX.push_back(validNodes[i]);
		instsY.push_back(validNodes[i]);
	}
	sort(instsX.begin(), instsX.end(), lessCoordX);
	sort(instsY.begin(), instsY.end(), lessCoordY);

	for (long i = 0; i < numMoveNodes; ++i) {
		instsX[i]->setCoordX(left + i * stepX);
		instsY[i]->setCoordY(bottom + i * stepY);
	}
}

void SimPlPlace::roughLegalization(long h, long v , long step) {
	long TEST = 0;
	ofstream fout;
	//fout.open("peakDensity.txt", ios::app);
	//cout<<"now buildGrids..."<<endl;
	buildGrids();

	//cout<<"now getBinsUsage..."<<endl;
	getBinsUsage(step);

	//cout<<"now findOverfilledBins..."<<endl;
	findOverfilledBins();
	//ofstream overf("overfilledBin", ios_base::out | ios_base::app);
	cout << "[DEBUG]: overfilledBin = " << overfilledBins.size() << endl;
	//overf << overfilledBins.size() << endl;
	//fout<<overfilledBins[0].element<<endl;
	//fout.close();
	long iter = 0;

	//DEBUG

	lookAheadLegalize(h, v, step);

	getBinsUsage(step);



	return;
}

void SimPlPlace::setDensFactor() {
	double cellArea = 0;
	double whiteSpace = 0;
	double obstacleArea = 0;
	//DEBUG
	//	cout << "blockX= " << blockX << endl << "blockW= " << blockW << endl <<
	//			"blockY= " << blockY << endl << "blockH= " << blockH << endl;
	//	while(1);

	double totalArea = (double) blockW * (double) blockH;
	for (long i = 0; i < numMoveNodes; i++) {
		assert(validNodes[i]->getStatus() == Moved);
		cellArea += (double) validNodes[i]->getArea();
	}
	for (long i = numMoveNodes; i < numValidNodes; i++) {
		if (validNodes[i]->getStatus() == Fixed) {
			if ((long) validNodes[i]->getCoordX() >= (long) blockX + blockW
					|| (long) validNodes[i]->getCoordX()
							+ validNodes[i]->getWidth() <= (long) blockX
					|| (long) validNodes[i]->getCoordY() >= blockY + blockH
					|| (long) validNodes[i]->getCoordY()
							+ validNodes[i]->getHeight() <= (long) blockY) {
				continue;
			}
			obstacleArea += validNodes[i]->getArea();
		}
	}
	whiteSpace = totalArea - obstacleArea;
	//cout<<"totalArea = "<<totalArea<<endl;
	//cout<<"totalCellArea = "<<cellArea<<endl;
	//cout<<"totalAvailable = "<<whiteSpace<<endl;
	if (obstacleArea / totalArea < 0.10) {
		simplified = true;
	} else {
		simplified = false;
	}
	overfilledFactor = cellArea / whiteSpace;
	//cout<<"[DEBUG_ZQ]: overfilledFator is "<<overfilledFactor<<endl;

	//cout<<(cellArea + obstacleArea) / totalArea<<endl;
	//cout<<cellArea<<" "<<obstacleArea<<" "<<totalArea<<endl;
}
void SimPlPlace::setAverageNodeArea() {
	double totalArea = 0;
	double totalHeight = 0;
	double totalWidth = 0;
	long maxWidth = 0;
	for (long i = 0; i < numMoveNodes; ++i) {
		totalArea += (double) validNodes[i]->getArea();
		totalWidth += (double) validNodes[i]->getWidth();
		totalHeight += (double) validNodes[i]->getHeight();
		maxWidth = maxWidth > (long) (validNodes[i]->getWidth()) ? maxWidth
				: (long) validNodes[i]->getWidth();
	}
	averageNodeArea = totalArea / numMoveNodes;
	averageNodeWidth = totalWidth / numMoveNodes;
	averageNodeHeight = totalHeight / numMoveNodes;

	//cout<<"averageNodeArea = "<<averageNodeArea<<endl;
	//cout<<"averageNodeWidth = "<<averageNodeWidth<<endl;
	//cout<<"averageNodeHeight = "<<averageNodeHeight<<endl;
	//cout<<"maxWidth = "<<maxWidth<<endl;

	return;
}

double SimPlPlace::getDensFactor() {
	return overfilledFactor;
}

void SimPlPlace::updateB2B(int type, double weight) {
	double base = 2.0;
	double eps = 0.1;
	double param = 100.0;
	long TEST = 0;


	//DEBUG
//	vector<myNet*>& nets = plcTopBlock->getNets();
//	cout << validNodes[0]->getInstTerms().size() << endl;
//	cout << nets[validNodes[0]->getInstTerms()[0].getIndexNet()]->getTerms().size() << endl;
//	cout << validNodes[nets[validNodes[0]->getInstTerms()[0].getIndexNet()]->getTerms()[0].getIndexInst()]->getCenterX() << endl;
//	cout << "OK!" << endl;
//	cin >> TEST;


	for (long i = 0; i < numMoveNodes; ++i) {
		//compute length
		double oldX = oldPos[i].coordX();
		double oldY = oldPos[i].coordY();
		double newX = validNodes[i]->getCenterX();
		double newY = validNodes[i]->getCenterY();
		double offX = oldX - newX;
		double offY = oldY - newY;


		//find the pos of diagoal element
		long id = validNodes[i]->getId();
		long indexX;
		long indexY;
		if (type == 0 || type == 1 || type == 2) {
			indexX = diagX[id];
			indexY = diagY[id];
		}

		//update matrix
//		cout << "Xx_size = " << Xx.size() << endl;
		if (offX != 0) {
			double w = base / (1.0 * (abs(offX * param) + eps * param));
//			double w = base / (abs(offX) + eps);
			w = w * weight;//  abs(offX);
//			cout << "bx_before = " << Bx[id] << endl;
//			cout << "bx_after = " << Bx[id] + (w * validNodes[i]->getCenterX()) << endl;
			double tmp_x = w * validNodes[i]->getCenterX();
			Bx[id] += tmp_x;
			if (type == 0 || type == 1 || type == 2) {
//				cout << "w_before = " << Xx[indexX] << endl;
//				cout << "w_after = " << Xx[indexX] + w << endl;
				Xx[indexX] += w;
			} else if (type == 3) {
				//linearSolver
				assert(B2B_MatrixX[i].back().row == i);
				assert(B2B_MatrixX[i].back().column == i);
				B2B_MatrixX[i].back().element += w;
			}
//			if(Bx[id] - (w * validNodes[i]->getCenterX()) != 0){
//				cin >> TEST;
//			}
			//cout<<"Weight = "<<w<<"   weight * newX = "<< w * newX<<endl;
		}

		if (offY != 0) {
			double w = base / (1.0 * (abs(offY * param) + eps * param));
//			double w = base / (abs(offY) + eps);
			w = w * weight;//  abs(offY);
			double tmp_y = w * validNodes[i]->getCenterY();
			By[id] += tmp_y;
			if (type == 0 || type == 1 || type == 2) {
				Yx[indexY] += w;
			} else if (type == 3) {
				//linearSolver
				assert(B2B_MatrixY[i].back().row == i);
				assert(B2B_MatrixY[i].back().column == i);
				B2B_MatrixY[i].back().element += w;
			}
		}
	}
	return;
}

void SimPlPlace::buildGrids() {
	gridNumX = blockW / gridSize + 1;
	gridNumY = blockH / gridSize + 1;
	//cout<<"[INFO]: "<<blockW<<" "<<gridSize<<" "<<gridNumX<<endl;
	//cout<<"[INFO]: "<<blockH<<" "<<gridSize<<" "<<gridNumY<<endl;

	bins.resize(gridNumX);
	for (long i = 0; i < gridNumX; ++i) {
		bins[i].resize(gridNumY);
	}
	gridCoordX.resize(gridNumX + 1);
	gridCoordY.resize(gridNumY + 1);
	// X left grid and Y bottom grid
	gridCoordX[0] = blockX;//the left bound of the grid
	gridCoordY[0] = blockY;//the bottom bound of the grid
	// center grid
	for (long i = 1; i < gridNumX; ++i) {
		gridCoordX[i] = gridCoordX[i - 1] + gridSize;
	}
	for (long i = 1; i < gridNumY; ++i) {
		gridCoordY[i] = gridCoordY[i - 1] + gridSize;
	}
	// X-Y direction, note the last grid
	gridCoordX[gridNumX] = blockX + blockW;
	gridCoordY[gridNumY] = blockY + blockH;

	for (long i = 0; i < gridNumX; ++i) {
		for (long j = 0; j < gridNumY; ++j) {
			RLRegion* newBin = new RLRegion(gridCoordX[i], gridCoordX[i + 1],
					gridCoordY[j], gridCoordY[j + 1]);
			bins[i][j] = newBin;
		}
	}
}

void SimPlPlace::getBinsUsage(long step) {
	long TEST = 0;
	//malloc memory
	//double gridArea = 1.0 * gridSizeX * gridSizeY; //Note: gridArea inaccurate
	for (long i = 0; i < gridNumX; i++) {
		for (long j = 0; j < gridNumY; j++) {
			bins[i][j]->clearData();
		}
	}

	//get all the bins density
	for (long i = 0; i < numValidNodes; i++) {
		double xl = validNodes[i] -> getCoordX();
		double xh = xl + validNodes[i] -> getWidth();
		double yl = validNodes[i] -> getCoordY();
		double yh = yl + validNodes[i] -> getHeight();

		assert(xh >= xl);
		assert(yh >= yl);

//		cout << blockX << ' ' << blockX + (long) blockW << endl;
//		cout << blockY << ' ' << blockY + (long) blockH << endl;
//		cin >> TEST;
//		if(step >= 18 && validNodes[i]->getCenterX() == 11152.2){
//			cout << "Pay Attention!" << endl;
//			cin >> TEST;
//		}
//		if(i == 254294 && gridSize == 50){
//			cout << "Pay Attention!" << endl;
//			cin >> TEST;
//		}
		//move some Movable insts into the block
		if (xh <= blockX || xl >= blockX + (long) blockW || yh <= blockY || yl//move into the block
				>= blockY + (long) blockH) {
			if (validNodes[i]->getStatus() == Moved) {
				if (xh <= blockX) {
					validNodes[i]->setCoordX(blockX);
					xl = blockX;
					xh = xl + validNodes[i]->getWidth();
//					cout << "1" << endl;
				} else if (xl >= blockX + (long) blockW) {
					validNodes[i]->setCoordX(
							blockX + blockW - validNodes[i]->getWidth());
					xl = validNodes[i]->getCoordX();
					xh = blockX + blockW;
//					cout << "2" << endl;
				}
				if (yh <= blockY) {
					validNodes[i]->setCoordY(blockY);
					yl = blockY;
					yh = yl + validNodes[i]->getHeight();
//					cout << "3" << endl;
				} else if (yl >= blockY + (long) blockH) {
					validNodes[i]->setCoordY(
							blockY + blockH - validNodes[i]->getHeight());
					yl = validNodes[i]->getCoordY();
					yh = blockY + blockH;
//					cout << "4" << endl;
				}
//				if(step >= 18 && validNodes[i]->getCenterX() == 11152.2){
//					cout << blockX << ' ' << blockX + blockW << endl;
//					cout << blockY << ' ' << blockY + blockH << endl;
//					cout << validNodes[i]->getCoordX() << ' ' << validNodes[i]->getWidth() << ' ' << validNodes[i]->getCenterX() << endl;
//					cout << validNodes[i]->getCoordY() << ' ' << validNodes[i]->getHeight() << ' ' << validNodes[i]->getCenterY() << endl;
//					cin >> TEST;
//				}
			} else {
				continue;
			}
		}

		//the start and end grid of the inst
		long xStart = (long) ((xl - blockX) / gridSize);
		long xEnd = (long) ((xh - blockX) / gridSize);
		long yStart = (long) ((yl - blockY) / gridSize);
		long yEnd = (long) ((yh - blockY) / gridSize);

		xStart = xStart < 0 ? 0 : xStart;
		yStart = yStart < 0 ? 0 : yStart;
		xEnd = xEnd < gridNumX ? xEnd : (gridNumX - 1);
		yEnd = yEnd < gridNumY ? yEnd : (gridNumY - 1);


		if (validNodes[i]->getStatus() == Moved) {
			long xGridCoord = floor((validNodes[i]->getCenterX() - blockX) / gridSize);
			//		cout << xGridCoord << endl;
			//		cin >> TEST;
			long yGridCoord = floor((validNodes[i]->getCenterY() - blockY) / gridSize);
			//		cout << yGridCoord << endl;
			//		cin >> TEST;
//			if(step >= 18 && validNodes[i]->getCenterX() == 11152.2){
//				cout << validNodes[i]->getCenterX() << ' ' << validNodes[i]->getCenterY() << endl;
//			}
			if(validNodes[i]->getCenterX() < blockX
					|| validNodes[i]->getCenterX() > blockX + blockW
					|| validNodes[i]->getCenterY() < blockY
					|| validNodes[i]->getCenterY() > blockY + blockH){
				cout << "Error: " << i << ' ' << gridSize << endl;
				cout << "gridNumX = " << gridNumX << ' ' << "gridNumY = " << gridNumY << endl;
				cout << "xGridCoord = " << xGridCoord << ' ' << "yGridCoord = " << yGridCoord << endl;
				cout << validNodes[i]->getCenterX() << ' ' << blockX << ' ' << validNodes[i]->getCenterY() << ' ' << blockY << endl;
			}else{
				bins[xGridCoord][yGridCoord]->moveInstsX.push_back(validNodes[i]);
			}
//			if(xGridCoord < 0 || xGridCoord >= gridNumX || yGridCoord < 0 || yGridCoord >= gridNumY){
//				cout << "Error: " << i << ' ' << gridSize << endl;
//				cout << "gridNumX = " << gridNumX << ' ' << "gridNumY = " << gridNumY << endl;
//				cout << "xGridCoord = " << xGridCoord << ' ' << "yGridCoord = " << yGridCoord << endl;
//				cout << validNodes[i]->getCenterX() << ' ' << blockX << ' ' << validNodes[i]->getCenterY() << ' ' << blockY << endl;
////				cin >> TEST;
//			}else{
//				bins[xGridCoord][yGridCoord]->moveInstsX.push_back(validNodes[i]);
//			}


			//		cout << bins[xGridCoord][yGridCoord]->moveInstsX[bins[xGridCoord][yGridCoord]->moveInstsX.size()-1]->getCenterX();
			//		cin >> TEST;
//			bins[xGridCoord][yGridCoord]->moveInstsY.push_back(validNodes[i]);
			//		cout << bins[xGridCoord][yGridCoord]->moveInstsY[bins[xGridCoord][yGridCoord]->moveInstsY.size()-1]->getCenterY();
			//		cin >> TEST;
		}else if(validNodes[i]->getStatus() == Fixed){
			validNodes[i]->initIncluded();
//			cout << validNodes[i]->getCoordX() << ' ' << validNodes[i]->getWidth() << endl;
//			cout << validNodes[i]->getCoordY() << ' ' << validNodes[i]->getHeight() << endl;
			for (long x = xStart; x <= xEnd; ++x) {
				for (long y = yStart; y <= yEnd; ++y) {
//					cout << bins[x][y]->left << ' ' << bins[x][y]->right << ' ' << bins[x][y]->bottom << ' ' << bins[x][y]->top << endl;
					bins[x][y]->fixInsts.push_back(validNodes[i]);
//					cin >> TEST;
				}
			}
		}

		//Important!Split one inst'area to several grids(bins),then calculate the summation of the total bin area
		for (long j = xStart; j <= xEnd; ++j) {
			for (long k = yStart; k <= yEnd; ++k) {
				double left = xl > gridCoordX[j] ? xl : gridCoordX[j];
				double right = xh < gridCoordX[j + 1] ? xh : gridCoordX[j + 1];
				double down = yl > gridCoordY[k] ? yl : gridCoordY[k];
				double up = yh < gridCoordY[k + 1] ? yh : gridCoordY[k + 1];
				assert(right >= left);
				assert(up >= down);
				if (validNodes[i]->getStatus() == Moved) {
					bins[j][k]->cellArea += 1.0 * (right - left) * (up - down);
					assert(bins[j][k]->cellArea >= 0);
				} else if (validNodes[i]->getStatus() == Fixed) {
					bins[j][k]->availableArea -= 1.0 * (right - left) * (up
							- down);
					if (bins[j][k]->availableArea < 0) {
						bins[j][k]->availableArea = 0;
						//cout<<j<<" "<<k<<" "<<bins[j][k]->availableArea<<endl;
						//cout<<left<<" "<<right<<" "<<down<<" "<<up<<endl;
						//cout<<(right - left) * (up - down)<<endl;
						//cout<<validNodes[i]->getName()<<" "<<validNodes[i]->getStatus()<<" "<<validNodes[i]->getCoordX()<<" "<<validNodes[i]->getCoordY()<<endl;
					}
					assert(bins[j][k]->availableArea >= 0);
				}
			}
		}
	}
//	cout << "OK!" << endl;



//	cout << bins[49][49]->moveInstsX.size() << endl;

//	for (long i = 0; i < gridNumX; ++i){
//		for(long j = 0; j < gridNumY; ++j){
//			if(bins[i][j]->moveInstsX.size() > 0){
//				cout << i << ' ' << j << ' ' << endl;
//				cout << bins[i][j]->left << ' ' << bins[i][j]->right << ' ' << bins[i][j]->bottom << ' ' << bins[i][j]->top << endl;
//				for(long k = 0; k < bins[i][j]->moveInstsX.size(); ++k){
//					cout << bins[i][j]->moveInstsX[k]->getCenterX() << ' ' << bins[i][j]->moveInstsX[k]->getCenterY() << endl;
//				}
//				cout << "OK!" << endl;
//				cin >> TEST;
//			}
//		}
//	}


	return;
}

void SimPlPlace::findOverfilledBins() {
	//for debug
	//double total = 0;
	//cout << "Begin Find Overfilled Bins " << endl;
	overfilledBins.clear();
//	cout << bins[66][45]->getDensity() << endl;
//	while(1);
	// ergodic the bins, search overfilled ones
	for (long i = 0; i < gridNumX; i++) {
		for (long j = 0; j < gridNumY; j++) {
			//cout<<i<<" "<<j<<" "<<bins[i][j]->getDensity()<<endl;
			//cout<<bins[i][j]->cellArea<<" "<<bins[i][j]->availableArea<<endl;
			//total += bins[i][j]->cellArea;
			//cout<<bins[i][j]->availableArea<<" "<<bins[i][j]->getArea()<<endl;
			//TODO

			//DEBUG
			if (bins[i][j]->getDensity() > overfillTol
					&& bins[i][j]->availableArea / bins[i][j]->getArea() > 0.2) {
//			if (bins[i][j]->getDensity() > overfillTol) {
				Triple tri(i, j, bins[i][j]->getDensity());
				overfilledBins.push_back(tri);
			}
		}
	}
	//cout<<"total!! = "<<total<<endl;

	//sorting the overfilled bins with their density
	//ofstream maxU("maxUtilization", ios_base::out | ios_base::app);
	if (overfilledBins.size() > 0) {
		sort(overfilledBins.begin(), overfilledBins.end(), greaterV1);
		cout << "[INFO] : maxUtilization = " << overfilledBins[0].element
				<< endl;
	//	maxU << overfilledBins[0].element << endl;
	} else {
		cout << "[WARNING]: no overfilled bins were found!!!" << endl;
	}
#ifdef DEBUG
	for (long i = 1; i < (long)overfilledBins.size(); ++i) {
		cout << i<< ": overfilledBin(" << overfilledBins[i].row
		<< ", " << overfilledBins[i].column << ") ="
		<< overfilledBins[i].element << endl;
		assert(overfilledBins[i-1].element >= overfilledBins[i].element);
	}
#endif
	//cout << "Finish Find Overfilled Bins: " << overfilledBins.size() << endl;
	return;
}

#if 0
void SimPlPlace::lookAheadLegalize(long h, long v) {
	// find the target region
	vector<RLRegion*> clusters;
	for (long i = 0; i < (long)overfilledBins.size(); ++i) {
		// get cluster
		if (bins[overfilledBins[i].row][overfilledBins[i].column]->isClustered()) {
			continue;
		}
		RLRegion* newCluster = NULL;
		newCluster = getCluster(overfilledBins[i], h, v);
		if (newCluster != NULL) {
			getCellsInCluster(newCluster);
			//checkOrder(newCluster->moveInstsX, true);
			//checkOrder(newCluster->moveInstsY, false);
			clusters.push_back(newCluster);
		}
		else {
			delete newCluster;
		}
	}
	cout<<"[INFO] : numCluster = "<<clusters.size()<<endl;
	//cout<<clusters[0]->left<<" "<<clusters[0]->right<<" "<<clusters[0]->bottom<<" "<<clusters[0]->top<<endl;
	//guiRect("clusters.gnu", clusters[0]->left, clusters[0]->right, clusters[0]->bottom, clusters[0]->top);
	for (long i = 0; i < clusters.size(); ++i) {
		centerDiffusion(clusters[i]);
	}
}
#endif

#if 1
void SimPlPlace::lookAheadLegalize(long h, long v, long step) {
	// find LAL aregion
	vector<RLRegion*> clusters;
	long TEST = 0;
	//DEBUG
//	cout << bins[overfilledBins[0].row][overfilledBins[0].column]->left << ' '
//			<< bins[overfilledBins[0].row][overfilledBins[0].column]->bottom
//			<< endl;
//	cin >> TEST;
	//	for (long i = (long) overfilledBins.size()-1; i >= 0; --i) {
	for (long i = 0; i < (long) overfilledBins.size(); ++i) {
		// get cluster
		if (bins[overfilledBins[i].row][overfilledBins[i].column]->isClustered()) {
			continue;
		}
		RLRegion* newCluster = NULL;
		newCluster = getCluster(overfilledBins[i], h, v, step);
		if (newCluster != NULL) {
			getCellsInCluster(newCluster);
			//checkOrder(newCluster->moveInstsX, true);
			//checkOrder(newCluster->moveInstsY, false);
			clusters.push_back(newCluster);

			//DEBUG
//			if (step >= 1) {
////				cout << overfilledBins.size() << endl;
//				cout << "step3 " << step << endl;
////				cout << i << ' ' << overfilledBins[i].element << endl;
////						gridSize*overfilledBins[i].row << ' ' <<
////						gridSize*overfilledBins[i].column << endl;
//				guiClustersGroup("Cluster_group.gnu", clusters , overfilledBins);
//				cout << "OK! " << endl;
//				cin >> TEST;
//			}
		} else {
			delete newCluster;
		}
	}



//	ofstream numC("numCluster", ios_base::out | ios_base::app);
	cout << "[INFO] : numCluster = " << clusters.size() << endl;
//	numC << clusters.size() << endl;

	//DEBUG for narrow boundingBox
//	cout<<clusters[0]->left<<" "<<clusters[0]->right<<" "<<clusters[0]->bottom<<" "<<clusters[0]->top<<endl;
//	long TEST=0;
//	double ratio_00 = 10.0;
//	double ratio_11 = 1.0 / 10.0;
//	guiClustersGroup("clusters_group.gnu",clusters);
//	ofstream clu("ClusterShape");
//	for(long i=0;i<(long)clusters.size();++i){
//		if(clusters[i]->getWidth() < clusters[i]->getHeight()){
//			clu << clusters[i]->getWidth() << ' ' << clusters[i]->getHeight() << endl;
//		}else{
//			clu << clusters[i]->getHeight() << ' ' << clusters[i]->getWidth() << endl;
//		}
//		if(clusters[i]->getWidth() / clusters[i]->getHeight() > ratio_00 ||
//				(1.0 * clusters[i]->getWidth() / clusters[i]->getHeight()) < ratio_11){\
//			cout << clusters[i]->getWidth() << endl;
//			cout << clusters[i]->getHeight() << endl;
//			numCluster++;
//		}
//		guiRect("clusters.gnu", clusters[i]->left, clusters[i]->right, clusters[i]->bottom, clusters[i]->top);
//		if(clusters[i]->getWidth() / clusters[i]->getHeight() > ratio_00 ||
//				(1.0 * clusters[i]->getWidth() / clusters[i]->getHeight()) < ratio_11){
//			guiRect("clusters.gnu", clusters[i]->left, clusters[i]->right, clusters[i]->bottom, clusters[i]->top);
//			cout << clusters[i]->getWidth() << endl;
//			cout << clusters[i]->getHeight() << endl;
//			numCluster++;
//			cin >> TEST;
//		}
//	}
//	clu.close();
//	cout << "Write the clusters shape is OK! " << endl;
//	if(clusters.size() > 50){
//		cin >> TEST;
//	}

//	while(1);

	//spread cells in subregions
	int level = 10;
	for (long i = 0; i < (long) clusters.size(); ++i) {
		//checkOrder(clusters[i]->moveInstsX, true);
		//checkOrder(clusters[i]->moveInstsY, false);
		RectLevel rl;
		rl.level = level;
		rl.region = clusters[i];
		while (!subRegions.empty()) {
			subRegions.pop();
		}
		subRegions.push(rl);

		while (!subRegions.empty()) {
			// diffusion cells in the cluster
			RectLevel rlt = subRegions.front();
			subRegions.pop();
			RLRegion* r = rlt.region;
			int l = rlt.level;

			diffusion(r, l);
		}
		//subCluster.clear();
	}
	//cout << "Finish Rough Legalization " << endl;
}
#endif

RLRegion* SimPlPlace::getCluster(Triple& overfilledBin, long h, long v, long step) {


	long TEST;
	long gridX_L = overfilledBin.row;
	long gridX_R = gridX_L;
	long gridY_B = overfilledBin.column;
	long gridY_T = gridY_B;
	long extend_x = 1;
	long extend_y = 1;
	bool extend[2];

	double tmpTargetTol = 0;
	//for debug
	/*for (long i = 0; i < gridNumX; ++i){
	 for (long j = 0; j < gridNumY; ++j){
	 assert(bins[i][j]->availableArea <= bins[i][j]->getArea() + 10e-6);
	 if (bins[i][j]->isClustered()){
	 cout<<i<<"/"<<gridNumX<<", "<<j<<"/"<<gridNumY<<endl;
	 }
	 assert(!bins[i][j]->isClustered());
	 }
	 }*/
	RLRegion* cluster = new RLRegion();

	double areaCell = 0;
	double areaAvailable = 0;
	double area = 0;//for debug

	//if the circuit has few fixed blocks, treat the whole placement region as a cluster
	//TODO unsure
	if (0) {
		//if (simplified && h == v){
		//cout<<"simplified!!"<<endl;
		for (long i = 0; i < gridNumX; ++i) {
			for (long j = 0; j < gridNumY; ++j) {
				areaCell += bins[i][j]->cellArea;
				areaAvailable += bins[i][j]->availableArea;
				area += bins[i][j]->getArea();
				bins[i][j]->setClustered(true);
			}
		}
		cluster->left = bins[0][0]->left;
		cluster->right = bins[gridNumX - 1][gridNumY - 1]->right;
		cluster->bottom = bins[0][0]->bottom;
		cluster->top = bins[gridNumX - 1][gridNumY - 1]->top;
		cluster->cellArea = areaCell;
		cluster->availableArea = areaAvailable;
		return cluster;
	}

	if (simplified && h != v) {
		tmpTargetTol = targetTol;
		targetTol = 1.0;
	} else {
		tmpTargetTol = targetTol;
	}

	areaCell += bins[gridX_L][gridY_B]->cellArea;
	areaAvailable += bins[gridX_L][gridY_B]->availableArea;
	area += bins[gridX_L][gridY_B]->getArea();//for debug

	//overfilled bin is not big enough
	if (areaAvailable < 3 * averageNodeArea) {
		//cout<<bins[gridX_L][gridY_B]->left<<" "<<bins[gridX_L][gridY_B]->right<<endl;
		//cout<<bins[gridX_L][gridY_B]->bottom<<" "<<bins[gridX_L][gridY_B]->top<<endl;
		//cout<<"overfilled bin is not big enough"<<endl;
		delete cluster;
		return NULL;
	}

	bins[gridX_L][gridY_B]->setClustered(true);

	//get a cluster of overflow bins
//	double ratio_0 = 10.0;
//	double ratio_1 = 1.0 / 10.0;
	bool T = true;
	while (1) {
		if (gridX_L == 0 && gridX_R == gridNumX - 1 && gridY_B == 0 && gridY_T
				== gridNumY - 1) {
			//cout<<"[DEBUG]: break at point 1"<<endl;
			break;
		}
		extend[1] = false;
		extend[0] = false;
		//left boundary
		--gridX_L;
		if (gridX_L >= 0) {
			for (long ll = gridY_B; ll <= gridY_T; ++ll) {
				if (bins[gridX_L][ll]->isClustered()) {
					extend[1] = false;
					break;
				}
				if (bins[gridX_L][ll]->getDensity() <= targetTol) {
					continue;
				}
				extend[1] = true;
			}
			if (extend[1]) {
				extend_x++;
				for (long ll = gridY_B; ll <= gridY_T; ++ll) {
					assert(bins[gridX_L][ll]->isClustered() == false);
					areaCell += bins[gridX_L][ll]->cellArea;
					areaAvailable += bins[gridX_L][ll]->availableArea;
					bins[gridX_L][ll]->setClustered(true);
					area += bins[gridX_L][ll]->getArea();
					assert(area >= areaAvailable - 10e-6);
				}
			} else {
				++gridX_L;
			}
		} else {
			extend[1] = false;
			++gridX_L;
		}
		//DEBUG
		if ((areaCell / areaAvailable) <= targetTol){
//				|| ((bins[gridX_R][gridY_B]->right
//						- bins[gridX_L][gridY_B]->left)
//						/ (bins[gridX_L][gridY_T]->top
//								- bins[gridX_L][gridY_B]->bottom)) > ratio_0
//				|| (1.0 * (bins[gridX_L][gridY_T]->top
//						- bins[gridX_L][gridY_B]->bottom)
//						/ (bins[gridX_R][gridY_B]->right
//								- bins[gridX_L][gridY_B]->left)) < ratio_1) {
			T = false;
			break;
		}
		//right boundary
		++gridX_R;
		extend[0] = extend[0] || extend[1];
		extend[1] = false;
		if (gridX_R < gridNumX) {
			for (long rr = gridY_T; rr >= gridY_B; --rr) {
				if (bins[gridX_R][rr]->isClustered()) {
					extend[1] = false;
					break;
				}
				if (bins[gridX_R][rr]->getDensity() <= targetTol) {
					continue;
				}
				extend[1] = true;
			}
			if (extend[1]) {
				extend_x++;
				for (long rr = gridY_T; rr >= gridY_B; --rr) {
					assert(bins[gridX_R][rr]->isClustered() == false);

					areaCell += bins[gridX_R][rr]->cellArea;
					areaAvailable += bins[gridX_R][rr]->availableArea;
					bins[gridX_R][rr]->setClustered(true);
					area += bins[gridX_R][rr]->getArea();
					assert(area >= areaAvailable - 10e-6);
				}
			} else {
				--gridX_R;
			}
		} else {
			extend[1] = false;
			--gridX_R;
		}
		//DEBUG
		if ((areaCell / areaAvailable) <= targetTol){
//				|| ((bins[gridX_R][gridY_B]->right
//						- bins[gridX_L][gridY_B]->left)
//						/ (bins[gridX_L][gridY_T]->top
//								- bins[gridX_L][gridY_B]->bottom)) > ratio_0
//				|| (1.0 * (bins[gridX_L][gridY_T]->top
//						- bins[gridX_L][gridY_B]->bottom)
//						/ (bins[gridX_R][gridY_B]->right
//								- bins[gridX_L][gridY_B]->left)) < ratio_1) {
			T = false;
			break;
		}
		//top boundary
		++gridY_T;
		extend[0] = extend[0] || extend[1];
		extend[1] = false;
		if (gridY_T < gridNumY) {
			for (long tt = gridX_L; tt <= gridX_R; ++tt) {
				if (bins[tt][gridY_T]->isClustered()) {
					extend[1] = false;
					break;
				}
				if (bins[tt][gridY_T]->getDensity() <= targetTol) {
					continue;
				}
				extend[1] = true;
			}
			if (extend[1]) {
				extend_y++;
				for (long tt = gridX_L; tt <= gridX_R; ++tt) {
					assert(bins[tt][gridY_T]->isClustered() == false);

					areaCell += bins[tt][gridY_T]->cellArea;
					areaAvailable += bins[tt][gridY_T]->availableArea;
					bins[tt][gridY_T]->setClustered(true);
					area += bins[tt][gridY_T]->getArea();
					assert(area >= areaAvailable - 10e-6);
				}
			} else {
				--gridY_T;
			}
		} else {
			extend[1] = false;
			--gridY_T;
		}
		//DEBUG
		if ((areaCell / areaAvailable) <= targetTol){
//				|| ((bins[gridX_R][gridY_B]->right
//						- bins[gridX_L][gridY_B]->left)
//						/ (bins[gridX_L][gridY_T]->top
//								- bins[gridX_L][gridY_B]->bottom)) > ratio_0
//				|| (1.0 * (bins[gridX_L][gridY_T]->top
//						- bins[gridX_L][gridY_B]->bottom)
//						/ (bins[gridX_R][gridY_B]->right
//								- bins[gridX_L][gridY_B]->left)) < ratio_1) {
			T = false;
			break;
		}
		//bottom boundary
		--gridY_B;
		extend[0] = extend[0] || extend[1];
		extend[1] = false;
		if (gridY_B >= 0) {
			for (long bb = gridX_R; bb >= gridX_L; --bb) {
				if (bins[bb][gridY_B]->isClustered()) {
					extend[1] = false;
					break;
				}
				if (bins[bb][gridY_B]->getDensity() <= targetTol) {
					continue;
				}
				extend[1] = true;
			}
			if (extend[1]) {
				extend_y++;
				for (long bb = gridX_R; bb >= gridX_L; --bb) {
//					cout << bb << ' ' << gridY_B << endl;
//					cout << bins[bb][gridY_B]->left << ' ' << bins[bb][gridY_B]->bottom << endl;
//					cout << bins[bb][gridY_B]->getDensity() << endl;
					assert(bins[bb][gridY_B]->isClustered() == false);
					areaCell += bins[bb][gridY_B]->cellArea;
					areaAvailable += bins[bb][gridY_B]->availableArea;
					bins[bb][gridY_B]->setClustered(true);
					area += bins[bb][gridY_B]->getArea();
					assert(area >= areaAvailable - 10e-6);
				}
//				cin >> TEST;
			} else {
				++gridY_B;
			}
		} else {
			extend[1] = false;
			++gridY_B;
		}
		//DEBUG
		if ((areaCell / areaAvailable) <= targetTol){
//				|| ((bins[gridX_R][gridY_B]->right
//						- bins[gridX_L][gridY_B]->left)
//						/ (bins[gridX_L][gridY_T]->top
//								- bins[gridX_L][gridY_B]->bottom)) > ratio_0
//				|| (1.0 * (bins[gridX_L][gridY_T]->top
//						- bins[gridX_L][gridY_B]->bottom)
//						/ (bins[gridX_R][gridY_B]->right
//								- bins[gridX_L][gridY_B]->left)) < ratio_1) {
			T = false;
			break;
		}

		extend[0] = extend[0] || extend[1];
		if (!extend[0]) {
			break;
		}
	}

	extend_x = 0;
	extend_y = 0;
	//DEBUG
	int F = 1;
	if (T == false) {
		F = 0;
	}

//	if (step >= 1) {
//		cout << "step1 " << step << endl;
//		guiRect("cluster.gnu", overfilledBins, bins[gridX_L][gridY_B]->left,
//				bins[gridX_R][gridY_B]->right, bins[gridX_L][gridY_B]->bottom,
//				bins[gridX_L][gridY_T]->top);
//	}

	//get a minimum rectangular region with utilization less than threshold
	while (F) {
		if (gridX_L == 0 && gridX_R == gridNumX - 1 && gridY_B == 0 && gridY_T
				== gridNumY - 1) {
			//cout<<"[DEBUG]: the whole region is clustered!!!"<<endl;
			break;
		}
//		//DEBUG
//		if (((bins[gridX_R][gridY_B]->right
//						- bins[gridX_L][gridY_B]->left)
//						/ (bins[gridX_L][gridY_T]->top
//								- bins[gridX_L][gridY_B]->bottom)) > 10
//				|| (1.0 * (bins[gridX_L][gridY_T]->top
//						- bins[gridX_L][gridY_B]->bottom)
//						/ (bins[gridX_R][gridY_B]->right
//								- bins[gridX_L][gridY_B]->left)) < 0.1) {
//			//cout<<"[DEBUG]: here1 below targetTol "<<areaCell<<"/"<<areaAvailable<<" = "<<areaCell / areaAvailable<<endl;
//			break;
//		}

		extend[1] = true;
		extend[0] = false;
		//left boundary
		--gridX_L;
		if (gridX_L >= 0) {
			for (long ll = gridY_B; ll <= gridY_T; ++ll) {
				if (bins[gridX_L][ll]->isClustered()) {
					extend[1] = false;
					//cout<<"left is blocked"<<endl;
					break;
				}
			}
			if (extend[1]) {
				extend_x++;
				for (long ll = gridY_B; ll <= gridY_T; ++ll) {
					assert(bins[gridX_L][ll]->isClustered() == false);
					areaCell += bins[gridX_L][ll]->cellArea;
					areaAvailable += bins[gridX_L][ll]->availableArea;
					bins[gridX_L][ll]->setClustered(true);
					area += bins[gridX_L][ll]->getArea();
					assert(area >= areaAvailable - 10e-6);
				}
			} else {
				++gridX_L;
			}
		} else {
			extend[1] = false;
			++gridX_L;
		}
		//cout<<"left boundary "<<extend[1]<<endl;
		if (areaCell / areaAvailable <= targetTol){
//				|| ((bins[gridX_R][gridY_B]->right
//						- bins[gridX_L][gridY_B]->left)
//						/ (bins[gridX_L][gridY_T]->top
//								- bins[gridX_L][gridY_B]->bottom)) > ratio_0
//				|| (1.0 * (bins[gridX_L][gridY_T]->top
//						- bins[gridX_L][gridY_B]->bottom)
//						/ (bins[gridX_R][gridY_B]->right
//								- bins[gridX_L][gridY_B]->left)) < ratio_1) {
			//cout<<"[DEBUG]: here1 below targetTol "<<areaCell<<"/"<<areaAvailable<<" = "<<areaCell / areaAvailable<<endl;
			break;
		}

		//right boundary
		++gridX_R;
		extend[0] = extend[0] || extend[1];
		extend[1] = true;
		if (gridX_R < gridNumX) {
			for (long rr = gridY_T; rr >= gridY_B; --rr) {
				if (bins[gridX_R][rr]->isClustered()) {
					extend[1] = false;
					//cout<<"right is blocked"<<endl;
					break;
				}
			}
			if (extend[1]) {
				extend_x++;
				for (long rr = gridY_T; rr >= gridY_B; --rr) {
					assert(bins[gridX_R][rr]->isClustered() == false);
					areaCell += bins[gridX_R][rr]->cellArea;
					areaAvailable += bins[gridX_R][rr]->availableArea;
					bins[gridX_R][rr]->setClustered(true);
					area += bins[gridX_R][rr]->getArea();
					assert(area >= areaAvailable - 10e-6);
				}

			} else {
				--gridX_R;
			}
		} else {
			extend[1] = false;
			--gridX_R;
		}
		if (areaCell / areaAvailable <= targetTol){
//				|| ((bins[gridX_R][gridY_B]->right
//						- bins[gridX_L][gridY_B]->left)
//						/ (bins[gridX_L][gridY_T]->top
//								- bins[gridX_L][gridY_B]->bottom)) > ratio_0
//				|| (1.0 * (bins[gridX_L][gridY_T]->top
//						- bins[gridX_L][gridY_B]->bottom)
//						/ (bins[gridX_R][gridY_B]->right
//								- bins[gridX_L][gridY_B]->left)) < ratio_1) {
			//cout<<"[DEBUG]: here2 below targetTol "<<areaCell<<"/"<<areaAvailable<<" = "<<areaCell / areaAvailable<<endl;
			break;
		}

		if (extend_x < h && extend[0]) {
			continue;
		} else {
			extend_x = 0;
		}

		//top boundary
		++gridY_T;
		extend[0] = extend[0] || extend[1];
		extend[1] = true;
		if (gridY_T < gridNumY) {
			for (long tt = gridX_L; tt <= gridX_R; ++tt) {
				if (bins[tt][gridY_T]->isClustered()) {
					extend[1] = false;
					//cout<<"top is blocked"<<endl;
					break;
				}
			}
			if (extend[1]) {
				extend_y++;
				for (long tt = gridX_L; tt <= gridX_R; ++tt) {
					assert(bins[tt][gridY_T]->isClustered() == false);
					areaCell += bins[tt][gridY_T]->cellArea;
					areaAvailable += bins[tt][gridY_T]->availableArea;
					bins[tt][gridY_T]->setClustered(true);
					area += bins[tt][gridY_T]->getArea();
					assert(area >= areaAvailable - 10e-6);
				}
			} else {
				--gridY_T;
			}
		} else {
			extend[1] = false;
			--gridY_T;
		}
		if (areaCell / areaAvailable <= targetTol){
//				|| ((bins[gridX_R][gridY_B]->right
//						- bins[gridX_L][gridY_B]->left)
//						/ (bins[gridX_L][gridY_T]->top
//								- bins[gridX_L][gridY_B]->bottom)) > ratio_0
//				|| (1.0 * (bins[gridX_L][gridY_T]->top
//						- bins[gridX_L][gridY_B]->bottom)
//						/ (bins[gridX_R][gridY_B]->right
//								- bins[gridX_L][gridY_B]->left)) < ratio_1) {
			//cout<<"[DEBUG]: here3 below targetTol "<<areaCell<<"/"<<areaAvailable<<" = "<<areaCell / areaAvailable<<endl;
			break;
		}
		//bottom boundary
		--gridY_B;
		extend[0] = extend[0] || extend[1];
		extend[1] = true;
		if (gridY_B >= 0) {
			for (long bb = gridX_R; bb >= gridX_L; --bb) {
				if (bins[bb][gridY_B]->isClustered()) {
					extend[1] = false;
					//cout<<"bottom is blocked"<<endl;
					break;
				}
			}
			if (extend[1]) {
				extend_y++;
				for (long bb = gridX_R; bb >= gridX_L; --bb) {
					assert(bins[bb][gridY_B]->isClustered() == false);
					areaCell += bins[bb][gridY_B]->cellArea;
					areaAvailable += bins[bb][gridY_B]->availableArea;
					bins[bb][gridY_B]->setClustered(true);
					area += bins[bb][gridY_B]->getArea();
					assert(area >= areaAvailable - 10e-6);
				}
			} else {
				++gridY_B;
			}
		} else {
			extend[1] = false;
			++gridY_B;
		}


		extend[0] = extend[0] || extend[1];

		if (areaCell / areaAvailable <= targetTol){
//				|| ((bins[gridX_R][gridY_B]->right
//						- bins[gridX_L][gridY_B]->left)
//						/ (bins[gridX_L][gridY_T]->top
//								- bins[gridX_L][gridY_B]->bottom)) > ratio_0
//				|| (1.0 * (bins[gridX_L][gridY_T]->top
//						- bins[gridX_L][gridY_B]->bottom)
//						/ (bins[gridX_R][gridY_B]->right
//								- bins[gridX_L][gridY_B]->left)) < ratio_1) {
			//cout<<"[DEBUG]: here4 below targetTol "<<areaCell<<"/"<<areaAvailable<<" = "<<areaCell / areaAvailable<<endl;
			break;
		}
		if (!extend[0]) {
			//cout<<"cant extend more, break"<<endl;
			//cout<<bins[gridX_L][gridY_B]->left<<" "<<bins[gridX_R][gridY_B]->right<<" "<<
			//bins[gridX_L][gridY_B]->bottom<<" "<<bins[gridX_L][gridY_T]->top<<endl;
			break;
		}
		//cout<<areaCell / areaAvailable<<endl;
	}

	cluster->left = bins[gridX_L][gridY_B]->left;
	cluster->right = bins[gridX_R][gridY_B]->right;
	cluster->bottom = bins[gridX_L][gridY_B]->bottom;
	cluster->top = bins[gridX_L][gridY_T]->top;
	cluster->gridLeft = gridX_L;
	cluster->gridRight = gridX_R;
	cluster->gridBottom = gridY_B;
	cluster->gridTop = gridY_T;
	cluster->cellArea = areaCell;
	cluster->availableArea = areaAvailable;

	//DEBUG
//	if (step >= 1) {
//		cout << "step2 " << step << endl;
//		guiRect("boundaryBox.gnu", overfilledBins, bins[gridX_L][gridY_B]->left,
//				bins[gridX_R][gridY_B]->right, bins[gridX_L][gridY_B]->bottom,
//				bins[gridX_L][gridY_T]->top);
//	}

	assert(abs(cluster->getArea() - area) < 10e-6);
	//cout << "here " << targetTol << " " << tmpTargetTol << endl;
	targetTol = tmpTargetTol;

	//assert(cluster->availableArea <= cluster->getArea() + 10e-6);
	return cluster;
}

void SimPlPlace::getCellsInCluster(RLRegion* cluster) {
	long left = cluster->left;
	long right = cluster->right;
	long bottom = cluster->bottom;
	long top = cluster->top;
	long TEST = 0;

//	for (long i = 0; i < gridNumX; ++i){
//		for(long j = 0; j < gridNumY; ++j){
//			if(bins[i][j]->moveInstsX.size() > 0){
//				cout << i << ' ' << j << ' ' << endl;
//				cout << bins[i][j]->left << ' ' << bins[i][j]->right << ' ' << bins[i][j]->bottom << ' ' << bins[i][j]->top << endl;
//				for(long k = 0; k < bins[i][j]->moveInstsX.size(); ++k){
//					cout << bins[i][j]->moveInstsX[k]->getCenterX() << ' ' << bins[i][j]->moveInstsX[k]->getCenterY() << endl;
//				}
//				cout << "OK!" << endl;
//				cin >> TEST;
//			}
//		}
//	}

	for(long i = cluster->getGridLeft(); i <= cluster->getGridRight(); ++i){
		for(long j = cluster->getGridBottom(); j <= cluster->getGridTop(); ++j){
			if(bins[i][j]->fixInsts.size() > 0){
				for(long k = 0; k < bins[i][j]->fixInsts.size(); ++k){
					if(bins[i][j]->fixInsts[k]->isIncluded()){
						bins[i][j]->fixInsts[k]->initIncluded();
					}
				}
			}
		}
	}

	for(long i = cluster->getGridLeft(); i <= cluster->getGridRight(); ++i){
		for(long j = cluster->getGridBottom(); j <= cluster->getGridTop(); ++j){
			if(bins[i][j]->moveInstsX.size() > 0){
				for(long k = 0; k < bins[i][j]->moveInstsX.size(); ++k){
					if(bins[i][j]->moveInstsX[k]->getCenterX() > left
							&& bins[i][j]->moveInstsX[k]->getCenterX() < right
							&& bins[i][j]->moveInstsX[k]->getCenterY() > bottom
							&& bins[i][j]->moveInstsX[k]->getCenterY() < top){
						cluster->moveInstsX.push_back(bins[i][j]->moveInstsX[k]);
						cluster->moveInstsY.push_back(bins[i][j]->moveInstsX[k]);
					}else{
						cout << bins[i][j]->moveInstsX[k]->getCenterX() << ' ' << left << ' ' << bins[i][j]->moveInstsX[k]->getCenterX() << ' ' << right << ' ' << bins[i][j]->moveInstsX[k]->getCenterY() << ' ' << bottom << ' ' << bins[i][j]->moveInstsX[k]->getCenterY() << ' ' << top << endl;
						cin >> TEST;
					}

//					cout << bins[i][j]->moveInstsX[k]->getCenterX() << ' ' << bins[i][j]->moveInstsX[k]->getCenterY() << endl;
				}
			}
			if(bins[i][j]->fixInsts.size() > 0){
				for(long k = 0; k < bins[i][j]->fixInsts.size(); ++k){
					if(!bins[i][j]->fixInsts[k]->isIncluded()
							&& bins[i][j]->fixInsts[k]->getCoordX() < right
							&& bins[i][j]->fixInsts[k]->getCoordX() + bins[i][j]->fixInsts[k]->getWidth() > left
							&& bins[i][j]->fixInsts[k]->getCoordY() < top
							&& bins[i][j]->fixInsts[k]->getCoordY() + bins[i][j]->fixInsts[k]->getHeight() > bottom){
						cluster->fixInsts.push_back(bins[i][j]->fixInsts[k]);
						bins[i][j]->fixInsts[k]->setIncluded();
					}
				}
			}
		}
	}



//	for (long i = 0; i < numValidNodes; ++i) {
//		if (validNodes[i]->getStatus() == Moved) {
//			if (validNodes[i]->getCenterX() < right
//					&& validNodes[i]->getCenterX() > left
//					&& validNodes[i]->getCenterY() < top
//					&& validNodes[i]->getCenterY() > bottom) {
//				cluster->moveInstsX.push_back(validNodes[i]);
//				cluster->moveInstsY.push_back(validNodes[i]);
//			}
//		} else if (validNodes[i]->getStatus() == Fixed) {
//			if (validNodes[i]->getCoordX() < right
//					&& validNodes[i]->getCoordX() + validNodes[i]->getWidth()
//							> left && validNodes[i]->getCoordY() < top
//					&& validNodes[i]->getCoordY() + validNodes[i]->getHeight()
//							> bottom) {
//				cluster->fixInsts.push_back(validNodes[i]);
//			}
//		} else {
//			//cout<<"[WARNING]: neither Moved nor Fixed!!!  ";
//			//cout<<validNodes[i]->getName()<<" "<<validNodes[i]->getStatus()<<endl;
//		}
//	}

	sort(cluster->moveInstsX.begin(), cluster->moveInstsX.end(), lessCenterX);
	sort(cluster->moveInstsY.begin(), cluster->moveInstsY.end(), lessCenterY);

	//DEBUG
//	sort(cluster->fixInsts.begin(), cluster->fixInsts.end(), lessCenterX);
//	ofstream move("moveInstsX",ios_base::out | ios_base::app);
//	ofstream fix("fixInsts",ios_base::out | ios_base::app);
//	for(long i = 0; i < cluster->moveInstsX.size(); ++i){
//		move << cluster->moveInstsX[i]->getCoordX() << endl;
//	}
//	for(long i = 0; i < cluster->fixInsts.size(); ++i){
//		fix << cluster->fixInsts[i]->getCoordX() << endl;
//	}
//	move << "\n";
//	fix << "\n";


	vector<Inst*>& instsX = cluster->moveInstsX;
	vector<Inst*>& instsY = cluster->moveInstsY;
	assert(instsX.size() == instsY.size());
//	cout << "OK" << endl;
	//for debug
	//checkOrder(cluster->moveInstsX, true);
	//checkOrder(cluster->moveInstsY, false);

}

void SimPlPlace::diffusion(RLRegion* rect, int level) {
	//for debug
	//cout<<"[DEBUG]: diffusion!!! level "<<level<<endl;
	assert(rect->availableArea <= rect->getArea());

	//checkOrder(rect->moveInstsX, true);
	//checkOrder(rect->moveInstsY, false);

	//DEBUG
	assert(rect->right - 1> rect->left);
	assert(rect->top - 1 > rect->bottom);

	double ratio_0 = 10.0;
	double ratio_1 = 1.0 / 10.0;

	if (level < 0) {
		delete rect;
		return;
	} else if (rect->getAvailableArea() < 5 * averageNodeArea) {
		delete rect;
		return;
	}


	else if (rect->getWidth() / rect->getHeight() > ratio_0 || (1.0
			* rect->getWidth() / rect->getHeight()) < ratio_1) {
		//cout<<"diffusion region is too narrow. level "<<level<<". "<<rect->getWidth()<<" "<<rect->getHeight()<<endl;
		delete rect;
		return;
	}

	vector<Inst*> targetCells;
	targetCells.clear();
	vector<Inst*> obstacles;
	obstacles.clear();
	obstacles = rect->fixInsts;
	if (rect->moveInstsX.size() == 0) {
		delete rect;
		return;
	}
	long sizeObstacles = obstacles.size();
	//cout<<"[DEBUG]: obstacle number = "<<sizeObstacles<<endl;

	// divide the region, using borders of fixed blocks
	vector<long> borders;
	borders.clear();
	if (level % 2 == 0) { //x-direction
		//for debug
		//cout<<rect->left<<" "<<rect->moveInstsX.size()<<" "<<rect->moveInstsY.size()<<" "<<rect->fixInsts.size()<<endl;
		//cout<<rect->moveInstsX[0]->getName()<<" "<<rect->moveInstsY[0]->getName()<<endl;

		targetCells = rect->moveInstsX;

		//reOrder(targetCells, true);

		borders.push_back(rect->left);
		borders.push_back(rect->right);
		for (long i = 0; i < sizeObstacles; i++) {
			if (obstacles[i]->getCoordX() > rect->left) {
				borders.push_back((long) obstacles[i]->getCoordX());
			}
			if (obstacles[i]->getCoordX() + obstacles[i]->getWidth()
					< rect->right) {
				borders.push_back(
						(long) obstacles[i]->getCoordX()
								+ obstacles[i]->getWidth());
			}
		}
		sort(borders.begin(), borders.end());
		borders.erase(unique(borders.begin(), borders.end()), borders.end());
	} else { //y-direction
		//for debug
		//cout<<rect->left<<" "<<rect->moveInstsX.size()<<" "<<rect->moveInstsY.size()<<" "<<rect->fixInsts.size()<<endl;
		//cout<<rect->moveInstsX[0]->getName()<<" "<<rect->moveInstsY[0]->getName()<<endl;

		targetCells = rect->moveInstsY;

		//reOrder(targetCells, false);

		borders.push_back(rect->bottom);
		borders.push_back(rect->top);
		for (long i = 0; i < sizeObstacles; i++) {
			if (obstacles[i]->getCoordY() > rect->bottom) {
				borders.push_back((long) obstacles[i]->getCoordY());
			}
			if (obstacles[i]->getCoordY() + obstacles[i]->getHeight()
					< rect->top) {
				borders.push_back(
						(long) obstacles[i]->getCoordY()
								+ obstacles[i]->getHeight());
			}
		}
		sort(borders.begin(), borders.end());
		borders.erase(unique(borders.begin(), borders.end()), borders.end());
	}

	int numStrips;
	numStrips = borders.size() - 1;
	assert(numStrips >= 1);

	double totalWhiteSpace = 0;
	//cout<<"[DEBUG]: totalWhiteSpace "<<totalWhiteSpace<<"/"<<rect->getArea()<<", "<<rect->left<<" "<<rect->right<<" "<<rect->bottom<<" "<<rect->top<<endl;

	vector<double> whiteSpace;
	whiteSpace.clear();
	whiteSpace.resize(numStrips, 0.0);

	for (int i = 0; i < targetCells.size(); ++i) {
		targetCells[i]->moved = false;
	}

	if (level % 2 == 0) {
		getStripsUsage(borders, obstacles, whiteSpace, rect->bottom, rect->top,
				level, totalWhiteSpace);
	} else {
		getStripsUsage(borders, obstacles, whiteSpace, rect->left, rect->right,
				level, totalWhiteSpace);
	}

	if (totalWhiteSpace < 5 * averageNodeArea) {
		delete rect;
		return;
	}
	//DEBUG
	else if (totalWhiteSpace < 0.1 * rect->getArea()) {
		delete rect;
		return;
	}

	rect->availableArea = totalWhiteSpace;

	double a = 0;
	//for debug
	//	for (long i = 0; i < whiteSpace.size(); ++i) {
	//		a += whiteSpace[i];
	//	}
	//	if ((long) a != (long) rect->availableArea) {
	//		cout << level << " " << a << " / " << rect->availableArea << endl;
	//	}

	long Cb = -1;
	double halfWS = 0;
	double WS1 = 0, WS2 = 0, rate = 0;
	double coordCb = 0;
	//find cutline Cb
	for (long i = 0; i < (long) whiteSpace.size(); ++i) {
		if ((halfWS + whiteSpace[i]) > totalWhiteSpace / 2) {
			WS1 = totalWhiteSpace / 2 - halfWS;
			WS2 = halfWS + whiteSpace[i] - totalWhiteSpace / 2;
			rate = WS1 / whiteSpace[i];
			coordCb = (borders[i] + rate * (borders[i + 1] - borders[i]));

			//DEBUG
			if (coordCb - borders[i] < 2){
//					&&(i!=0)) {
				//cout<<"[DEBUG]: Cb is near left"<<endl;
				Cb = i;
			} else if ((borders[i + 1] - coordCb) < 2 ){
//					&&((i+1) != whiteSpace.size())) {
				//cout<<"[DEBUG]: Cb is near right"<<endl;
				Cb = i + 1;
				halfWS += whiteSpace[i];
			} else {
				//cout<<"[DEBUG]: Cb is accurate"<<endl;
				assert(coordCb != borders[i]);
				assert(coordCb != borders[i+1]);
				whiteSpace[i] = WS1;
				whiteSpace.insert(whiteSpace.begin() + i + 1, WS2);
				borders.insert(borders.begin() + i + 1, (long) coordCb);
				Cb = i + 1;
				halfWS += whiteSpace[i];
			}
			break;
		}
		halfWS += whiteSpace[i];
	}
	//cout<<halfWS<<" "<<totalWhiteSpace<<" "<<halfWS / totalWhiteSpace<<endl;
	//for debug
	if (abs(halfWS / totalWhiteSpace - 0.5) > 0.01) {
		//cout<<"[WARNING]: halfWS of left part = "<<halfWS<<", "<<halfWS / totalWhiteSpace<<", "<<Cb<<"/"<<whiteSpace.size()<<endl;
		//cout<<level<<"---"<<rect->left<<" "<<rect->right<<" "<<rect->bottom<<" "<<rect->top<<endl;
	}
	//assert(abs(halfWS / totalWhiteSpace - 0.5) <= 0.01);
	if (Cb == -1) {
		cout << "Cb == -1!!!" << endl;
		cout << "level " << level << endl;
		cout << rect->left << " " << rect->right << " " << rect->bottom << " "
				<< rect->top << endl;
		cout << halfWS << " / " << totalWhiteSpace << endl;
		cout << "[ERROR]: Cb not found" << endl;
	}
	if (Cb == 0) {
		cout << "Cb == 0!!!" << endl;
		cout << "level " << level << endl;
		cout << rect->left << " " << rect->right << " " << rect->bottom << " "
				<< rect->top << endl;
		cout << halfWS << " / " << totalWhiteSpace << endl;
		cout << rect->getArea() << " " << averageNodeArea << endl;
		cout << "whitespace size = " << whiteSpace.size() << endl;

		for (long i = 0; i < (long) whiteSpace.size(); ++i) {
			cout << i << " " << whiteSpace[i] << endl;
		}
	}
	numStrips = whiteSpace.size();

	if(Cb == 0 || Cb == -1){
		delete rect;
		return;
	}

	cellShifting(borders, targetCells, whiteSpace, level, totalWhiteSpace, Cb);

	//for debug
	//	for (int i = 0; i < targetCells.size(); ++i){
	//		cout<<level<<" "<<targetCells[i]->getCenterX()<<" "<<targetCells[i]->getCenterY()<<endl;
	//	}
	//	long x;
	//	cin >> x;


	//for debug
	//if (level == 1){
	//	guiRect("region.gnu", rect->left, rect->right, rect->bottom, rect->top);
	//	cout<<"pause, level "<<level<<endl;
	//	long x;
	//	cin>>x;
	//}


	//for debug
	//	if (level % 2 == 0) {
	//		checkOrder(rect->moveInstsX, true);
	//	} else {
	//		checkOrder(rect->moveInstsY, false);
	//	}

	//init data in 2 subregions
	RectLevel rl1;
	RectLevel rl2;
	rl1.level = level - 1;
	rl2.level = level - 1;

	if (level % 2 == 0) {
		assert(Cb < numStrips);
		assert(Cb > 0);
		assert(borders[Cb] > borders[0]);
		assert(borders[numStrips] > borders[Cb]);

		rl1.region = new RLRegion(borders[0], borders[Cb], rect->bottom,
				rect->top); //left
		rl2.region = new RLRegion(borders[Cb], borders[numStrips],
				rect->bottom, rect->top); //right

		for (long i = 0; i < (long) rect->fixInsts.size(); ++i) {
			if (rect->fixInsts[i]->getCoordX() < borders[Cb]) {
				rl1.region->fixInsts.push_back(rect->fixInsts[i]);
			}
			if (rect->fixInsts[i]->getCoordX() + rect->fixInsts[i]->getWidth()
					> borders[Cb]) {
				rl2.region->fixInsts.push_back(rect->fixInsts[i]);
			}
		}

		for (long i = 0; i < (long) rect->moveInstsX.size(); ++i) {
			if (rect->moveInstsX[i]->getCenterX() <= borders[Cb]) {
				rl1.region->moveInstsX.push_back(rect->moveInstsX[i]);
				rl1.region->cellArea += rect->moveInstsX[i]->getArea();
			} else {
				rl2.region->moveInstsX.push_back(rect->moveInstsX[i]);
				rl2.region->cellArea += rect->moveInstsX[i]->getArea();
			}
		}
		for (long i = 0; i < (long) rect->moveInstsY.size(); ++i) {
			if (rect->moveInstsY[i]->getCenterX() <= borders[Cb]) {
				rl1.region->moveInstsY.push_back(rect->moveInstsY[i]);
				rl1.region->cellArea += rect->moveInstsY[i]->getArea();
			} else {
				rl2.region->moveInstsY.push_back(rect->moveInstsY[i]);
				rl2.region->cellArea += rect->moveInstsY[i]->getArea();
			}
		}
		assert(rl1.region->moveInstsX.size() == rl1.region->moveInstsY.size());
		assert(rl2.region->moveInstsX.size() == rl2.region->moveInstsY.size());
	} else {
		//for debug
		/*if (Cb <= 0){
		 cout<<"[ERROR]: Cb = "<<Cb<<endl;
		 cout<<halfWS<<" "<<totalWhiteSpace<<endl;
		 cout<<"whiteSpace size = "<<whiteSpace.size()<<endl;
		 for (long q = 0; q < whiteSpace.size(); ++q){
		 cout<<whiteSpace[q]<<endl;
		 }
		 cout<<endl;
		 for (long p = 0; p < borders.size(); ++p){
		 cout<<borders[p]<<endl;
		 }
		 }*/assert(Cb > 0);
		assert(Cb < numStrips);
		assert(borders[Cb] > borders[0]);
		assert(borders[numStrips] > borders[Cb]);
		rl1.region = new RLRegion(rect->left, rect->right, borders[0],
				borders[Cb]); //bottom
		rl2.region = new RLRegion(rect->left, rect->right, borders[Cb],
				borders[numStrips]); //top

		for (long i = 0; i < (long) rect->fixInsts.size(); ++i) {
			if (rect->fixInsts[i]->getCoordY() < borders[Cb]) {
				rl1.region->fixInsts.push_back(rect->fixInsts[i]);
			}
			if (rect->fixInsts[i]->getCoordY() + rect->fixInsts[i]->getHeight()
					> borders[Cb]) {
				rl2.region->fixInsts.push_back(rect->fixInsts[i]);
			}
		}

		for (long i = 0; i < (long) rect->moveInstsX.size(); ++i) {
			if (rect->moveInstsX[i]->getCenterY() <= borders[Cb]) {
				rl1.region->moveInstsX.push_back(rect->moveInstsX[i]);
				rl1.region->cellArea += rect->moveInstsX[i]->getArea();
			} else {
				rl2.region->moveInstsX.push_back(rect->moveInstsX[i]);
				rl2.region->cellArea += rect->moveInstsX[i]->getArea();
			}
		}
		for (long i = 0; i < (long) rect->moveInstsY.size(); ++i) {
			if (rect->moveInstsY[i]->getCenterY() <= borders[Cb]) {
				rl1.region->moveInstsY.push_back(rect->moveInstsY[i]);
				rl1.region->cellArea += rect->moveInstsY[i]->getArea();
			} else {
				rl2.region->moveInstsY.push_back(rect->moveInstsY[i]);
				rl2.region->cellArea += rect->moveInstsY[i]->getArea();
			}
		}
		assert(rl1.region->moveInstsX.size() == rl1.region->moveInstsY.size());
		assert(rl2.region->moveInstsX.size() == rl2.region->moveInstsY.size());

	}
	//for debug
	//	for (int i = 0; i < rl1.region->moveInstsX.size(); ++i) {
	//		cout << rl1.level << " " << rl1.region->moveInstsX[i]->getCenterY()<<" "<<rl1.region->moveInstsY[i]->getCenterY()
	//				<< endl;
	//		cout << rl1.level << " " << rl1.region->moveInstsY[i]->getCenterX()<<" "<<rl1.region->moveInstsY[i]->getCenterY()
	//				<< endl;
	//	}
	//	long x;
	//	cin >> x;


	//for debug
	/*a = 0;
	 double b = 0;
	 double c = 0;
	 for (long i = 0; i < Cb; ++i){
	 a += whiteSpace[i];
	 }
	 if ((long)a != (long)halfWS){
	 cout<<"[ERROR1]: "<<a<<" / "<<halfWS<<endl;
	 }

	 for (long i = Cb; i < whiteSpace.size(); ++i){
	 b += whiteSpace[i];
	 }
	 if ((long)b != (long)(rect->availableArea - halfWS)){
	 cout<<"[ERROR2]: "<<b<<" / "<<rect->availableArea - halfWS<<endl;
	 }

	 for (long i = 0; i < whiteSpace.size(); ++i){
	 c += whiteSpace[i];
	 }
	 if ((long)c != (long)(rect->availableArea)){
	 cout<<"[ERROR3]: "<<c<<" / "<<rect->availableArea<<endl;
	 }


	 rl1.region->availableArea = halfWS;
	 rl2.region->availableArea = rect->availableArea - halfWS;

	 if (rl2.region->availableArea > rl2.region->getArea()){
	 cout<<rect->availableArea<<" "<<rect->getArea()<<endl;
	 cout<<rl1.region->availableArea<<" "<<rl1.region->getArea()<<endl;
	 cout<<rl2.region->availableArea<<" "<<rl2.region->getArea()<<endl;
	 cout<<rl2.region->left<<" "<<rl2.region->right<<" "<<rl2.region->bottom<<" "<<rl2.region->top<<endl;
	 }
	 assert(rl2.region->availableArea <= rl2.region->getArea());*/

	//for debug
	//checkOrder(rl1.region->moveInstsX, true);
	//checkOrder(rl1.region->moveInstsY, false);
	//checkOrder(rl2.region->moveInstsX, true);
	//checkOrder(rl2.region->moveInstsY, false);

	subRegions.push(rl1);
	subRegions.push(rl2);
	delete rect;
	return;
}

void SimPlPlace::getStripsUsage(vector<long>& borders,
		vector<Inst*>& obstacles, vector<double>& whiteSpace, long boundary1,
		long boundary2, int level, double& totalWhiteSpace) {
	//initialize
	long numStrips = (long) whiteSpace.size();
	for (long i = 0; i < numStrips; ++i) {
		whiteSpace[i] = 1.0 * (boundary2 - boundary1) * (borders[i + 1]
				- borders[i]);
	}

	// calculate fixed cells area
	if (level % 2 == 0) { //x-direction
		long xStart = 0;
		long xEnd = 0;
		for (long i = 0; i < (long) obstacles.size(); ++i) {
			long xl = obstacles[i] -> getCoordX();
			long xh = xl + obstacles[i] -> getWidth();
			long yl = obstacles[i] ->getCoordY();
			long yh = yl + obstacles[i] -> getHeight();
			if (xl < borders[0]) {
				xStart = 0;
			}
			for (long j = 0; j < (long) borders.size(); ++j) {
				if (xl == borders[j]) {
					xStart = j;
				}
				if (xh == borders[j]) {
					xEnd = j;
					break;
				}
			}
			if (xh > borders.back()) {
				xEnd = borders.size() - 1;
			}
			// whiteSpace of stripe
			long down = yl > boundary1 ? yl : boundary1;
			long up = yh < boundary2 ? yh : boundary2;
			for (long j = xStart; j < xEnd; j++) {
				long left = xl > borders[j] ? xl : borders[j];
				long right = xh < borders[j + 1] ? xh : borders[j + 1];

				assert(xh > xl);
				assert(borders[j+1] > borders[j]);
				assert(right >= left);
				assert(up >= down);

				whiteSpace[j] -= 1.0 * (right - left) * (up - down);
				if (whiteSpace[j] < 0) {
					whiteSpace[j] = 0;
				}
				assert(whiteSpace[j] >= 0);
			}
		}
	}

	else { //y-direction
		long yStart = 0;
		long yEnd = 0;
		for (long i = 0; i < (long) obstacles.size(); ++i) {
			long xl = obstacles[i]->getCoordX();
			long xh = xl + obstacles[i]->getWidth();
			long yl = obstacles[i]->getCoordY();
			long yh = yl + obstacles[i] -> getHeight();

			if (yl < borders[0]) {
				yStart = 0;
			}
			for (long j = 0; j < (long) borders.size(); ++j) {
				if (yl == borders[j]) {
					yStart = j;
				}
				if (yh == borders[j]) {
					yEnd = j;
					break;
				}
			}
			if (yh > borders.back()) {
				yEnd = borders.size() - 1;
			}

			// whiteSpace of stripe
			long left = xl > boundary1 ? xl : boundary1;
			long right = xh < boundary2 ? xh : boundary2;
			for (long j = yStart; j < yEnd; j++) {
				long down = yl > borders[j] ? yl : borders[j];
				long up = yh < borders[j + 1] ? yh : borders[j + 1];

				assert(right >= left);
				assert(up >= down);

				whiteSpace[j] -= 1.0 * (right - left) * (up - down);
				if (whiteSpace[j] < 0) {
					whiteSpace[j] = 0;
				}
				assert(whiteSpace[j] >= 0);
			}
		}
	}

	for (long i = 0; i < (long) whiteSpace.size(); ++i) {
		totalWhiteSpace += whiteSpace[i];
	}

	//DEBUG
	for (long i = 0; i < (long) whiteSpace.size(); ++i) {
		if (whiteSpace[i] > totalWhiteSpace / 10 && (borders[i + 1]
				- borders[i] > 3)) {
			double tempWS = whiteSpace[i] / 2;
			long tempB = (borders[i] + borders[i + 1]) / 2;

			assert(tempB != borders[i]);
			assert(tempB != borders[i+1]);
			borders.insert(borders.begin() + i + 1, (long) tempB);
			whiteSpace[i] = tempWS;
			whiteSpace.insert(whiteSpace.begin() + i + 1, tempWS);
			i--;
		}
	}
	//assert(whiteSpace.size() >= 10);
	assert(borders.size() == whiteSpace.size() + 1);
	return;
}

	void SimPlPlace::cellShifting(vector<long> borders, vector<Inst*> targetCells,
			vector<double> whiteSpace, int level, double totalWhiteSpace, double Cb) {
	//for debug
	//	for (long i = 0; i < targetCells.size(); ++i){
	//		cout<<level<<" in "<<targetCells[i]->getCenterX()<<" "<<targetCells[i]->getCenterY()<<endl;
	//	}
	//	long x;
	//	cin >> x;
	//cout<<"cell shifting "<<level<<endl;
	//checkOrder(targetCells, (level % 2 == 0));
	//cout<<"[INFO]: total cell number is :"<<targetCells.size()<<endl;
	// get stripes usage
	int numStrips = whiteSpace.size();
	long numCells = targetCells.size();

	double clusterMovedArea = 0;
	for (long i = 0; i < numCells; ++i) {
		clusterMovedArea += targetCells[i]->getArea();
	}
	double density = clusterMovedArea / totalWhiteSpace;
	//	cout<<"[DEBUG]: density = "<<density<<endl;
	//	while(1);


	//for debug
	double halfCellArea = 0;
	double halfWhiteSpace = 0;
	double tempCA = 0;
	double tempWS = 0;

	double areaAvailable = 0;
	long halfCellNo = 0;
	long currentCell = -1;
	long formerCell = 0;
	bool flag = false;
	bool flag1 = false;
	//long t; //for debug

	//LEFT PART
	for (long i = 0; i < Cb; i++) {
		if (whiteSpace[i] < 10e-6) {
			continue;
		}
		flag1 = false;
		areaAvailable += whiteSpace[i];
		//stripeCells = 0;
		while (areaAvailable > 0) {
			flag1 = true;
			currentCell++;
			//DEBUG
			areaAvailable -= targetCells[currentCell]->getArea() / density;//density is ga_ma
			halfCellArea += targetCells[currentCell]->getArea() / density;
			//			areaAvailable -= targetCells[currentCell]->getArea() / targetTol;
			//			halfCellArea += targetCells[currentCell]->getArea() / targetTol;
			//
			if (currentCell == numCells - 1) {
				flag = true;
				break;
			}
		}
		halfWhiteSpace += whiteSpace[i];

		assert(currentCell < numCells);
		if (!flag1) {
			continue;
		}
		//t = i+1;
		//cout<<"[DEBUG]: level---"<<level<<", cell "<<formerCell<<"-"<<currentCell<<" is placed between "<<borders[i]<<"-"<<borders[i+1]<<endl;

		linearScaling(targetCells, formerCell, currentCell, borders[i],
				borders[i + 1], level);
		formerCell = currentCell + 1;
		if (flag) {
			//cout<<"[WARNING] : all cells are placed in left part!!!"<<endl;
			break;
		}
	}
	//cout<<"[DEBUG]: "<<currentCell + 1<<" cells are placed in left part"<<endl;
	halfCellNo = currentCell; //at this point, currentCell has already been placed
	//cout<<"[DEBUG]: half cell area = "<<halfCellArea<<"/"<<clusterMovedArea<<" = "<<halfCellArea / clusterMovedArea<<endl;
	//cout<<"[DEBUG]: half white space = "<<halfWhiteSpace<<"/"<<totalWhiteSpace<<" = "<<halfWhiteSpace / totalWhiteSpace<<endl;

	//RIGHT PART
	areaAvailable = 0;
	currentCell = numCells;
	formerCell = currentCell - 1;
	flag = false;
	for (long i = numStrips - 1; i >= Cb; --i) {//?
		//tempCA = 0;
		//tempWS = 0;
		if (whiteSpace[i] < 10e-6) {
			continue;
		}
		flag1 = false;
		areaAvailable += whiteSpace[i];
		tempWS += whiteSpace[i];
		//cout<<"tempWS = "<<tempWS<<endl;
		while (areaAvailable > 0) {
			currentCell--;
			areaAvailable -= targetCells[currentCell]->getArea() / density;
			tempCA += targetCells[currentCell]->getArea() / density;
			flag1 = true;
			if (currentCell <= halfCellNo + 1) {
				break;
			}
		}
		//cout<<"tempCA = "<<tempCA<<endl;
		assert(currentCell >= 0);
		if (!flag1) {
			if (i == Cb && currentCell > halfCellNo + 1) {
				//cout<<"[DEBUG]: "<<i<<"/"<<Cb<<" condition3-level---"<<level<<", cell "<<halfCellNo + 1<<"-"<<formerCell<<" is placed between "<<borders[i]<<"-"<<borders[i+1]<<endl;
				//cout<<"temp density = "<<tempCA / tempWS<<endl;
				linearScaling(targetCells, halfCellNo + 1, formerCell,
						borders[i], borders[i + 1], level);
			}
			continue;
		}
		//t = i;
		if (i == Cb && currentCell > halfCellNo + 1) {
			//cout<<"[DEBUG]: "<<i<<"/"<<Cb<<" condition1-level---"<<level<<", cell "<<halfCellNo + 1<<"-"<<formerCell<<" is placed between "<<borders[i]<<"-"<<borders[i+1]<<endl;
			//cout<<"temp density = "<<tempCA / tempWS<<endl;
			linearScaling(targetCells, halfCellNo + 1, formerCell, borders[i],
					borders[i + 1], level);
		} else {
			//cout<<"[DEBUG]: "<<i<<"/"<<Cb<<" condition2-level---"<<level<<", cell "<<currentCell<<"-"<<formerCell<<" is placed between "<<borders[i]<<"-"<<borders[i+1]<<endl;
			//cout<<"temp density = "<<tempCA / tempWS<<endl;
			linearScaling(targetCells, currentCell, formerCell, borders[i],
					borders[i + 1], level);
		}

		formerCell = currentCell - 1;
		if (currentCell <= halfCellNo) {
			break;
		}
	}

	//	for (long i = 0; i < targetCells.size(); ++i){
	//		cout<<level<<" out "<<targetCells[i]->getCenterX()<<" "<<targetCells[i]->getCenterY()<<endl;
	//	}
	//	long x;
	//	cin >> x;


	//cout<<"[DEBUG]: "<<targetCells.size() - currentCell<<" cells are placed in right part"<<endl;
	///checkOrder(targetCells, (level % 2 == 0));


	/*if (level%2 == 0){  //x-direction
	 long i = 0;
	 double center = region.left() + targetCells[0]->getWidth() / 2;
	 while (targetCells[i]->getCenterX() <= center + 10e-6){
	 targetCells[i]->setCoordX(center - targetCells[i]->getWidth() / 2);
	 center = targetCells[i]->getCenterX();
	 i++;
	 if (i == targetCells.size()){
	 cout<<"[WARNING]: x+ = "<<i<<"/"<<targetCells.size()<<endl;
	 break;
	 }
	 if (center < region.left() + targetCells[i]->getWidth() / 2){
	 center = region.left() + targetCells[i]->getWidth() / 2;
	 }
	 }


	 i = targetCells.size() - 1;
	 center = region.right() - targetCells[i]->getWidth() / 2;
	 while (targetCells[i]->getCenterX() >= center - 10e-6){
	 targetCells[i]->setCoordX(center - targetCells[i]->getWidth() / 2);
	 center = targetCells[i]->getCenterX();
	 i--;
	 if (i == -1){
	 cout<<"[WARNING]: x- = "<<i<<"/"<<targetCells.size()<<endl;
	 break;
	 }
	 if (center > region.right() - targetCells[i]->getWidth() / 2){
	 center = region.right() - targetCells[i]->getWidth() / 2;
	 }
	 }


	 }
	 else{           //y-direction
	 long i = 0;
	 double center = region.bottom() + targetCells[0]->getHeight() / 2;
	 while (targetCells[i]->getCenterY() <= center + 10e-6){
	 targetCells[i]->setCoordY(center - targetCells[i]->getHeight() / 2);
	 center = targetCells[i]->getCenterY();
	 i++;
	 if (i == targetCells.size()){
	 cout<<"[WARNING]: y+ = "<<i<<"/"<<targetCells.size()<<endl;
	 break;
	 }
	 if (center < region.bottom() + targetCells[i]->getHeight() / 2){
	 center = region.bottom() + targetCells[i]->getHeight() / 2;
	 }
	 }

	 i = targetCells.size() - 1;
	 center = region.top() - targetCells[i]->getHeight() / 2;
	 while (targetCells[i]->getCenterY() >= center - 10e-6){
	 targetCells[i]->setCoordY(center - targetCells[i]->getHeight() / 2);
	 center = targetCells[i]->getCenterY();
	 i--;
	 if (i == -1){
	 cout<<"[WARNING]: y- = "<<i<<"/"<<targetCells.size()<<endl;
	 break;
	 }
	 if (center > region.top() - targetCells[i]->getHeight() / 2){
	 center = region.top() - targetCells[i]->getHeight() / 2;
	 }
	 }
	 }*/

	//checkOrder(targetCells, (level % 2 == 0));

	return;
}

void SimPlPlace::linearScaling(vector<Inst*> targetCells, long begin, long end,
		long boundaryL, long boundaryH, int level) {
	assert(end >= begin);
	assert(boundaryH > boundaryL);
	assert(begin >= 0);
	assert(end < targetCells.size());
	//for debug
	vector<Inst*> tempInsts;
	tempInsts.clear();
	double x0 = 0;
	double x0_new = 0;
	double x1 = 0;
	double x1_new = 0;
	double coord = 0;
	double extend = 0;
	long TEST = 0;


	if (end == begin) {
		return;
	}

	//DEBUG

//	vector<long> cnNodes;
//	vector<long>::iterator it;
//	cnNodes.clear();
//	vector<InstTerm> pinsOnNode = validNodes[211446]->getInstTerms();
//	long netIndex = 0;
//	for (long k = 0; k < pinsOnNode.size(); ++k) {
//		netIndex = pinsOnNode[k].getIndexNet();
//		for (long t = 0; t < nets[netIndex]->getNumTerms(); ++t) {
//			if (nets[netIndex]->getTerms()[t].getIndexInst() == 211446
//					|| validNodes[nets[netIndex]->getTerms()[t].getIndexInst()]->getStatus() != Moved) {
//				continue;
//			}
//			cnNodes.push_back(nets[netIndex]->getTerms()[t].getIndexInst());
//		}
//	}
//	cout << "The number of elements in cnNodes is " << cnNodes.size() << endl;
//	cin >> TEST;


	//x-direction
	if (level % 2 == 0) {
		x0 = targetCells[begin]->getCenterX();
		x1 = targetCells[end]->getCenterX();
		x0_new = boundaryL;
		x1_new = boundaryH;

		double step = (x1_new - x0_new) / (end - begin);

		//assert(x1 >= x0 + (-10e-6));
		//TODO unsure
		if (1) {
//		if (abs(x1 - x0) < 10e-6) {
			for (long i = begin; i <= end; ++i) {
//				it = find(cnNodes.begin(), cnNodes.end(), targetCells[i]->getIndex());
//				if (it != cnNodes.end()){
//					continue;
//				}


				targetCells[i]->setCoordX(
						boundaryL + step * (i - begin)
								- targetCells[i]->getWidth() / 2);
				if (i != begin) {
					assert(targetCells[i]->getCenterX() - targetCells[i-1]->getCenterX() > -10e-6);
				}
			}
		}
		else {
//			cout << "OK!" <<endl;
//			while(1);
			assert(x1_new > x0_new - 10e-6);
			extend = (x1_new - x0_new) / (x1 - x0);
			if (extend <= 0) {
				cout << extend << " " << x0_new << " " << x1_new << " " << x0
						<< " " << x1 << endl;
			}
			assert(extend > 0);
			for (long i = begin; i <= end; ++i) {
				assert(targetCells[i]->getCenterX() > x0 - 10e-6);
				coord = x0_new + extend * (targetCells[i]->getCenterX() - x0);
				if (coord < x0_new - 10e-6) {
					cout << "[ERROR]: " << coord << " " << x0_new << endl;
				}

				targetCells[i]->setCoordX(
						coord - targetCells[i]->getWidth() / 2);

				if (begin == 121 && end == 122) {
					cout << " !!! " << targetCells[0]->getCenterX() << endl;
				}

				if (!targetCells[i]->moved) {
					targetCells[i]->moved = true;
				} else {
					cout << "condition 2: " << targetCells[i]->getId()
							<< " is moved!" << endl;
				}

				assert(coord <= x1_new + (10e-6));
				assert(coord >= x0_new + (-10e-6));
				if (i != begin) {
					if (targetCells[i]->getCenterX()
							- targetCells[i - 1]->getCenterX() <= -10e-6) {
						cout << "#" << i - 1 << " "
								<< targetCells[i - 1]->getCenterX() << ", #"
								<< i << " " << targetCells[i]->getCenterX()
								<< endl;
					}
					assert(targetCells[i]->getCenterX() - targetCells[i-1]->getCenterX() > -10e-6);
				}
			}
		}
	}

	//y-direction
	else {
		x0 = targetCells[begin]->getCenterY();
		x1 = targetCells[end]->getCenterY();
		x0_new = boundaryL;
		x1_new = boundaryH;

		double step = (x1_new - x0_new) / (end - begin);

		//assert(x1 >= x0 - 10e-6);
		//TODO
		if (1) {
			//		if (abs(x1 - x0) < 10e-6){
			for (long i = begin; i <= end; ++i) {
				targetCells[i]->setCoordY(
						boundaryL + step * (i - begin)
								- targetCells[i]->getHeight() / 2);

			}
		} else {
			assert(x1 > x0 - 10e-6);
			assert(x1_new > x0_new - 10e-6);
			extend = (x1_new - x0_new) / (x1 - x0);
			assert(extend > 0);
			for (long i = begin; i <= end; ++i) {
				assert(targetCells[i]->getCenterY() > x0 - 10e-6);
				coord = x0_new + extend * (targetCells[i]->getCenterY() - x0);
				targetCells[i]->setCoordY(
						coord - targetCells[i]->getHeight() / 2);
				if (!targetCells[i]->moved) {
					targetCells[i]->moved = true;
				} else {
					cout << "condition 4: " << targetCells[i]->getId()
							<< " is moved!" << endl;
				}
				if (i != begin) {
					assert(targetCells[i]->getCenterY() - targetCells[i-1]->getCenterY() > -10e-6);
				}
			}
		}
	}
}

void SimPlPlace::globalRefine() {
	vector<Inst*> targetCells;
	vector<bool> lock;
	queue<Inst*> v;
	Inst* instNow;
	long id;
	double cellAreaIn = 0;
	double cellAreaOut = 0;

	//prepare data
	for (long i = 0; i < numValidNodes; ++i) {
		if (validNodes[i]->getStatus() == Moved) {
			targetCells.push_back(validNodes[i]);
			long gridX = (validNodes[i]->getCenterX() - region.left())
					/ gridSize;
			long gridY = (validNodes[i]->getCenterY() - region.bottom())
					/ gridSize;
			bins[gridX][gridY]->moveInstsX.push_back(validNodes[i]);
			lock.push_back(false);
		}
	}
	assert(targetCells.size() == numMoveNodes);
	//sort nodes by the number of pins in descend order
	sort(targetCells.begin(), targetCells.end(), morePin);

	//sort nodes within bins in the same order
	for (long i = 0; i < gridNumX; ++i) {
		for (long j = 0; j < gridNumY; ++j) {
			sort(bins[i][j]->moveInstsX.begin(), bins[i][j]->moveInstsX.end(),
					morePin);
		}
	}

	//cout<<"[GlobalRefine]: init complete"<<endl;

	for (long i = 0; i < targetCells.size(); ++i) {
		id = targetCells[i]->getId();
		if (lock[id] == false) {
			v.push(targetCells[i]);
			//cout<<"[GlobalRefine]: move cell "<<targetCells[i]->getName()<<endl;
		} else {
			continue;
		}
		while (!v.empty()) {
			instNow = v.front();
			//cout<<"current position of instNow : "<<instNow->getName()<<" "<<instNow->getCoordX()<<" "<<instNow->getCoordY()<<endl;
			v.pop();
			cellAreaIn = instNow->getArea();
			//get optimal bin for cell instNow
			Rect netBox(0, 0, 0, 0);
			Rect optBox(0, 0, 0, 0);
			if (instNow->getNumInstTerms() == 0) {
				return;
			}

			vector<long> x(2 * instNow->getNumInstTerms());
			vector<long> y(2 * instNow->getNumInstTerms());
			vector<InstTerm>& instTermTemp = instNow->getInstTerms();
			long t = 0;
			for (vector<InstTerm>::iterator iter = instTermTemp.begin(); iter
					!= instTermTemp.end(); ++iter) {
				myNet* net = validNets[(*iter).getIndexNet()];
				myPoint pnt;
				long yt = region.bottom(), yb = region.top(), xl =
						region.right(), xr = region.left();
				vector<InstTerm>& instTerms = net->getTerms();
				for (vector<InstTerm>::iterator iter1 = instTerms.begin(); iter1
						!= instTerms.end(); ++iter1) {
					if ((*iter1).getIndexInst() != (*iter).getIndexInst()) {
						Inst* conInst = validNodes[(*iter1).getIndexInst()];
						if (conInst->getCenterX() > xr)
							xr = conInst->getCenterX();
						if (conInst->getCenterX() < xl)
							xl = conInst->getCenterX();
						if (conInst->getCenterY() > yt)
							yt = conInst->getCenterY();
						if (conInst->getCenterY() < yb)
							yb = conInst->getCenterY();
					}
				}
				// cout<<"net box "<<net->getNumTerms()<<"---"<<xl<<" "<<xr<<" "<<yb<<" "<<yt<<endl;
				netBox.setCoord(xl, yb, xr, yt);
				if (netBox.top() == region.bottom())
					continue;
				x[t] = netBox.left();
				y[t++] = netBox.bottom();
				x[t] = netBox.right();
				y[t++] = netBox.top();
			}
			if (t == 0) {
				//cout<<"if "<<endl;
				optBox.setLeft(instNow->getCoordX());
				optBox.setRight(instNow->getCoordX() + instNow->getWidth());
				optBox.setBottom(instNow->getCoordY());
				optBox.setTop(instNow->getCoordY() + instNow->getHeight());
			} else {
				//cout<<" else"<<endl;
				sort(x.begin(), x.end());
				sort(y.begin(), y.end());
				//for (long s =0; s < x.size(); ++s){
				//	cout<<x[s]<<" ";
				//}
				//cout<<endl;
				//for (long s =0; s < y.size(); ++s){
				//	cout<<y[s]<<" ";
				//}
				//cout<<endl;
				optBox.setLeft(x[x.size() / 2 - 1]);
				optBox.setRight(x[x.size() / 2]);
				optBox.setBottom(y[y.size() / 2 - 1]);
				optBox.setTop(y[y.size() / 2]);
			}
			x.clear();
			y.clear();

			// cout<<"[GlobalRefine]: optimal region for cell "<<instNow->getName()<<" is "<<optBox.left()
			//<<" "<<optBox.right()<<" "<<optBox.bottom()<<" "<<optBox.top()<<endl;

			long gridXl = (optBox.left() - region.left()) / gridSize;
			long gridXr = (optBox.right() - region.left()) / gridSize;
			long gridYb = (optBox.bottom() - region.bottom()) / gridSize;
			long gridYt = (optBox.top() - region.bottom()) / gridSize;

			//if instNow is already in its optimal region, skip
			if (instNow->getCenterX() >= bins[gridXl][gridYb]->left
					&& instNow->getCenterX() <= bins[gridXr][gridYt]->right
					&& instNow->getCenterY() >= bins[gridXl][gridYb]->bottom
					&& instNow->getCenterY() <= bins[gridXr][gridYt]->top) {
				id = instNow->getId();
				// cout<<"already in optimal region "<<instNow->getCenterX()<<" "<<instNow->getCenterY()<<endl;
				lock[id] = true;
				continue;
			}
			//cout<<"optimal bin for cell "<<instNow->getName()<<" is "<<(gridXl + gridXr) / 2<<" "<<(gridYb + gridYt) / 2<<endl;
			RLRegion* targetBin = bins[(gridXl + gridXr) / 2][(gridYb + gridYt)
					/ 2];
			// cout<<"targetbin:!!!"<<targetBin->moveInstsX.size()<<endl;
			//find some cells within targetBins to move out, perserving density
			for (long j = 0; j < targetBin->moveInstsX.size(); ++j) {
				id = targetBin->moveInstsX[j]->getId();
				if (lock[id] == false) {
					instNow->setCoordX(targetBin->moveInstsX[j]->getCoordX());
					instNow->setCoordY(targetBin->moveInstsX[j]->getCoordY());

					//cout<<"[INFO]: global refine---"<<instNow->getName()<<" is set to "<<targetBin->moveInstsX[j]->getCoordX()<<", "<<targetBin->moveInstsX[j]->getCoordY()<<endl;

					v.push(targetBin->moveInstsX[j]);
					//cout<<targetBin->moveInstsX[j]->getName()<<" is pushed in queue"<<endl;
					cellAreaOut += targetBin->moveInstsX[j]->getArea();
				}
				if (cellAreaOut > cellAreaIn - averageNodeArea / 5) {
					break;
				}
			}
			id = instNow->getId();
			lock[id] = true;
		}
	}
}

void SimPlPlace::setNetCenterOfInst() {
	Inst* instNow;
	double sumX;
	double sumY;
	vector<Inst*> insts = plcTopBlock->getInsts();
	vector<myNet*> nets = plcTopBlock->getNets();

	for (long i = 0; i < validNodes.size(); ++i) {
		//cout<<"i = "<<i<<endl;
		instNow = validNodes[i];
		sumX = 0;
		sumY = 0;
		if (instNow->getNumInstTerms() == 0) {
			//cout<<"There is no pin on inst "<<instNow->getName()<<" "<<instNow->getStatus()<<" "<<", continue!!"<<endl;
			continue;
		}
		vector<InstTerm>& instTermTemp = instNow->getInstTerms();
		long t = 0;
		for (vector<InstTerm>::iterator iter = instTermTemp.begin(); iter
				!= instTermTemp.end(); ++iter) {
			//cout<<"\t"<<instTermTemp.size()<<" 111111 in"<<endl;
			//cout<<(*iter).getIndexNet()<<endl;
			myNet* net = nets[(*iter).getIndexNet()];
			//cout<<net->getName()<<endl;
			vector<InstTerm>& instTerms = net->getTerms();
			//cout<<"\t\t"<<instTerms.size()<<endl;
			for (vector<InstTerm>::iterator iter1 = instTerms.begin(); iter1
					!= instTerms.end(); ++iter1) {
				//cout<<"\t\t222222 in "<<endl;
				//cout<<(*iter).getIndexInst()<<endl;
				//cout<<(*iter1).getIndexInst()<<endl;
				//cout<<"aaa"<<endl;
				if ((*iter1).getIndexInst() != (*iter).getIndexInst()) {
					Inst* conInst = insts[(*iter1).getIndexInst()];
					sumX += conInst->getCenterX();
					sumY += conInst->getCenterY();
					t++;
				}
			}
		}
		instNow->setGCX(sumX / t);
		instNow->setGCY(sumY / t);
	}
	//cout<<"set center finished"<<endl;
}

void SimPlPlace::saveOldPos() {
	oldPos.clear();
	for (long i = 0; i < numMoveNodes; ++i) {
		myPoint p(validNodes[i]->getCenterX(), validNodes[i]->getCenterY());
		oldPos.push_back(p);
	}
}

void SimPlPlace::guiFile(const char* fname) {
	ofstream gpFile(fname);
	ifstream inPlName("instName.tmp");
	string cell = string(fname) + "_cell";
	string obstacle = string(fname) + "_fix";
	gpFile << "set title \" ------Placement Result------ \"" << endl;
	gpFile << "set xrange [" << blockX << ":" << blockX + blockW << "]" << endl;
	gpFile << "set yrange [" << blockY << ":" << blockY + blockH << "]" << endl;
	/*gpFile << "set xrange [" << "0" << ":"
	 << blockX + blockW + 50000 << "]" << endl;
	 gpFile << "set yrange [" << "0" << ":"
	 << blockY + blockH + 50000<< "]" << endl;*/
	/*gpFile << "set xtics [" << blockX << ","
	 << blockX + blockW << "]" << endl;
	 gpFile << "set ytics [" << blockY << ":"
	 << blockY + blockH << "]" << endl*/;
	gpFile << "plot \"" << cell << "\" with steps," << "\"" << obstacle
			<< "\" with steps" << endl;

	const char* fname_cell = cell.c_str();
	const char* fname_obstacle = obstacle.c_str();
	ofstream cFile(fname_cell);
	ofstream fixFile(fname_obstacle);
	vector<Inst*> &insts = plcTopBlock->getInsts();
	gpFile << endl << endl;
	long j;
	string name;
	for (j = 0; j < (long) insts.size(); j++) {
		if (insts[j]->getStatus() == Moved) {
			cFile << "# Cell Name " << insts[j]->getName() << endl;

			cFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			cFile << endl << endl;
		} else {
			fixFile << "# Cell Name " << insts[j]->getName() << endl;

			fixFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			fixFile << endl << endl;
		}

	}
	inPlName.close();
	cFile.close();
	fixFile.close();
	gpFile.close();
}

void SimPlPlace::guiRect(const char* fname, vector<Triple>& overf, long x1, long x2, long y1, long y2) {
	ofstream gpFile(fname);
	ifstream inPlName("instName.tmp");
	string cell = string(fname) + "_cell";
	string obstacle = string(fname) + "_fix";
	string rects = string(fname) + "_region";
	string bin = string(fname) + "_overfilledBins";
	gpFile << "set title \" ------Placement Result------ \"" << endl;
	gpFile << "set xrange [" << blockX << ":" << blockX + blockW << "]" << endl;
	gpFile << "set yrange [" << blockY << ":" << blockY + blockH << "]" << endl;
	/*gpFile << "set xtics [" << blockX << ","
	 << blockX + blockW << "]" << endl;
	 gpFile << "set ytics [" << blockY << ":"
	 << blockY + blockH << "]" << endl*/;
	gpFile << "plot \"" << cell << "\" with steps, " << "\"" << obstacle
			<< "\" with steps, " << "\"" << bin << "\" with steps, " << "\""
			<< rects << "\" with steps" << endl;

	const char* fname_cell = cell.c_str();
	const char* fname_obstacle = obstacle.c_str();
	const char* fname_rect = rects.c_str();
	const char* fname_bin = bin.c_str();
	ofstream cFile(fname_cell);
	ofstream fixFile(fname_obstacle);
	ofstream rectFile(fname_rect);
	ofstream binFile(fname_bin);
	vector<Inst*> &insts = plcTopBlock->getInsts();
	gpFile << endl << endl;
	long j;
	string name;
	for (j = 0; j < (long) insts.size(); j++) {
		if (insts[j]->getStatus() == Moved) {
			inPlName >> name;
			cFile << "# Cell Name " << name << endl;

			cFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			cFile << endl << endl;
		} else {
			inPlName >> name;
			fixFile << "# Cell Name " << name << endl;

			fixFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			fixFile << endl << endl;
		}

	}

	binFile << "# Bin " << endl;

	for(long i = 0; i < (long)overf.size() ; ++i){
		binFile << setw(10) << bins[overf[i].row][overf[i].column]->left << "  " << setw(10)
				<< bins[overf[i].row][overf[i].column]->bottom << "   " << endl;
		binFile << setw(10) << bins[overf[i].row][overf[i].column]->right << "  " << setw(10)
				<< bins[overf[i].row][overf[i].column]->top << "   " << endl;
		binFile << setw(10) << bins[overf[i].row][overf[i].column]->right << "  " << setw(10)
				<< bins[overf[i].row][overf[i].column]->top << "   " << endl;
		binFile << setw(10) << bins[overf[i].row][overf[i].column]->left << "  " << setw(10)
				<< bins[overf[i].row][overf[i].column]->bottom << "   " << endl;
		binFile << endl << endl;
	}

	rectFile << "# Rect " << endl;

	rectFile << setw(10) << x1 << "  " << setw(10) << y1 << "   " << endl;
	rectFile << setw(10) << x2 << "  " << setw(10) << y2 << "   " << endl;
	rectFile << setw(10) << x2 << "  " << setw(10) << y2 << "   " << endl;
	rectFile << setw(10) << x1 << "  " << setw(10) << y1 << "   " << endl;
	rectFile << endl << endl;

	cFile.close();
	fixFile.close();
	rectFile.close();
	gpFile.close();
	inPlName.close();
}

void SimPlPlace::guiClustersGroup(const char* fname, vector<RLRegion*>& clus , vector<Triple>& overf) {
	ofstream gpFile(fname);
	ifstream inPlName("instName.tmp");
	string cell = string(fname) + "_cell";
	string obstacle = string(fname) + "_fix";
	string rect = string(fname) + "_region";
	string bin = string(fname) + "_overfillesBins";
	gpFile << "set title \" ------Placement Result------ \"" << endl;
	gpFile << "set xrange [" << blockX << ":" << blockX + blockW << "]" << endl;
	gpFile << "set yrange [" << blockY << ":" << blockY + blockH << "]" << endl;
	/*gpFile << "set xtics [" << blockX << ","
	 << blockX + blockW << "]" << endl;
	 gpFile << "set ytics [" << blockY << ":"
	 << blockY + blockH << "]" << endl*/;
	gpFile << "plot \"" << cell << "\" with steps, " << "\"" << obstacle
			<< "\" with steps, " << "\"" << bin
			<< "\" with steps, "<< "\"" << rect << "\" with steps" << endl;

	const char* fname_cell = cell.c_str();
	const char* fname_obstacle = obstacle.c_str();
	const char* fname_rect = rect.c_str();
	const char* fname_bin = bin.c_str();
	ofstream cFile(fname_cell);
	ofstream fixFile(fname_obstacle);
	ofstream rectFile(fname_rect);
	ofstream binFile(fname_bin);

	vector<Inst*> &insts = plcTopBlock->getInsts();
	gpFile << endl << endl;
	long j;
	string name;
	for (j = 0; j < (long) insts.size(); j++) {
		if (insts[j]->getStatus() == Moved) {
			inPlName >> name;
			cFile << "# Cell Name " << name << endl;

			cFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			cFile << endl << endl;
		} else {
			inPlName >> name;
			fixFile << "# Cell Name " << name << endl;

			fixFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			fixFile << endl << endl;
		}

	}

	binFile << "# Bin " << endl;

	for(long i = 0; i < (long)overf.size() ; ++i){
		binFile << setw(10) << bins[overf[i].row][overf[i].column]->left << "  " << setw(10)
				<< bins[overf[i].row][overf[i].column]->bottom << "   " << endl;
		binFile << setw(10) << bins[overf[i].row][overf[i].column]->right << "  " << setw(10)
				<< bins[overf[i].row][overf[i].column]->top << "   " << endl;
		binFile << setw(10) << bins[overf[i].row][overf[i].column]->right << "  " << setw(10)
				<< bins[overf[i].row][overf[i].column]->top << "   " << endl;
		binFile << setw(10) << bins[overf[i].row][overf[i].column]->left << "  " << setw(10)
				<< bins[overf[i].row][overf[i].column]->bottom << "   " << endl;
		binFile << endl << endl;
	}

	rectFile << "# Rect " << endl;

	for (long i = 0; i < (long) clus.size(); ++i) {
		rectFile << setw(10) << clus[i]->left << "  " << setw(10)
				<< clus[i]->bottom << "   " << endl;
		rectFile << setw(10) << clus[i]->right << "  " << setw(10)
				<< clus[i]->top << "   " << endl;
		rectFile << setw(10) << clus[i]->right << "  " << setw(10)
				<< clus[i]->top << "   " << endl;
		rectFile << setw(10) << clus[i]->left << "  " << setw(10)
				<< clus[i]->bottom << "   " << endl;
		rectFile << endl << endl;
	}



	cFile.close();
	fixFile.close();
	rectFile.close();
	binFile.close();
	gpFile.close();
	inPlName.close();
}

void SimPlPlace::guiRectGroup(const char* fname, vector<Rect>& rects) {
	ofstream gpFile(fname);
	ifstream inPlName("instName.tmp");
	string cell = string(fname) + "_cell";
	string obstacle = string(fname) + "_fix";
	string rect = string(fname) + "_region";
	gpFile << "set title \" ------Placement Result------ \"" << endl;
	gpFile << "set xrange [" << blockX << ":" << blockX + blockW << "]" << endl;
	gpFile << "set yrange [" << blockY << ":" << blockY + blockH << "]" << endl;
	/*gpFile << "set xtics [" << blockX << ","
	 << blockX + blockW << "]" << endl;
	 gpFile << "set ytics [" << blockY << ":"
	 << blockY + blockH << "]" << endl*/;
	gpFile << "plot \"" << cell << "\" with steps, " << "\"" << obstacle
			<< "\" with steps, " << "\"" << rect << "\" with steps" << endl;

	const char* fname_cell = cell.c_str();
	const char* fname_obstacle = obstacle.c_str();
	const char* fname_rect = rect.c_str();
	ofstream cFile(fname_cell);
	ofstream fixFile(fname_obstacle);
	ofstream rectFile(fname_rect);
	vector<Inst*> &insts = plcTopBlock->getInsts();
	gpFile << endl << endl;
	long j;
	string name;
	for (j = 0; j < (long) insts.size(); j++) {
		if (insts[j]->getStatus() == Moved) {
			inPlName >> name;
			cFile << "# Cell Name " << name << endl;

			cFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			cFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			cFile << endl << endl;
		} else {
			inPlName >> name;
			fixFile << "# Cell Name " << name << endl;

			fixFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX()
					+ insts[j] -> getWidth() << "  " << setw(10)
					<< insts[j] -> getCoordY() + insts[j] -> getHeight()
					<< "   " << endl;
			fixFile << setw(10) << insts[j] -> getCoordX() << "  " << setw(10)
					<< insts[j] -> getCoordY() << "   " << endl;
			fixFile << endl << endl;
		}

	}

	rectFile << "# Rect " << endl;

	for (long i = 0; i < (long) rects.size(); ++i) {
		rectFile << setw(10) << rects[i].left() << "  " << setw(10)
				<< rects[i].bottom() << "   " << endl;
		rectFile << setw(10) << rects[i].right() << "  " << setw(10)
				<< rects[i].top() << "   " << endl;
		rectFile << setw(10) << rects[i].right() << "  " << setw(10)
				<< rects[i].top() << "   " << endl;
		rectFile << setw(10) << rects[i].left() << "  " << setw(10)
				<< rects[i].bottom() << "   " << endl;
		rectFile << endl << endl;
	}

	cFile.close();
	fixFile.close();
	rectFile.close();
	gpFile.close();
	inPlName.close();
}

void SimPlPlace::checkOrder(vector<Inst*> insts, bool horizontal) {
	if (horizontal) {
		for (int i = 0; i < (long) insts.size() - 1; ++i) {
			if (insts[i + 1]->getCenterX() < insts[i]->getCenterX() - 10e-6) {
				cout << "[ERROR2483]: #" << i << ": " << insts[i]->getCenterX()
						<< ", #" << i + 1 << ": " << insts[i + 1]->getCenterX()
						<< endl;
				cout << insts[i]->getName() << " " << insts[i]->getCoordX()
						<< " " << insts[i]->getWidth() << endl;
			}
			assert(insts[i+1]->getCenterX() - insts[i]->getCenterX() >= -10e-6);
			assert(insts[i+1]->getId() != insts[i]->getId());
		}
	} else {
		for (int i = 0; i < (long) insts.size() - 1; ++i) {
			if (insts[i + 1]->getCenterY() < insts[i]->getCenterY() - 10e-6) {
				cout << "[ERROR2495]: #" << i << ": " << insts[i]->getCenterY()
						<< ", #" << i + 1 << ": " << insts[i + 1]->getCenterY()
						<< endl;
			}
			assert(insts[i+1]->getCenterY() >= insts[i]->getCenterY() - 10e-6);
			assert(insts[i+1]->getId() != insts[i]->getId());
		}
	}
}

void SimPlPlace::constraintMove(double alpha) {
	for (long i = 0; i < validNodes.size(); ++i) {
		if (validNodes[i]->getStatus() == Moved) {
			double coordx = alpha * validNodes[i]->getCenterX() + (1 - alpha)
					* oldPos[i].coordX();
			double coordy = alpha * validNodes[i]->getCenterY() + (1 - alpha)
					* oldPos[i].coordY();
			validNodes[i]->setCoordX(coordx - validNodes[i]->getWidth() / 2);
			validNodes[i]->setCoordY(coordy - validNodes[i]->getHeight() / 2);
		}
	}
}

void SimPlPlace::placeAtBinCenter() {
	long x = 0, y = 0;
	for (long i = 0; i < numMoveNodes; ++i) {
		y = (validNodes[i]->getCenterY() - blockY) / gridSize;
		validNodes[i]->setCoordX(
				bins[x][y]->left + bins[x][y]->getWidth() / 2
						- validNodes[i]->getWidth() / 2);
		validNodes[i]->setCoordY(
				bins[x][y]->bottom + bins[x][y]->getHeight() / 2
						- validNodes[i]->getHeight() / 2);
	}
}

void SimPlPlace::centerDiffusion(RLRegion* region) {
	long gridLeft = (region->left - blockX) / gridSize;
	long gridRight = (region->right - blockX) / gridSize;
	long gridBottom = (region->bottom - blockY) / gridSize;
	long gridTop = (region->top - blockY) / gridSize;

	double availableArea;

	vector<Inst*> insts = region->moveInstsX;

	//x-direction
	long gridX = gridLeft;
	availableArea = 0;
	for (long y = gridBottom; y < gridTop; ++y) {
		availableArea += bins[gridX][y]->availableArea;
	}
	for (long i = 0; i < insts.size(); ++i) {
		if (availableArea <= 0) {
			gridX++;
			if (gridX < gridRight) {
				for (long y = gridBottom; y < gridTop; ++y) {
					availableArea += bins[gridX][y]->availableArea;
				}
			} else {
				cout << "x-direction: not enough available area!!! gridX = "
						<< gridX << " / " << gridRight << ", cellNum = " << i
						<< " / " << insts.size();
			}
		} else {
			insts[i]->setCoordX(
					blockX + gridSize * gridX - insts[i]->getWidth() / 2);
			availableArea -= insts[i]->getArea();
		}
	}

	//y-direction
	long gridY = gridBottom;
	availableArea = 0;
	for (long x = gridLeft; x < gridRight; ++x) {
		availableArea += bins[x][gridY]->availableArea;
	}
	for (long i = 0; i < insts.size(); ++i) {
		if (availableArea <= 0) {
			gridY++;
			if (gridY < gridTop) {
				for (long x = gridLeft; x < gridRight; ++x) {
					availableArea += bins[x][gridY]->availableArea;
				}
			} else {
				cout << "y-direction: not enough available area!!! gridY = "
						<< gridY << " / " << gridTop << ", cellNum = " << i
						<< " / " << insts.size();
			}
		} else {
			insts[i]->setCoordY(
					blockY + gridSize * gridY - insts[i]->getHeight() / 2);
			availableArea -= insts[i]->getArea();
		}
	}

	guiFile("centerDiffusion.gnu");
	cout << "pause" << endl;
	long x;
	cin >> x;
}

void SimPlPlace::globalRefine1() {
	vector<long> horizontalMove;
	vector<long> verticalMove;
	vector<Inst*> tmpInsts;

	horizontalMove.clear();
	verticalMove.clear();
	horizontalMove.resize(numMoveNodes);
	verticalMove.resize(numMoveNodes);
	tmpInsts.clear();
	for (long i = 0; i < validNets.size(); ++i) {
		if (validNets[i]->getNumTerms() > 50) {
			continue;
		}
		tmpInsts.clear();
		vector<InstTerm>& terms = validNets[i]->getTerms();
		for (long j = 0; j < terms.size(); ++j) {
			if (getInst(terms[j])->getStatus() != Moved) {
				continue;
			}
			tmpInsts.push_back(getInst(terms[j]));
		}

		for (long index1 = 0; index1 < tmpInsts.size(); index1++) {
			for (long index2 = index1 + 1; index2 < tmpInsts.size(); index2++) {
				Inst* inst1 = tmpInsts[index1];
				Inst* inst2 = tmpInsts[index2];
				if (inst1->getCenterX() > inst2->getCenterX()) {
					horizontalMove[inst1->getId()]--;
					horizontalMove[inst2->getId()]++;
				} else if (inst1->getCenterX() < inst2->getCenterX()) {
					horizontalMove[inst1->getId()]++;
					horizontalMove[inst2->getId()]--;
				} else {

				}

				if (inst1->getCenterY() > inst2->getCenterY()) {
					verticalMove[inst1->getId()]--;
					verticalMove[inst2->getId()]++;
				} else if (inst1->getCenterY() < inst2->getCenterY()) {
					verticalMove[inst1->getId()]--;
					verticalMove[inst2->getId()]++;
				} else {

				}
			}
		}
	}

	for (long i = 0; i < numMoveNodes; ++i) {
		if (horizontalMove[i] > 5) {
			validNodes[i]->setCoordX(validNodes[i]->getCoordX() + gridSize);
			cout << "move right" << endl;
		} else if (horizontalMove[i] < -5) {
			validNodes[i]->setCoordX(validNodes[i]->getCoordX() - gridSize);
			cout << "move left" << endl;
		}

		if (verticalMove[i] > 5) {
			validNodes[i]->setCoordY(validNodes[i]->getCoordY() + gridSize);
			cout << "move up" << endl;
		} else if (verticalMove[i] < -5) {
			validNodes[i]->setCoordY(validNodes[i]->getCoordY() - gridSize);
			cout << "move down" << endl;
		}
	}
}

void SimPlPlace::reOrder(vector<Inst*>& insts, bool isDirectX) {
	long a, b;
	long start = -1, end = insts.size() - 1;
	long countX = 0, countY = 0;
	cout << insts.size() << " " << insts.front()->getCenterX() << " "
			<< insts.back()->getCenterX() << endl;
	if (isDirectX) {
		for (long i = 0; i < (long) insts.size() - 1; ++i) {
			if ((long) insts[i]->getCenterX()
					== (long) insts[i + 1]->getCenterX() && i != insts.size()
					- 2) {
				if (start == -1) {
					start = i;
					cout << "startX = " << start << endl;
					continue;
				} else {
					continue;
				}
			} else {
				cout << "endX =  " << end << endl;
				end = i;
			}
			for (int r = start; r <= end; ++r) {
				a = start + (long) insts[i]->getId() % (end - start + 1);
				b = start + (long) insts[i + 1]->getId() % (end - start + 1);
				Inst* temp = insts[a];
				insts[a] = insts[b];
				insts[b] = temp;
				countX++;
			}
			start = -1;
			end = insts.size() - 1;
		}
	} else {
		for (long i = 0; i < (long) insts.size() - 1; ++i) {
			if ((long) insts[i]->getCenterY()
					== (long) insts[i + 1]->getCenterY()) {
				if (start == -1) {
					start = i;
				} else {
				}
			} else {
				end = i;
				for (int r = start; r <= end; ++r) {
					a = start + (long) insts[i]->getId() % (end - start + 1);
					b = start + (long) insts[i + 1]->getId()
							% (end - start + 1);
					Inst* temp = insts[a];
					insts[a] = insts[b];
					insts[b] = temp;
					countY++;
				}
				start = -1;
				end = insts.size() - 1;
			}
		}
	}
	cout << "countX = " << countX << ", countY = " << countY << endl;
}
