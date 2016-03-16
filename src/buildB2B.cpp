#include <iostream>
#include <algorithm>
#include <vector>
#include "simPlPlace.h"
#include "point.h"

using namespace std;

int COUNT = 0;

void SimPlPlace::freeB2B() {
	for (long i = 0; i < numMoveNodes; ++i) {
		B2B_MatrixX[i].clear();
		B2B_MatrixY[i].clear();
	}
	B2B_MatrixX.clear();
	B2B_MatrixY.clear();
	Bx.clear();
	By.clear();
	return;
}

void SimPlPlace::buildB2B() {
	bool isDirX = true;

	COUNT = 0;

	//for debug
	long tempCount = 0;

	// initialize
	B2B_MatrixX.resize(numMoveNodes);
	B2B_MatrixY.resize(numMoveNodes);
	Bx.resize(numMoveNodes);
	By.resize(numMoveNodes);
	for (long id = 0; id < numMoveNodes; ++id) {
		Triple diagT;
		diagT.row = id;
		diagT.column = id;
		diagT.element = 0.0;
		B2B_MatrixX[id].push_back(diagT);
		B2B_MatrixY[id].push_back(diagT);
		//assert(B2B_MatrixX[id].size() == 1);
		//assert(B2B_MatrixY[id].size() == 1);
		Bx[id] = 0.0;
		By[id] = 0.0;
	}
	// calculate
	for (long netId = 0; netId < (long) validNets.size(); netId++) {
		//long netWeight = validNets[netId]->getWeight();
		long netWeight = 1;
		vector<InstTerm>& instTerm = validNets[netId] -> getTerms();
		assert(instTerm.size() > 1);

		// boundary pins
		vector<Inst*> netInsts;
		getInstsOfNet(validNets[netId], netInsts);
		assert(netInsts.size() > 1);

		long pinsNum = netInsts.size();

		if (pinsNum > 500) {
			//cout<<validNets[netId]->getName()<<" "<<pinsNum<<endl;
		}

		Inst* tmp1 = netInsts[0];

		myPoint origin1 = tmp1->getCenter();
		double minX = origin1.coordX();
		double minY = origin1.coordY();
		double maxX = origin1.coordX();
		double maxY = origin1.coordY();
		Inst* lowerNodeX = tmp1;
		Inst* upperNodeX = tmp1;
		Inst* lowerNodeY = tmp1;
		Inst* upperNodeY = tmp1;
		for (long instId = 1; instId < netInsts.size(); instId++) {
			Inst* tmp = netInsts[instId];

			myPoint origin = tmp->getCenter();

			if (origin.coordX() > maxX) {
				maxX = origin.coordX();
				upperNodeX = tmp;
				//cout<<"X: "<<origin.coordX() <<" > "<<maxX<<", upperNodeX = "<<tmp->getId()<<endl;
			}
			if (origin.coordX() <= minX) {
				minX = origin.coordX();
				lowerNodeX = tmp;
				//cout<<"X: "<<origin.coordX() <<" < "<<minX<<", lowerNodeX = "<<tmp->getId()<<endl;
			}
			if (origin.coordY() > maxY) {
				maxY = origin.coordY();
				upperNodeY = tmp;
			}
			if (origin.coordY() <= minY) {
				minY = origin.coordY();
				lowerNodeY = tmp;
			}
		}

		// build Q matrix 
		long lowerIdX = lowerNodeX -> getId();
		long upperIdX = upperNodeX -> getId();
		long lowerIdY = lowerNodeY -> getId();
		long upperIdY = upperNodeY -> getId();

		//for debug
		if (lowerIdX == upperIdX) {
			cout << lowerIdX << " " << upperIdX << endl;
			cout << lowerNodeX->getName() << " " << upperNodeX->getName()
					<< endl;
			cout << pinsNum << endl;
			for (long i = 0; i < pinsNum; ++i) {
				cout << instTerm[i].getName() << " "
						<< getInst(instTerm[i])->getName() << " " << getInst(
						instTerm[i])->getId() << endl;
				cout << getInst(instTerm[i])->getCenterX() << " " << getInst(
						instTerm[i])->getCenterY() << endl;
				cout << getInst(instTerm[i])->getCoordX() << " " << getInst(
						instTerm[i])->getWidth() << endl;

			}
		}
		assert(lowerIdX != upperIdX);
		if (lowerIdY == upperIdY) {
			cout << lowerIdY << " " << upperIdY << endl;
			cout << lowerNodeY->getName() << " " << upperNodeY->getName()
					<< endl;
			cout << pinsNum << endl;
			for (long i = 0; i < pinsNum; ++i) {
				cout << instTerm[i].getName() << " "
						<< getInst(instTerm[i])->getName() << " " << getInst(
						instTerm[i])->getId() << endl;
				cout << getInst(instTerm[i])->getCenterX() << " " << getInst(
						instTerm[i])->getCenterY() << endl;
				cout << getInst(instTerm[i])->getCoordY() << " " << getInst(
						instTerm[i])->getHeight() << endl;

			}
		}
		assert(lowerIdY != upperIdY);

		//for debug
		if (lowerNodeX->getCenterX() >= upperNodeX->getCenterX()) {
			//cout<<"[WARNING]: lowerX equals upperX "<<lowerNodeX->getCenterX()<<" "<<upperNodeX->getCenterX()<<endl;;
			//continue;
		}
		if (lowerNodeY->getCenterY() >= upperNodeY->getCenterY()) {
			//cout<<"[WARNING]: lowerY equals upperY "<<lowerNodeY->getCenterY()<<" "<<upperNodeY->getCenterY()<<endl;;
			//continue;
		}

		//assert(lowerNodeX->getCenterX() < upperNodeX->getCenterX());
		//assert(lowerNodeY->getCenterY() < upperNodeY->getCenterY());

		//set net center added by ZhouQ
		assert(maxX >= minX);
		assert(maxY >= minY);
		validNets[netId]->centerX = (maxX + minX) / 2;
		validNets[netId]->centerY = (maxY + minY) / 2;

		//for debug
		//if (lowerNodeX->getCenterX() == upperNodeX->getCenterX() &&
		//		lowerNodeY->getCenterY() == upperNodeY->getCenterY()){
		//	COUNT++;
		//}
		//copy two nodes on boundary of net
		double weight = 0.0;
		weight = calWeight(lowerNodeX, upperNodeX, pinsNum, isDirX, netWeight);
		if (lowerIdX < numMoveNodes) {
			if (upperIdX < numMoveNodes) {
				//movable and movable
				insertElement(upperIdX, lowerIdX, -weight, isDirX);
			} else {
				//movable and fixed
				//cout<<weight<<" "<<upperNodeX->getCenterX()<<endl;
				Bx[lowerIdX] += weight * upperNodeX->getCenterX();
				B2B_MatrixX[lowerIdX][0].element += weight;
			}
		} else {
			//cout<<weight<<" "<<lowerNodeX->getCenterX()<<endl;
			if (upperIdX < numMoveNodes) {
				//fixed and movable
				Bx[upperIdX] += weight * lowerNodeX->getCenterX();
				B2B_MatrixX[upperIdX][0].element += weight;
			} else {
				tempCount++;
				//totalCount++;
			}
		}
		weight = calWeight(lowerNodeY, upperNodeY, pinsNum, !isDirX, netWeight);
		if (lowerIdY < numMoveNodes) {
			if (upperIdY < numMoveNodes) {
				//movable and movable
				insertElement(upperIdY, lowerIdY, -weight, !isDirX);
			} else {
				//movable and fixed
				//cout<<weight<<" "<<lowerNodeY->getCenterY()<<endl;
				By[lowerIdY] += weight * upperNodeY->getCenterY();
				B2B_MatrixY[lowerIdY][0].element += weight;
			}
		} else {
			//cout<<weight<<" "<<lowerNodeY->getCenterY()<<endl;
			if (upperIdY < numMoveNodes) {
				//fixed and movable
				By[upperIdY] += weight * lowerNodeY->getCenterY();
				B2B_MatrixY[upperIdY][0].element += weight;
			} else {
				tempCount++;
			}
		}
		//copy nodes inside boundary of net
		for (long instId = 0; instId < netInsts.size(); instId++) {
			Inst *inst = netInsts[instId];
			long centerId = inst -> getId();
			if (centerId < numMoveNodes) {
				//inst is movable
				// x-direction 
				if (centerId != lowerIdX && centerId != upperIdX) {
					//if (abs(inst->getCenterX() - lowerNodeX->getCenterX()) > 10e-6){
					weight = calWeight(inst, lowerNodeX, pinsNum, isDirX,
							netWeight);
					if (lowerIdX >= numMoveNodes) { //lower node is fixed
						Bx[centerId] += weight * inst->getCenterX();
						B2B_MatrixX[centerId][0].element += weight;
					} else {
						insertElement(centerId, lowerIdX, -weight, isDirX);
					}
					//}

					//if (abs(inst->getCenterX() - upperNodeX->getCenterX()) > 10e-6){
					weight = calWeight(inst, upperNodeX, pinsNum, isDirX,
							netWeight);
					if (upperIdX >= numMoveNodes) { //upper node is fixed
						Bx[centerId] += weight * inst->getCenterX();
						B2B_MatrixX[centerId][0].element += weight;
					} else {
						insertElement(centerId, upperIdX, -weight, isDirX);
					}
					//}
				}
				// y-direction
				if (centerId != lowerIdY && centerId != upperIdY) {
					//if (abs(inst->getCenterY() - lowerNodeX->getCenterY()) > 10e-6){
					weight = calWeight(inst, lowerNodeY, pinsNum, !isDirX,
							netWeight);
					if (lowerIdY >= numMoveNodes) { //lower node is fixed
						By[centerId] += weight * inst->getCenterY();
						B2B_MatrixY[centerId][0].element += weight;
					} else {
						insertElement(centerId, lowerIdY, -weight, !isDirX);
					}
					//}
					//if (abs(inst->getCenterY() - upperNodeX->getCenterY()) > 10e-6){
					weight = calWeight(inst, upperNodeY, pinsNum, !isDirX,
							netWeight);
					if (upperIdY >= numMoveNodes) { //upper node is fixed
						By[centerId] += weight * inst->getCenterY();
						B2B_MatrixY[centerId][0].element += weight;
					} else {
						insertElement(centerId, upperIdY, -weight, !isDirX);
					}
					//}
				}
			} else {
				//inst is fixed
				//x-direction
				if (centerId != lowerIdX && centerId != upperIdX) {
					if (lowerIdX < numMoveNodes) {
						weight = calWeight(inst, lowerNodeX, pinsNum, isDirX,
								netWeight);
						Bx[lowerIdX] += weight * inst->getCenterX();
						B2B_MatrixX[lowerIdX][0].element += weight;
					}
					if (upperIdX < numMoveNodes) {
						weight = calWeight(inst, upperNodeX, pinsNum, isDirX,
								netWeight);
						Bx[upperIdX] += weight * inst->getCenterX();
						B2B_MatrixX[upperIdX][0].element += weight;
					}
				}
				//y-direction
				if (centerId != lowerIdY && centerId != upperIdY) {
					if (lowerIdY < numMoveNodes) {
						weight = calWeight(inst, lowerNodeY, pinsNum, !isDirX,
								netWeight);
						By[lowerIdY] += weight * inst->getCenterY();
						B2B_MatrixY[lowerIdY][0].element += weight;
					}
					if (upperIdY < numMoveNodes) {
						weight = calWeight(inst, upperNodeY, pinsNum, !isDirX,
								netWeight);
						By[upperIdY] += weight * inst->getCenterY();
						B2B_MatrixY[upperIdY][0].element += weight;
					}
				}
			}
		}
	}
	return;
}

double SimPlPlace::calWeight(Inst* instA, Inst* instB, long num, bool isDirX,
		long netWeight) {
	myPoint originA = instA -> getCenter();
	myPoint originB = instB -> getCenter();
	if (netWeight != 1) {
		cout << "net weight = " << netWeight << endl;
	}
	//    assert(originA.coordX() >= region.left() || instA->getStatus() != Moved);
	//    assert(originA.coordX() <= region.right() || instA->getStatus() != Moved);
	//    assert(originA.coordY() >= region.bottom() || instA->getStatus() != Moved);
	//    assert(originA.coordY() <= region.top() || instA->getStatus() != Moved);
	//    assert(originB.coordX() >= region.left() || instB->getStatus() != Moved);
	//
	//    assert(originB.coordX() <= region.right() || instB->getStatus() != Moved);
	//    assert(originB.coordY() >= region.bottom() || instB->getStatus() != Moved);
	//    assert(originB.coordY() <= region.top() || instB->getStatus() != Moved);

	double base = 2.0;
	double eps = 0.1;
	double param = 100.0;
	//double constraint = 100;

	//double extendX = constraint / region.width();
	//double extendY = constraint / region.height();
	//double coordX = 0;
	//double coordY = 0;
	//coordX = extendX * (originA.coordX() - region.left());
	//coordY = extendY * (originA.coordY() - region.bottom());
	//originA.setCoordXY(coordX, coordY);

	//coordX = extendX * (originB.coordX() - region.left());
	//coordY = extendY * (originB.coordY() - region.bottom());
	//originB.setCoordXY(coordX, coordY);

	//cout<<originA.coordX()<<" "<<originB.coordX()<<endl;
	if (isDirX) {
		double w = base / (1.0 * (num - 1) * (eps*param + abs(
				(originA.coordX())*param - (originB.coordX())*param))) * netWeight;
//		double w = base / (1.0 * (num - 1) * (eps + abs(
//				originA.coordX() - originB.coordX()))) * netWeight;
//	    double dividend_x = 1.0*num*abs(originA.coordX() - originB.coordX()) - abs(originA.coordX() - originB.coordX()) -
//				eps + 1.0*num*eps;
//		double w = base / dividend_x * netWeight;
		assert(w > 0);
		//return double(long(w * 10e6)) / 10e6;
		return w;
		//return w * (num - 1);
	} else {
		double w = base / (1.0 * (num - 1) * (eps*param + abs(
				(originA.coordY())*param - (originB.coordY())*param))) * netWeight;
//		double w = base / (1.0 * (num - 1) * (eps + abs(
//				originA.coordY() - originB.coordY()))) * netWeight;
//		double dividend_y = 1.0*num*abs(originA.coordY() - originB.coordY()) - abs(originA.coordY() - originB.coordY()) -
//				eps + 1.0*num*eps;
//		double w = base / dividend_y * netWeight;
		assert(w > 0);
		//return w * (num - 1);
		//return double(long(w * 10e6)) / 10e6;
		return w;
	}
}

void SimPlPlace::insertElement(long row, long column, double element,
		bool isDirX) {
	assert(row != column);
	Triple triple;
	triple.row = row;
	triple.column = column;
	triple.element = element;
	if (element == 0) {
		cout << "Error : element = 0" << endl;
		return;
	} else {
		assert(element < 0);
	}
	if (isDirX) {
		B2B_MatrixX[row].push_back(triple);
		triple.row = column;
		triple.column = row;
		B2B_MatrixX[column].push_back(triple);
		B2B_MatrixX[row][0].element -= element;
		B2B_MatrixX[column][0].element -= element;
	} else {
		B2B_MatrixY[row].push_back(triple);
		triple.row = column;
		triple.column = row;
		B2B_MatrixY[column].push_back(triple);
		B2B_MatrixY[row][0].element -= element;
		B2B_MatrixY[column][0].element -= element;
	}
}

void SimPlPlace::compressMatrixX(int type, int dim) {
	Xp.clear();
	Xi.clear();
	Xx.clear();
	diagX.clear();

	for (long i = 0; i < dim; i++) {
		sort(B2B_MatrixX[i].begin(), B2B_MatrixX[i].end(), greater<Triple> ());
	}

	if (type == 3) { //eigen solver
		//get lower triangle part
		for (long i = 0; i < dim; i++) {
			for (vector<Triple>::iterator t = B2B_MatrixX[i].begin(); t
					!= B2B_MatrixX[i].end(); ++t) {
				if ((*t).row < (*t).column) {
					t = B2B_MatrixX[i].erase(t);
					t--;
				}
			}
		}
		return;
	}

	long nnz = 0;

	for (long id = 0; id < dim; ++id) {
		long numRow = B2B_MatrixX[id].size();
		if (type == 1 || type == 0) {
			Xp.push_back(nnz);
		}
		bool vl = false;
		for (long j = 0; j < numRow; ++j) {
			//test
			/*cout<<B2B_MatrixX[id][j].row<<"   "
			 <<B2B_MatrixX[id][j].column<<"   "
			 <<B2B_MatrixX[id][j].element<<endl;*/
			if (type == 1) { //for cholmod solver
				if (!vl && (B2B_MatrixX[id][j].element) <= 0) {
					continue;
				} else {
					vl = true;
				}
				if (B2B_MatrixX[id][j].column > id) {
					if (Xi[nnz - 1] == B2B_MatrixX[id][j].column) {
						Xx[nnz - 1] += (B2B_MatrixX[id][j].element);
						continue;
					}
				}
			} else if (type == 0 || type == 2) { //for amgpcg solver and iccg
				if (j > 0) {
					assert(B2B_MatrixX[id][j].row == B2B_MatrixX[id][j-1].row);
					if (Xi[nnz - 1] == B2B_MatrixX[id][j].column) {
						Xx[nnz - 1] += (B2B_MatrixX[id][j].element);
						continue;
					}
				}
				if (type == 2) { //for iccg
					Xp.push_back(B2B_MatrixX[id][j].row); //for iccg
				}
			} else {
				cout << "Error : type = " << type << " was not supported."
						<< endl;
			}
			//cout<<"xi push back: "<<B2B_MatrixX[id][j].column<<endl;
			Xi.push_back(B2B_MatrixX[id][j].column);
			//cout<<"xx push back: "<<B2B_MatrixX[id][j].element<<endl<<endl;
			Xx.push_back(B2B_MatrixX[id][j].element);

			if (id == B2B_MatrixX[id][j].column) {
				diagX.push_back(nnz);
				if (Xx[nnz] <= 0) {
					cout << id << " " << j << " " << Xx[nnz] << endl;
					cout << B2B_MatrixX[id][j].row << " "
							<< B2B_MatrixX[id][j].column << " "
							<< B2B_MatrixX[id][j].element << endl;
				}
				assert(Xx[nnz] > 0);
			}
			++nnz;
		}
	}
	if (type == 0 || type == 1) {
		Xp.push_back(nnz);
	}
#if 0
	for(int i = 0; i < numMoveNodes; ++i) {
		if (Xp[i] == Xp[i+1]) {
			cout<<"Error : Xp[i]==Xp[i+1] : "<<i<<endl;
		}
	}
#endif
	assert(dim == (long)diagX.size());
	assert(nnz == (long)Xi.size());
	assert(nnz == (long)Xx.size());
	return;
}

void SimPlPlace::compressMatrixY(int type, int dim) {
	Yp.clear();
	Yi.clear();
	Yx.clear();
	diagY.clear();

	for (long i = 0; i < dim; i++) {
		sort(B2B_MatrixY[i].begin(), B2B_MatrixY[i].end(), greater<Triple> ());
	}

	if (type == 3) { //eigen solver
		//get lower triangle part
		for (long i = 0; i < dim; i++) {
			for (vector<Triple>::iterator t = B2B_MatrixY[i].begin(); t
					!= B2B_MatrixY[i].end(); ++t) {
				if ((*t).row < (*t).column) {
					t = B2B_MatrixY[i].erase(t);
					t--;
				}
			}
		}
		return;
	}

	long nnz = 0;

	for (long id = 0; id < dim; ++id) {
		long numRow = B2B_MatrixY[id].size();
		if (type == 1 || type == 0) {
			Yp.push_back(nnz);
		}
		bool vl = false;
		for (long j = 0; j < numRow; ++j) {
			if (type == 1) { //for cholmod solver
				if (!vl && (B2B_MatrixY[id][j].element) <= 0) {
					continue;
				} else {
					vl = true;
				}
				if (B2B_MatrixY[id][j].column > id) {
					if (Yi[nnz - 1] == B2B_MatrixY[id][j].column) {
						Yx[nnz - 1] += (B2B_MatrixY[id][j].element);
						continue;
					}
				}
			} else if (type == 0 || type == 2) { //for amgpcg solver
				if (j > 0) {
					assert(B2B_MatrixY[id][j].row == B2B_MatrixY[id][j-1].row);
					if (Yi[nnz - 1] == B2B_MatrixY[id][j].column) {
						Yx[nnz - 1] += (B2B_MatrixY[id][j].element);
						continue;
					}
				}
				if (type == 2) {
					Yp.push_back(B2B_MatrixY[id][j].row); //for iccg
				}
			} else {
				cout << "Error : type = " << type << " was not supported."
						<< endl;
			}
			Yi.push_back(B2B_MatrixY[id][j].column);
			Yx.push_back(B2B_MatrixY[id][j].element);
			if (id == B2B_MatrixY[id][j].column) {
				diagY.push_back(nnz);
				assert(Yx[nnz] > 0);
			}
			++nnz;
		}
	}

	if (type == 0 || type == 1) {
		Yp.push_back(nnz);
	}
#if 0
	for(int i = 0; i < numMoveNodes; ++i) {
		if (Yp[i] == Yp[i+1]) {
			cout<<"Error : Yp[i]==Yp[i+1] : "<<i<<endl;
		}
	}
#endif
	assert(dim == (long)diagY.size());
	assert(nnz == (long)Yi.size());
	assert(nnz == (long)Yx.size());

	return;
}

//quan add, for matlab
bool dumpFile2(const char* file1, const char* file2, int n, double *a, int *ja,
		int *ia, double *b) {
	int i, k;
	FILE *fp = NULL;
	if (file1) {
		fp = fopen(file1, "w");
		for (i = 0; i < n; i++) {
			for (k = ia[i]; k < ia[i + 1]; k++) {
				//fprintf(fp, "%d %d %1.9f\n", i, ja[k], a[k]);
				//add by ZQ, output for matlab
				fprintf(fp, "%d %d %1.9f\n", i + 1, ja[k] + 1, a[k]);
			}
		}
		fclose(fp);
	}

	if (file2) {
		fp = fopen(file2, "w");
		for (i = 0; i < n; i++) {
			fprintf(fp, "%f\n", b[i]);
		}
		fclose(fp);
	}
	return true;
}

/*bool dumpFile2(const char* file1, const char* file2, int n, double *a,
 int *ja, int *ia, double *b)
 {
 int i, k;
 FILE *fp = NULL;
 if (file1) {
 fp = fopen(file1, "w");
 for(i = 0; i < n; i++) {
 for(k = ia[i]; k < ia[i+1]; k++) {
 fprintf(fp, "%d %d %1.9f\n", i, ja[k], a[k]);
 }
 }
 fclose(fp);
 }

 if (file2) {
 fp = fopen(file2, "w");
 for(i = 0; i < n; i++) {
 fprintf(fp, "%f\n", b[i]);
 }
 fclose(fp);
 }
 return true;
 }*/
void checking(int n, double *a, int *ja, int *ia, double *b, double *sol) {
	double *gap = new double[n];
	double maxG = 0;
	double maxR = 0;
	double maxR_r = 0;
	double maxR_s = 0;
	int maxIndex = 0;
	//ofstream fout;
	//fout.open("gapX.txt");
	double QtimesSol;

	for (int r = 0; r < n; ++r) {
		QtimesSol = 0;
		for (int i = ia[r]; i < ia[r + 1]; ++i) {
			/*if (r == 201186 ){
			 cout<<QtimesSol<<" += "<<a[i]<<"("<<r<<") * "<<sol[ja[i]]<<"("<<ja[i]<<") = "<<QtimesSol + a[i] * sol[ja[i]]<<endl;
			 }*/
			QtimesSol += a[i] * sol[ja[i]];

		}
		if (b[r] != 0) {
			gap[r] = (QtimesSol - b[r]) / b[r];

			if (gap[r] > maxR) {
				maxR = gap[r];
				maxR_r = b[r];
				maxR_s = QtimesSol;
				maxIndex = r;
			}
		}
		maxG = max(maxG, QtimesSol - b[r]);
		//fout<<gap[r]<<endl;
	}
	cout << "max gapR is " << maxR << endl;
	cout << "max gapR_r is " << maxR_r << endl;
	cout << "max gapR_s is " << maxR_s << endl;
	cout << "max Index = " << maxIndex << endl;
	cout << "max gapG is " << maxG << endl;
	delete[] gap;
}

void SimPlPlace::linearSolveX(int type, int dim) {
	//cout<<"dimension = "<<numMoveNodes<<endl;
	if (type == 3) {
		linearSolverX();
		return;
	}

	int nnz = Xp[dim];
	if (type == 2) {//iccg
		nnz = Xp.size();
	}
	assert(nnz == (long)Xi.size());
	//cout<<"nonZero = "<<nnz<<endl;
	int *ccol = NULL;
	int *col = NULL;
	if (type == 0 || type == 1) {
		ccol = new int[dim + 1];
	} else if (type == 2) {
		col = new int[nnz]; //for iccg
	} else {
		cout << "Error : type = " << type << " was not supported." << endl;
	}
	int* row = new int[nnz];
	double *val = new double[nnz];
	double *bx = new double[dim];
	double *sol = new double[dim];
	if (type == 0 || type == 1) {
		for (long i = 0; i <= dim; ++i) {
			ccol[i] = Xp[i];
		}
	}
	for (long i = 0; i < nnz; ++i) {
		if (type == 2) {
			col[i] = Xp[i]; //for iccg
		}
		row[i] = Xi[i];
		val[i] = Xx[i];
		assert(row[i] < (int)dim);
	}
	for (long i = 0; i < dim; ++i) {
		//assert(Bx[i] >= 0);
		bx[i] = Bx[i];
		sol[i] = 0.0;
	}
#if 0  //for test
	//for (int i = 0; i < nnz; ++i) { //for iccg
	for (int i = 0; i <= numMoveNodes; ++i) { //for amgpcg and cholmod
		cout<<col[i]<<"   ";
	}
	cout<<endl;
	for (int i = 0; i < nnz; ++i) {
		cout<<row[i]<<"  ";
	}
	cout<<endl;
	for (int i = 0; i < nnz; ++i) {
		cout<<val[i]<<"  ";
	}
	cout<<endl;
	for (int i = 0; i < numMoveNodes; ++i) {
		cout<<bx[i]<<"   ";
	}
	cout<<endl;
#endif
	//dumpFile2("xa.txt", "xb.txt", numMoveNodes, val, row, ccol, bx);
	//cout<<"nnzX = "<<nnz<<endl;

	if (type == 0) {
		amgpcg(dim, val, row, ccol, bx, sol, 100, 1e-5, 0);
		//fpCG(ccol, row, val, nnz, numMoveNodes, bx, sol);
		//classicCG(numMoveNodes, val, row, ccol, bx, sol, 100, 1e-5, 0);
		//checking(numMoveNodes, val, row, ccol, origin_bx, sol);
		//dumpFile2(NULL, "amgpcgXsol.txt", numMoveNodes, NULL, NULL, NULL, sol);
	} else if (type == 1) {
		//cholmodSolve3(numMoveNodes, nnz, row, ccol, val, bx, sol);
		//checking(numMoveNodes, val, row, ccol, origin_bx, sol);
		//dumpFile2(NULL, "cholmodXsol.txt", numMoveNodes, NULL, NULL, NULL, sol);
	} else if (type == 2) {
		//iccgSolve(row, col, val, nnz, numMoveNodes, bx, sol);
		//dumpFile2(NULL, "iccgXsol.txt", numMoveNodes, NULL, NULL, NULL, sol);
	} else {
		cout << "Error : type = " << type << " was not supported." << endl;
	}

	for (long i = 0; i < numMoveNodes; ++i) {
		//cout<<sol[i]<<"  "<<endl;
		//		sol[i] = max(sol[i], (double)region.left() + validNodes[i]->getWidth() / 2);
		//		sol[i] = min(sol[i], (double)region.right() - validNodes[i]->getWidth() / 2);
		validNodes[i]->setCoordX(sol[i] - validNodes[i]->getWidth() / 2);
	}

	if (type == 0 || type == 1) {
		delete[] ccol;
	} else if (type == 2) {
		delete[] col;
	}
	delete [] row;
	delete [] val;
	delete [] bx;
	delete[] sol;

	return;
}

void SimPlPlace::linearSolveY(int type, int dim) {
	if (type == 3) {
		linearSolverY();
		return;
	}
	//cout<<"dimension = "<<numMoveNodes<<endl;
	int nnz = Yp[dim];
	if (type == 2) {//iccg
		nnz = Yp.size();
	}
	assert(nnz == (long)Yi.size());
	//cout<<"nonZero = "<<nnz<<endl;
	int *ccol = NULL;
	int *col = NULL;
	if (type == 0 || type == 1) {
		ccol = new int[dim + 1];
	} else if (type == 2) {
		col = new int[nnz]; //for iccg
	} else {
		cout << "Error : type = " << type << " was not supported." << endl;
	}
	int* row = new int[nnz];
	double *val = new double[nnz];
	double *by = new double[dim];
	double *sol = new double[dim];
	if (type == 0 || type == 1) {
		for (long i = 0; i <= dim; ++i) {
			ccol[i] = Yp[i];
		}
	}
	for (long i = 0; i < nnz; ++i) {
		if (type == 2) {
			col[i] = Yp[i]; //for iccg
		}
		row[i] = Yi[i];
		val[i] = Yx[i];
		assert(row[i] < (int)dim);
	}
	for (long i = 0; i < dim; ++i) {
		//assert(By[i] >= 0);
		by[i] = By[i];
		sol[i] = 0.0;
	}

#if 0 //for test
	//for (int i = 0; i < nnz; ++i) { //for iccg
	for (int i = 0; i <= numMoveNodes; ++i) { //for amg and cholmod
		cout<<"Bp["<<i<<"] = "<<Yp[i]<<endl;
	}
	for (int i = 0; i < nnz; ++i) {
		cout<<"Bi["<<i<<"] = "<<Yi[i]<<endl;
	}
	for (int i = 0; i < nnz; ++i) {
		cout<<"Bx["<<i<<"] = "<<Yx[i]<<endl;
	}
	for (int i = 0; i < numMoveNodes; ++i) {
		cout<<"By["<<i<<"] = "<<By[i]<<endl;
	}
#endif
	//dumpFile2("ya.txt", "yb.txt", numMoveNodes, val, row, ccol, by);

	if (type == 0) {
		amgpcg(dim, val, row, ccol, by, sol, 100, 1e-5, 0);
		//fpCG(ccol, row, val, nnz, numMoveNodes, by, sol);

		//classicCG(numMoveNodes, val, row, ccol, by, sol, 100, 1e-5, 0);
		//checking(numMoveNodes, val, row, ccol, origin_by, sol);
		//dumpFile2(NULL, "amgpcgYsol.txt", numMoveNodes, NULL, NULL, NULL, sol);

		//for debug
		//cout<<"solver pause"<<endl;
		//long x;
		//cin>>x;
		//cholmodSolve3(numMoveNodes, nnz, row, ccol, val, by, sol);
		//dumpFile2(NULL, "cholmodYsol.txt", numMoveNodes, NULL, NULL, NULL, sol);
	} else if (type == 2) {
		//iccgSolve(row, col, val, nnz, numMoveNodes, by, sol);
		//dumpFile2(NULL, "iccgYsol.txt", numMoveNodes, NULL, NULL, NULL, sol);
	} else {
		cout << "Error : type = " << type << " was not supported." << endl;
	}

	for (long i = 0; i < numMoveNodes; ++i) {
		//cout<<sol[i]<<"  "<<endl;
		//		sol[i] = max(sol[i], (double)region.bottom() + validNodes[i]->getHeight() / 2);
		//		sol[i] = min(sol[i], (double)region.top() - validNodes[i]->getHeight() / 2);
		validNodes[i]->setCoordY(sol[i] - validNodes[i]->getHeight() / 2);
	}

	if (type == 0 || type == 1) {
		delete[] ccol;
	} else if (type == 2) {
		delete[] col;
	}
	delete [] row;
	delete [] val;
	delete [] by;
	delete[] sol;

	return;
}
void SimPlPlace::buildClique() {
	// initialize
	B2B_MatrixX.resize(numMoveNodes);
	B2B_MatrixY.resize(numMoveNodes);
	Bx.resize(numMoveNodes);
	By.resize(numMoveNodes);
	for (long id = 0; id < numMoveNodes; ++id) {
		Triple diagT;
		diagT.row = id;
		diagT.column = id;
		diagT.element = 0.0;
		B2B_MatrixX[id].push_back(diagT);
		B2B_MatrixY[id].push_back(diagT);
		//assert(B2B_MatrixX[id].size() == 1);
		//assert(B2B_MatrixY[id].size() == 1);
		Bx[id] = 0.0;
		By[id] = 0.0;
	}
	for (long netId = 0; netId < (long) validNets.size(); netId++) {
		long pinsNum = validNets[netId] -> getNumTerms();
		assert(pinsNum > 1);
		double weight = 1.0 / (pinsNum - 1);

		vector<InstTerm>& instTerm = validNets[netId] -> getTerms();
		assert(instTerm.size() > 1);
		for (long tId1 = 0; tId1 < pinsNum; tId1++) {
			for (long tId2 = tId1 + 1; tId2 < pinsNum; tId2++) {
				Inst* tmp1 = getInst(instTerm[tId1]);
				Inst* tmp2 = getInst(instTerm[tId2]);

				if (tmp1->getId() == tmp2->getId()) {
					continue;
				}

				if (tmp1->getStatus() != Moved) {
					if (tmp2->getStatus() != Moved) {
						continue;
					} else {
						//movable2 and fixed1
						Bx[tmp2->getId()] += weight * tmp1->getCenterX();
						B2B_MatrixX[tmp2->getId()][0].element += weight;
						By[tmp2->getId()] += weight * tmp1->getCenterY();
						B2B_MatrixY[tmp2->getId()][0].element += weight;
					}
				} else {
					if (tmp2->getStatus() != Moved) {
						//movable1 and fixed2
						Bx[tmp1->getId()] += weight * tmp2->getCenterX();
						B2B_MatrixX[tmp1->getId()][0].element += weight;
						By[tmp1->getId()] += weight * tmp2->getCenterY();
						B2B_MatrixY[tmp1->getId()][0].element += weight;
					} else {
						//movable1 and movable2
						insertElement(tmp1->getId(), tmp2->getId(), -weight,
								true);
						insertElement(tmp1->getId(), tmp2->getId(), -weight,
								false);
					}
				}
			}
		}
	}
	for (int i = 0; i < numMoveNodes; ++i) {
		assert(B2B_MatrixX[i][0].element > 0);
	}
}

void SimPlPlace::buildHybrid() {
	//cout<<"num star net = "<<numStarNets<<"/ "<<validNets.size()<<endl;
	long starCount = 0;
	// initialize
	B2B_MatrixX.resize(numMoveNodes + numStarNets);
	B2B_MatrixY.resize(numMoveNodes + numStarNets);
	Bx.resize(numMoveNodes + numStarNets);
	By.resize(numMoveNodes + numStarNets);
	for (long id = 0; id < numMoveNodes + numStarNets; ++id) {
		Triple diagT;
		diagT.row = id;
		diagT.column = id;
		diagT.element = 0.0;
		B2B_MatrixX[id].push_back(diagT);
		B2B_MatrixY[id].push_back(diagT);
		//assert(B2B_MatrixX[id].size() == 1);
		//assert(B2B_MatrixY[id].size() == 1);
		Bx[id] = 0.0;
		By[id] = 0.0;
	}
	for (long netId = 0; netId < (long) validNets.size(); netId++) {
		long pinsNum = validNets[netId] -> getNumTerms();
		assert(pinsNum > 1);
		if (pinsNum <= 3) {
			//clique model
			double weight = 1.0 / (pinsNum - 1);
			vector<InstTerm>& instTerm = validNets[netId] -> getTerms();
			assert(instTerm.size() > 1);
			for (long tId1 = 0; tId1 < pinsNum; tId1++) {
				for (long tId2 = tId1 + 1; tId2 < pinsNum; tId2++) {
					Inst* tmp1 = getInst(instTerm[tId1]);
					Inst* tmp2 = getInst(instTerm[tId2]);

					if (tmp1->getId() == tmp2->getId()) {
						continue;
					}

					if (tmp1->getStatus() != Moved) {
						if (tmp2->getStatus() != Moved) {
							continue;
						} else {
							//movable2 and fixed1
							Bx[tmp2->getId()] += weight * tmp1->getCenterX();
							B2B_MatrixX[tmp2->getId()][0].element += weight;
							By[tmp2->getId()] += weight * tmp1->getCenterY();
							B2B_MatrixY[tmp2->getId()][0].element += weight;
						}
					} else {
						if (tmp2->getStatus() != Moved) {
							//movable1 and fixed2
							Bx[tmp1->getId()] += weight * tmp2->getCenterX();
							B2B_MatrixX[tmp1->getId()][0].element += weight;
							By[tmp1->getId()] += weight * tmp2->getCenterY();
							B2B_MatrixY[tmp1->getId()][0].element += weight;
						} else {
							//movable1 and movable2
							insertElement(tmp1->getId(), tmp2->getId(),
									-weight, true);
							insertElement(tmp1->getId(), tmp2->getId(),
									-weight, false);
						}
					}
				}
			}
		} else {
			//star model
			double weight = 1.0 * pinsNum / (pinsNum - 1);
			vector<InstTerm>& instTerm = validNets[netId] -> getTerms();
			assert(instTerm.size() > 1);
			for (long tId1 = 0; tId1 < pinsNum; tId1++) {
				Inst* tmp1 = getInst(instTerm[tId1]);

				if (tmp1->getStatus() != Moved) {
					//movable2 and fixed1
					Bx[numMoveNodes + starCount] += weight * tmp1->getCenterX();
					B2B_MatrixX[numMoveNodes + starCount][0].element += weight;
					By[numMoveNodes + starCount] += weight * tmp1->getCenterY();
					B2B_MatrixY[numMoveNodes + starCount][0].element += weight;
				} else {
					insertElement(tmp1->getId(), numMoveNodes + starCount,
							-weight, true);
					insertElement(tmp1->getId(), numMoveNodes + starCount,
							-weight, false);
				}
			}
			starCount++;
		}
	}
	assert(starCount == numStarNets);
	for (int i = 0; i < numMoveNodes; ++i) {
		assert(B2B_MatrixX[i][0].element > 0);
	}
}

/*int cholmodSolve3(int dim, int nnz, int* Ap, int*Ai, double* Ax, double* bb, double*sol){
 int* row = new int[nnz];
 int* col = new int[nnz];
 double* val = new double[nnz];
 cout<<"[INFO]: nnz = "<<nnz<<endl;
 for (long i = 0; i < dim; ++i){
 for (long j = Ai[i]; j < Ai[i+1]; ++j){
 //cout<<i<<" "<<j<<" "<<Ap[j]<<" "<<Ax[j]<<"\t"<<dim<<endl;
 //assert(j < dim);
 //if (Ap[j] >= i){
 row[j] = i;
 col[j] = Ap[j];
 val[j] = Ax[j];
 assert(col[j] < dim);
 //}
 //cout<<j<<": "<<row[j]<<" "<<col[j]<<" "<<val[j]<<endl;
 }
 }
 cholmod_triplet tri;
 tri.nnz = nnz;
 tri.ncol = dim;
 tri.nrow = dim;
 tri.i = row;
 tri.j = col;
 tri.x = val;
 tri.xtype = 1;
 tri.stype = 1;

 cholmod_sparse *A ;
 cholmod_dense *x, *b, *r ;
 cholmod_factor *L ;
 double one [2] = {1,0}, m1 [2] = {-1,0} ; // basic scalars
 cholmod_common c ;
 cholmod_start (&c) ; // start CHOLMOD
 A = cholmod_triplet_to_sparse(&tri, 1, &c) ; // read in a matrix
 cholmod_print_sparse (A, "A", &c) ; // print the matrix
 if (A == NULL || A->stype == 0) // A must be symmetric
 {
 cholmod_free_sparse (&A, &c) ;
 cholmod_finish (&c) ;
 return 0;
 }
 //cout<<"here1"<<endl;
 //b = cholmod_ones (A->nrow, 1, A->xtype, &c) ; // b = ones(n,1)
 b = cholmod_allocate_dense(dim, 1, dim, 1, &c);
 b->x = bb;

 cholmod_print_dense(b, "b", &c);

 L = cholmod_analyze (A, &c) ; // analyze
 cholmod_print_factor(L, "L", &c);

 //cout<<"here2"<<endl;

 cholmod_factorize (A, L, &c) ; // factorize
 x = cholmod_solve (CHOLMOD_A, L, b, &c) ; // solve Ax=b
 r = cholmod_copy_dense (b, &c) ; // r = b
 cholmod_sdmult (A, 0, m1, one, x, r, &c) ; // r = r-Ax

 printf ("norm(b-Ax) %8.1e\n",
 cholmod_norm_dense (r, 0, &c)) ; // print norm(r)

 //cholmod_print_dense(x, "x", &c);

 sol = static_cast<double*>(x->x);
 //cout<<sol[0]<<" "<<sol[1]<<" "<<sol[2]<<endl;

 cholmod_free_factor (&L, &c) ; // free matrices
 cholmod_free_sparse (&A, &c) ;
 cholmod_free_dense (&r, &c) ;
 cholmod_free_dense (&x, &c) ;
 cholmod_free_dense (&b, &c) ;
 cholmod_finish (&c) ; // finish CHOLMOD

 //delete[] row;
 //delete[] col;
 //delete[] val;
 //cout<<"here3"<<endl;
 return 1;
 }*/
