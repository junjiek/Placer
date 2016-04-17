#include "ldplace.h"
#include <time.h>
#include <iomanip>
#include <iostream>
#include <fstream>
using namespace std;

Ldplace::Ldplace(Block *blk) :
	myPlacement(blk) {
	this->block = blk;
	//  ofstream Ld;
	//	Ld.open("/home/mals/hujy/Ldtime.txt",ios::out|ios::app);
	//cout<<"Ldplace dplace(&block) time:"<<endl;
	clock_t Ldt0 = clock();
	//getregion() from myPlacement
	Rect& chipArea = this->getRegion();
	vector<myRow *>& rows = block->getRows();
	vector<Inst *>& insts = block->getInsts();
	numInst = block-> getNumNodes();
	//rowArea = region, in default
	clock_t Ldt1 = clock();
	rowAreaLeft = chipArea.left();
	rowAreaRight = chipArea.right();
	rowAreaBottom = chipArea.bottom();
	rowAreaTop = chipArea.top();
	clock_t Ldt2 = clock();
	rowHeight = rows[0]->getHeight();
	siteWidth = rows[0]->getSiteWidth();
	clock_t Ldt3 = clock();
	numRow = block->getNumRows();
	clock_t Ldt4 = clock();

	//cout<< rowAreaBottom <<" "<<rowAreaLeft<<" "<< rowAreaTop << " "<< rowAreaRight<< endl;
	initLdplace(rows, insts);
	//cout<<"initLdplace Successfully!"<<endl;
	clock_t Ldt5 = clock();

	/*	Ld<<"getregion from placement time:"<<double(Ldt1-Ldt0)/CLOCKS_PER_SEC<<endl;
	 Ld<<"chipArea time:"<<double(Ldt2-Ldt1)/CLOCKS_PER_SEC<<endl;
	 Ld<<"rows time:"<<double(Ldt3-Ldt2)/CLOCKS_PER_SEC<<endl;
	 Ld<<"getNumRows time:"<<double(Ldt4-Ldt3)/CLOCKS_PER_SEC<<endl;
	 Ld.close();
	 */
}
void Ldplace::initLdplace(vector<myRow *>& rows, vector<Inst*>& insts) {

	// get rows
	initLdprows(rows);
	//cout<<ldpRows[numRow-1]->getRowIndex()<<endl;
	//split the rows into subrows by fixed rect insts
	//numFixed = 0;

	//cout<<"***initLdprows***"<<endl;

	//for debug
	cout<<rows.size()<<" "<<insts.size()<<endl;

	initSubrow(insts, rows);

	// cout<<"***initSubrow***"<<endl;

	//for(long i = 0; i < numRow; i++)numFixed += ldpRows[i]->getNumSubRow();

	//get the instOfSubRow for each mySubRow
	long size = 0;
	initInstofSubrow();

	// cout<<"***initInstOfSubrow***"<<endl;

	//cout<<"total mov:  "<<movValInsts.size()<<endl;

	for (long i = 0; i < numRow; i++) {
		for (long j = 0; j < ldpRows[i]->numSubRow; j++)
			//if(ldpRows[i]->subRows[j]->totalInstWidth > ldpRows[i]->subRows[j]->getWidth())size++;
			size += ldpRows[i]->subRows[j]->instOfSubRow.size();
		//cout<<"i: "<<i<<" "<<size<<endl;
	}
	// cout<< "num Fixed Insts: "<< numFixed << endl;
	// cout<< "num Movable Insts: "<< movValInsts.size() << " "<< size <<endl;
	// cout<< "num InvMovable Insts:" << movInvInsts.size() << endl;
	// cout<<"rowAreaL,R,B,T: "<<rowAreaLeft<<" "<<rowAreaRight<<" "<<rowAreaBottom<<" "<<rowAreaTop<<endl;

}

inline void Ldplace::initLdprows(vector<myRow *>& rows) {
	ldpRows = new LdplaceRow *[numRow];
	mySubRow* Temp;
	for (long i = 0; i < numRow; ++i) {
		ldpRows[i] = new LdplaceRow(i, rows[i]->getCoordY(), 0); //LdplaceRow(long index, long y, long numSubrow)
		Temp = new mySubRow(i, rows[i]->getSiteWidth(), rows[i]->getCoordY(),
				rows[i]->getCoordX(),
				rows[i]->getCoordX() + rows[i]->getWidth());
		//mySubRow(long index, long sWidth, long y, long l, long r)
		ldpRows[i]->subRows.push_back(Temp);
		ldpRows[i]->setNumSubRow(ldpRows[i]->getNumSubRow() + 1);
	}
}

inline void Ldplace::initSubrow(vector<Inst*>& insts, vector<myRow *>& rows) {
	Rect bbox;
	long bottomIndex, topIndex;
	long left, right;
	long jLeft, jRight;
	mySubRow* subRowTemp;
	numFixed = 0;
	try {
		cout << "lalalal: " << numInst << ", " << insts.size() << endl;

		for (unsigned long i = 0; i < numInst; i++) {
			cout << i << endl;
			if ((insts[i]->getStatus() == Moved) && (insts[i]->getHeight()
					> rowHeight))
				cout << "The movable MacroCell is:" << insts[i]->getName()
						<< "  @@@@@@" << endl;

			if (insts[i]->isRect()) {
				//=================hjy20130320================================
				//if(insts[i]->getStatus()==Moved)
				//	      movValInsts.push_back(insts[i]);//original code
				//else movInvInsts.push_back(insts[i]);//original
				//if(insts[i]->getIsValid() == true);//original
				//	if(insts[i]->getStatus() == Fixed )

				if ((insts[i]->getStatus() == Moved) && (insts[i]->getHeight()
						<= rowHeight)) {
					movValInsts.push_back(insts[i]);
				}
				if ((insts[i]->getHeight() > rowHeight)
						|| (insts[i]->getStatus() == Fixed)) {
					//=================================================================

					//		cout<<"insts["<<i<<"] is Fixed"<<endl;

					LdpUtility::getBBox(insts[i], bbox);
					numFixed++;
					//since it's legalized, left and right are in site grid.
					// left = bbox.left(), right = bbox.right();
					left = bbox.left();
					right = bbox.right();
					//cout<< i <<":  "<< left <<" "<< right <<" "<<bbox.bottom()<<" "<< bbox.top()<< endl;
					// get the rows index need to be splited
					bottomIndex = getRowIndexOccupied(bbox.bottom(), true);
					topIndex = getRowIndexOccupied(bbox.top(), false);

					//cout<< i <<":  "<< left <<" "<< right <<" "<<bbox.bottom()<<" "<< bbox.top()<< endl;
					for (long k = bottomIndex; k <= topIndex; ++k) {
						//test
						/*
						 for(vector<mySubRow*>::iterator j = (ldpRows[k]->subRows).begin();
						 j != (ldpRows[k]->subRows).end(); ++j){
						 cout<<"    "<< (*j)->getLeftBoundary() <<" "<< (*j)->getRightBoundary() <<endl;
						 }
						 cout<<"row: "<<k<<endl;
						 */
						for (vector<mySubRow*>::iterator j =
								(ldpRows[k]->subRows).begin(); j
								!= (ldpRows[k]->subRows).end(); ++j) {
							// get the range where the subrows need to be splited
							jLeft = (*j)->getLeftBoundary();
							jRight = (*j)->getRightBoundary();
							if (jRight <= left)
								continue; // haven't reach, continue
							if (jLeft >= right)
								break; // have left, break
							else {
								if (jLeft < left) {
									(*j)->setRightBoundary(left);
									if (jRight > right) {
										ldpRows[k]->setNumSubRow(
												ldpRows[k]->getNumSubRow() + 1);
										subRowTemp = new mySubRow(k,
												rows[k]->getSiteWidth(),
												rows[k]->getCoordY(), right,
												jRight);
										// insert operation guarantees the subRows stored in order, that's why we can break
										// if the insert costs more than breaks save, we need to use push_bach
										j = ldpRows[k]->subRows.insert(j + 1,
												subRowTemp);
										break;
									}
								} else {
									if (jRight <= right) {
										delete *j;
										j = ldpRows[k]->subRows.erase(j);
										ldpRows[k]->setNumSubRow(
												ldpRows[k]->getNumSubRow() - 1);
										// decrease 1, after erase
										--j;
									} else {
										(*j)->setLeftBoundary(right);
										break;
									}
								}
							}
						}
						//test
						/*for(vector<mySubRow*>::iterator j = (ldpRows[k]->subRows).begin();
						 j != (ldpRows[k]->subRows).end(); ++j){
						 cout<<"    "<< (*j)->getLeftBoundary()<<" "<<(*j)->getRightBoundary()<<endl;
						 }
						 cout<<"row: "<<k<<" end "<<endl;
						 */

					}

				}
			}
		}
	} catch (...) {
		cout << "failed initSubRow!" << endl;
	}
}

void Ldplace::initInstofSubrow() {

	Inst* instNow;
	long rowIndex;
	LdplaceRow* rowNow;
	long instLeft;
	long instRight;
	mySubRow* subNow;
	//=========hjytest=============
	int count = 0;
	//	int test=5;
	//========================================

	//	cout<<"The row Height is :"<<rowHeight<<"  #################"<<endl<<endl;

	for (unsigned long in = 0; in < movValInsts.size(); ++in) {

		bool done = false;
		instNow = movValInsts[in];
		rowIndex = (instNow->getCoordY() - rowAreaBottom) / rowHeight;// getRowIndex(instNow);

		rowNow = ldpRows[rowIndex];
		instLeft = instNow->getCoordX();
		instRight = instLeft + instNow->getWidth();

		for (long i = 0; i < rowNow->getNumSubRow(); i++) {
			subNow = rowNow->subRows[i];

			if (instLeft >= subNow->getLeftBoundary() && instRight
					<= subNow->getRightBoundary()) {

				//guarantee insts in instOfSubRow in order

				if (subNow->instOfSubRow.empty() == true) {

					subNow->instOfSubRow.push_back(instNow);
					done = true;
				} else {

					for (list<Inst*>::iterator j = subNow->instOfSubRow.begin(); j
							!= subNow->instOfSubRow.end(); ++j) {

						if ((*j)->getCoordX() < instLeft) {
							if (++j == subNow->instOfSubRow.end()) {

								j = subNow->instOfSubRow.insert(j, instNow);
								done = true;
								break;
							}

							--j;
							continue;

						} else {

							j = subNow->instOfSubRow.insert(j, instNow);
							done = true;
							break;
						}

					}
				}
				subNow->totalInstWidth += instNow->getWidth();
				// if find the instOfSubRow, break
				break;
			}
			//==================hjy121214test========================

		}

		if (!done) {
			cout << "[Error]: Can't put the inst into InstofSubrow!" << endl;
			cout << in << ": " << instLeft << " " << instRight << " "
					<< rowIndex << endl;
			cout << movValInsts[in]->getId() << " "
					<< movValInsts[in]->getName() << endl;
			//   	mySubRow* subNowt = rowNow->subRows[25];
			//      for (list<Inst*>::iterator j = subNowt->instOfSubRow.begin();
			//  					j != subNowt->instOfSubRow.end(); ++j)
			//  	cout<< (*j)->getCoordX() << " "<< (*j)->getCoordX()+(*j)->getWidth();
			//    for (long i = 0; i < rowNow->getNumSubRow(); i++) {
			//  	cout<<i<<": "<<rowNow->subRows[i]->getLeftBoundary()<<" "<<rowNow->subRows[i]->getRightBoundary()<<endl;
			//      }

			//continue;

			break;
		}
	}

	//  cout<<"count:   "<<count<<"***"<<endl;
	//============hjytest==================
	//	cout<<"Hit the can't put inst into InstofSubrow :"<<count<<" times"<<endl;
	//====================================================
}

Ldplace::~Ldplace() {

	for (long i = 0; i < numRow; i++) {
		delete ldpRows[i];
	}
	delete[] ldpRows;
}

void Ldplace::run() {

	//run directly on the entire chip
	//=============hjy20130117====add========================
	/*	setRegion();
	 Area= getRegion();
	 blockX = Area.left();
	 blockY = Area.bottom();
	 blockW = Area.width();
	 blockH = Area.height();

	 insts = plcTopBlock->getInsts();

	 guiFile_DP("LGplace.gnu");//list the original legal gnu
	 */
	//	cout<<"run..."<<endl;
	//add by ZhouQ
	initBins();

	/*int countT[15] = {0};
	 for (long x = 0; x < gridNumX; ++x){
	 for (long y = 0; y < gridNumY; ++y){
	 if (gridAreaAvailable[x][y] == 0){
	 countT[0]++;
	 }
	 else{
	 countT[(int)(gridAreaAvailable[x][y] / 60000) + 1] ++;
	 }
	 }
	 }
	 cout<<"total bin number is "<<gridNumX * gridNumY<<endl;
	 cout<<"0: "<<countT[0]<<endl;
	 for (long i = 0; i < 14; ++i){
	 cout<<i * 60000<<"-"<<(i+1) * 60000<<": "<<countT[i+1]<<endl;
	 }*/

	postOptimize();
	if (visible_DP) {
		string DP = "DP";
		string guiName = string("Place.") + DP + string(".gnu");
		guiFile_DP(guiName.c_str());
	}
	long size = 0;
	for (long i = 0; i < numRow; i++)
		for (long j = 0; j < ldpRows[i]->numSubRow; j++)
			if (ldpRows[i]->subRows[j]->totalInstWidth
					> ldpRows[i]->subRows[j]->getWidth()) {
				cout << "overflow subRow: " << i << " " << j << endl;
			}

	if (gridDensity) {
		for (long i = 0; i < gridNumX; i++) {
			delete[] gridDensity[i];
		}
		delete[] gridDensity;
	}
	if (gridCellArea) {
		for (long i = 0; i < gridNumX; i++) {
			delete[] gridCellArea[i];
		}
		delete[] gridCellArea;
	}
	if (gridAreaAvailable) {
		for (long i = 0; i < gridNumX; i++) {
			delete[] gridAreaAvailable[i];
		}
		delete[] gridAreaAvailable;
	}

	//	cout<<"Detailed time: "<<double(run3-run1)/CLOCKS_PER_SEC<<endl;

	//cout<<"overflow for subRow: "<< size<< endl;
	/*
	 double move = 0.0;
	 for (unsigned long i = 0; i < movValInsts.size(); i++) {
	 myPoint origin;
	 Inst::getOrigin(movValInsts[i], origin);
	 move += (x[i] - origin.coordX()) * (x[i] - origin.coordX()) + (y[i] - origin.coordY())
	 * (y[i] - origin.coordY());
	 }
	 cout << "Total movement from Global = " << move << endl << endl;

	 delete[] x;
	 delete[] y;
	 */
	//	return 1;
}

void Ldplace::dplace() {

	//	cout << "Ldplace..." << endl;

	//showDesign();
	//	unsigned long HPWLGplace = myPlacement::getHPWL(block);

	//	clock_t start = clock();


	postOptimize();
	//	clock_t finishOptimize = clock();
	//	unsigned long HPWLOptimize = myPlacement::getHPWL(block);

	//	clock_t finishAll = clock();
	//	unsigned long HPWLAll = HPWLOptimize;


	//	cout << "Ldplace succeeded!" << left << endl << endl;

	/*	cout << setw(23) << "Global myPlacement " << " HPWL: " << setw(12)
	 << HPWLGplace << endl;
	 cout << setw(23) << "Smooth Critical Path" << " HPWL: " << setw(12)
	 << HPWLSmooth << "   Runtime: " << (double) (finishSmooth - start)
	 / CLOCKS_PER_SEC << endl;
	 cout << setw(23) << "Legalization " << " HPWL: " << setw(12)
	 << HPWLLegalize << "   Runtime: " << (double) (finishLegalize
	 - finishSmooth) / CLOCKS_PER_SEC << endl;
	 cout << setw(23) << "Post Optimization " << " HPWL: " << setw(12)
	 << HPWLOptimize << "   Runtime: " << (double) (finishOptimize
	 - finishLegalize) / CLOCKS_PER_SEC << endl;
	 cout << setw(23) << "Ldplace " << " HPWL: " << setw(12) << HPWLAll
	 << "   Runtime: " << (double) (finishAll - start) / CLOCKS_PER_SEC
	 << endl;
	 */
	cout << endl;
}

/*
 void Ldplace::check() {
 oaString msg, name;
 myPoint origin;
 //check overflow
 cout << "Overflow:" << endl;
 for (size_t i = 0; i < insts.size(); i++) {
 Inst *inst = insts[i];

 if (inst->getPlacementStatus() == oacFixedPlacementStatus
 || inst->getPlacementStatus() == oacLockedPlacementStatus) {
 continue;
 }

 Inst::getOrigin(inst, origin);
 Rect bbox;
 Inst::getBBox(inst, bbox);

 if ((origin.x() - rowAreaLeft) % siteWidth != 0) {
 msg += "unmatch siteX; ";
 }
 if ((origin.y() - rowAreaBottom) % rowHeight != 0) {
 msg += "unmatch siteY; ";
 }
 if (origin.x() < rowAreaLeft) {
 msg += "left overflow; ";
 }
 if (origin.x() + (Int4) bbox.getWidth() > rowAreaRight) {
 msg += "right overflow; ";
 }
 if (origin.y() < rowAreaBottom) {
 msg += "bottom overflow; ";
 }
 if (origin.y() + (Int4) bbox.getHeight() > rowAreaTop) {
 msg += "top overflow; ";
 }
 if (!msg.isEmpty()) {
 inst->getName(DB::getNS(), name);
 cout << name << "(" << origin.x() << ", " << origin.y() << "): "
 << msg << endl;
 msg = "";
 }
 }

 //check overlap
 cout << "check overlap:" << endl;

 for(long i=0; i<numInst-1; i++){
 msg = "";
 for(long j = i+1; j<numInst; j++){
 if(!checkOverlap(insts[i], insts[j])){
 insts[j]->getName(DB::getNS(), name);
 msg += name + ",";
 }
 }
 if(!msg.isEmpty()){
 insts[i]->getName(DB::getNS(), name);
 Inst::getOrigin(insts[i], origin);
 cout<<name<<"("<<origin.x()<<", "<<origin.y()<<"): "<<msg<<endl;
 }
 }


 // Macros
 for (long i = 0; i < numMacro - 1; i++) {
 msg = "";
 for (long j = i + 1; j < numMacro; j++) {
 if (!checkOverlap(macros[i], macros[j])) {
 macros[j]->getName(DB::getNS(), name);
 msg += name + ", ";
 }
 }
 if (!msg.isEmpty()) {
 macros[i]->getName(DB::getNS(), name);
 Inst::getOrigin(macros[i], origin);
 cout << name << "(" << origin.x() << ", " << origin.y() << "): "
 << msg << endl;
 }
 }

 // StdCells
 for (long i = 0; i < numRow; i++) {
 for (size_t j = 0; j < instOfRows[i].size(); j++) {
 Inst *inst = instOfRows[i][j];
 msg = "";
 // with macros
 for (long k = 0; k < numMacro; k++) {
 if (!checkOverlap(inst, macros[k])) {
 macros[k]->getName(DB::getNS(), name);
 msg += name + ", ";
 }
 }

 // with stdCells in the same row
 if (j < instOfRows[i].size() - 1) {
 for (size_t k = j + 1; k < instOfRows[i].size(); k++) {
 if (!checkOverlap(inst, instOfRows[i][k])) {
 instOfRows[i][k]->getName(DB::getNS(), name);
 msg += name + ", ";
 }
 }
 }

 if (!msg.isEmpty()) {
 inst->getName(DB::getNS(), name);
 Inst::getOrigin(inst, origin);
 cout << name << "(" << origin.x() << ", " << origin.y()
 << "): " << msg << endl;
 }
 }
 }

 }

 // with overlap return false, otherwise true
 bool Ldplace::checkOverlap(Inst *a, Inst *b) {
 Rect bboxa, bboxb;
 Inst::getBBox(a, bboxa);
 Inst::getBBox(b, bboxb);

 if (bboxa.left() <= bboxb.left()) {
 if (bboxa.left() + (Int4) bboxa.getWidth() <= bboxb.left()) {
 return true;
 }
 } else {
 if (bboxb.left() + (Int4) bboxb.getWidth() <= bboxa.left()) {
 return true;
 }
 }

 if (bboxa.bottom() <= bboxb.bottom()) {
 if (bboxa.bottom() + (Int4) bboxa.getHeight() <= bboxb.bottom()) {
 return true;
 }
 } else {
 if (bboxb.bottom() + (Int4) bboxb.getHeight() <= bboxa.bottom()) {
 return true;
 }
 }
 return false;
 }

 void Ldplace::showDesign() {
 oaDesign *design = Design::getCurrent();
 oaString cellName;
 design->getCellName(DB::getNS(), cellName);
 cout << "###################################################" << endl;
 cout << "Design info:" << cellName << endl;
 cout << "chipArea = (" << chipArea.left() << ", " << chipArea.bottom()
 << ", " << chipArea.right() << ", " << chipArea.top() << ")"
 << endl;
 cout << "rowArea = (" << rowAreaLeft << ", " << rowAreaBottom << ", "
 << rowAreaRight << ", " << rowAreaTop << ")" << endl;
 cout << "chipArea width = " << chipArea.getWidth() << ", height = "
 << chipArea.getHeight() << endl;
 cout << "numInst = " << numInst << endl;
 cout << "numMovableInst = " << numMovableInst << endl;
 cout << "numFixedInst = " << numFixedInst << endl;
 //cout<<"numRLMPin = "<<numRLMPin<<endl;
 cout << "numMacro = " << numMacro << endl;
 cout << "numStdCell = " << numStdCell << endl;
 cout << "numTerm = " << numTerm << endl;
 cout << "numRow = " << numRow << ", rowHeight = " << rowHeight
 << ", siteWidth = " << siteWidth << endl;

 for (long i = 0; i < numFixedInst; i++) {
 myPoint origin;
 Rect bbox;
 Inst::getOrigin(insts[i], origin);
 Inst::getBBox(insts[i], bbox);
 cout << i << ": " << origin.x() << "  " << origin.y() << "  "
 << bbox.getWidth() << "  " << bbox.getHeight() << endl;
 }

 cout << "###################################################" << endl;
 }

 bool Ldplace::isRLMPin(Inst *inst) {
 oaString name;
 inst->getName(DB::getNS(), name);
 const char *nameChar = String::getBuffer(name);
 if (nameChar[0] == 'p') {
 return true;
 }
 return false;
 }

 bool Ldplace::isMacro(Inst *inst) {
 Rect bbox;
 Inst::getBBox(inst, bbox);
 if ((long) bbox.getHeight() > rowHeight) {
 return true;
 }
 return false;
 }

 void Ldplace::insert(Inst *inst, long rowIndex) {

 if (instOfRow[rowIndex].empty()) {
 instOfRow[rowIndex].push_back(inst);
 return;
 }

 long xCoord = getXCoord(inst);
 for (vector<Inst*>::iterator it = instOfRow[rowIndex].begin(); it
 != instOfRow[rowIndex].end(); it++) {
 long xTemp = getXCoord(*it);
 if (xCoord < xTemp) {
 instOfRow[rowIndex].insert(it, inst);
 return;
 }
 }
 instOfRow[rowIndex].push_back(inst);

 }

 // for std cells
 void Ldplace::getSite(Inst *inst, long &rowIndex, long &siteIndex) {
 if (isMacro(inst)) {
 return;
 }
 myPoint origin, center;
 Rect bbox;
 Inst::getOrigin(inst, origin);
 Inst::getBBox(inst, bbox);
 bbox.getCenter(center);

 rowIndex = (long) ((center.y() - chipArea.bottom()) / rowHeight);
 siteIndex = (long) ((origin.x() - chipArea.left()) / siteWidth);
 }

 // for macro cells
 void Ldplace::getSite(Inst *inst, long &rowIndexBegin, long &rowIndexEnd,
 long &siteIndexBegin, long &siteIndexEnd) {
 if (!isMacro(inst)) {
 return;
 }

 Rect bbox;
 Inst::getBBox(inst, bbox);

 rowIndexBegin = (long) ((bbox.bottom() - chipArea.bottom()) / rowHeight);
 long temp = (bbox.getHeight() % rowHeight == 0) ? 0 : 1;
 rowIndexEnd = rowIndexBegin + (long) (bbox.getHeight() / rowHeight) + temp;

 siteIndexBegin = (long) ((bbox.left() - chipArea.left()) / siteWidth);
 temp = (bbox.getWidth() % siteWidth == 0) ? 0 : 1;
 siteIndexEnd = siteIndexBegin + (long) (bbox.getWidth() / siteWidth) + temp;
 }
 */
//used
inline long Ldplace::getRowIndex(Inst *inst) {
	Rect instBBox;
	myPoint center;
	LdpUtility::getBBox(inst, instBBox);
	return (long) ((instBBox.coordCenter(kY) - rowAreaBottom) / rowHeight);
}

/*
 *  return rowIndex and get the subRowIndex the inst belong to
 */
/*
 long Ldplace::getRowIndex(Inst *inst, size_t &subRowIndex) {
 Rect instBBox, rowBBox;
 myPoint center;

 Inst::getBBox(inst, instBBox);
 long rowIndex = (long) ((instBBox.coordCenter(kY) - rowAreaBottom) / rowHeight);

 for (long i = 0; i < ldpRows[rowIndex]->numSubRow; i++) {
 mySubRow *subRow = ldpRows[rowIndex]->subRows[i];
 if (center.x() >= subRow->left && center.y() <= subRow->right) {
 subRowIndex = i;
 break;
 }
 }

 return rowIndex;
 }
 */
//used
inline long Ldplace::getRowIndexOccupied(long y, bool bottom) {
	if (y < rowAreaBottom) {
		y = rowAreaBottom;
	} else if (y > rowAreaTop) {
		y = rowAreaTop;
	}
	long index = (long) ((y - rowAreaBottom) / rowHeight);
	if (!bottom && (y - rowAreaBottom) % rowHeight == 0) {
		--index;
	}
	return index;
}

/*
 inline mySubRow * Ldplace::getSubRow(Inst*  inst) {
 long rowIndex = getRowIndex(inst);
 LdplaceRow *row = ldpRows[rowIndex];
 long left = inst->getCoordX();
 long right = left + inst->getWidth();
 for (long i = 0; i < row->getNumSubRow(); i++) {
 // to be corrected
 if (left >= row->subRows[i]->getLeftBoundary() && right
 <= row->subRows[i]->getRightBoundary()) {
 return row->subRows[i];
 }
 }
 return NULL;
 }
 */
/*
 void Ldplace::getInstOfRows() {
 for (long i = 0; i < numRow; i++) {
 long numRowInst = 0;
 for (long j = 0; j < ldpRows[i]->getNumSubRow(); j++) {
 numRowInst += ldpRows[i]->subRows[j]->getNumInst();
 }
 instOfRows[i].clear();
 instOfRows[i].reserve(numRowInst);
 for (long j = 0; j < ldpRows[i]->getNumSubRow(); j++) {
 instOfRows[i].insert(instOfRows[i].end(),
 ldpRows[i]->subRows[j]->instOfSubRow.begin(),
 ldpRows[i]->subRows[j]->instOfSubRow.end());
 }
 }
 }

 void Ldplace::checkInstNum() {
 long num = 0;
 for (long i = 0; i < numRow; i++) {
 num += ldpRows[i]->subRows[0]->instOfSubRow.size();
 }
 cout << "inst num in ldpRows = " << num << endl;
 }

 bool Ldplace::isFixedInst(Inst *inst) {
 bool fixed;
 switch (inst->getPlacementStatus()) {
 case oacFixedPlacementStatus:
 case oacLockedPlacementStatus:
 fixed = true;
 break;
 default:
 fixed = false;
 }

 return fixed;
 }
 */
///=================hjy20130117============add==================
void Ldplace::guiFile_DP(const char *fname) {
	ofstream gpFile(fname);
	string cell = string(fname) + "_cell";
	string obstacle = string(fname) + "_fix";
	gpFile << "set title \" ------Placement Result------ \"" << endl;
	gpFile << "set xrange [" << region.left() << ":" << region.right() << "]"
			<< endl;
	gpFile << "set yrange [" << region.bottom() << ":" << region.top() << "]"
			<< endl;
	/*gpFile << "set xtics [" << blockX << ","
	 << blockX + blockW << "]" << endl;
	 gpFile << "set ytics [" << blockY << ":"
	 << blockY + blockH << "]" << endl*/;
	gpFile << "plot \"" << cell << "\" with steps lt 3, " << "\"" << obstacle
			<< "\" with steps lt -1 " << endl;

	const char* fname_cell = cell.c_str();
	const char* fname_obstacle = obstacle.c_str();
	ofstream cFile(fname_cell);
	ofstream fixFile(fname_obstacle);
	vector<Inst*> &insts = block->getInsts();
	gpFile << endl << endl;
	unsigned j;
	string name;
	for (j = 0; j < insts.size(); j++) {
		if (insts[j]->getStatus() == Moved) {
			cFile << "# Cell Name " << insts[j] -> getName() << endl;

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
		} else if (insts[j]->getStatus() == Fixed) {
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
	cFile.close();
	fixFile.close();
	gpFile.close();
}

//zhouq2013724
void Ldplace::guiRectGroup(const char* fname, vector<Rect>& rects) {
	ofstream gpFile(fname);
	ifstream inPlName("instName.tmp");
	string cell = string(fname) + "_cell";
	string obstacle = string(fname) + "_fix";
	string rect = string(fname) + "_region";
	gpFile << "set title \" ------Placement Result------ \"" << endl;
	gpFile << "set xrange [" << region.left() << ":" << region.right() << "]"
			<< endl;
	gpFile << "set yrange [" << region.bottom() << ":" << region.top() << "]"
			<< endl;
	/*gpFile << "set xtics [" << blockX << ","
	 << blockX + blockW << "]" << endl;
	 gpFile << "set ytics [" << blockY << ":"
	 << blockY + blockH << "]" << endl*/;
	gpFile << "plot \"" << cell << "\" with steps lt 3, " << "\"" << obstacle
			<< "\" with steps lt -1, " << "\"" << rect << "\" with steps lt 1"
			<< endl;

	const char* fname_cell = cell.c_str();
	const char* fname_obstacle = obstacle.c_str();
	const char* fname_rect = rect.c_str();
	ofstream cFile(fname_cell);
	ofstream fixFile(fname_obstacle);
	ofstream rectFile(fname_rect);
	vector<Inst*> &insts = block->getInsts();
	gpFile << endl << endl;
	unsigned j;
	string name;
	for (j = 0; j < insts.size(); j++) {
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
		} else if (insts[j]->getStatus() == Fixed) {
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

	for (unsigned long i = 0; i < rects.size(); ++i) {
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

