//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <block.cpp>
//
// Intrpret the bookshelf files 
//
// Author: Wang Sifei
// History: 2012/02/12 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************

#include "block.h"
#include <algorithm>
#include<fstream>

bool lessIndex(InstTerm a, InstTerm b) {
	return (a.getIndexInst() < b.getIndexInst());
}

void Block::saveOA(oaBlock *oaBlk, Block* block) {
	//coordRestore();
	int i = 0;
	oaCollection<oaInst, oaBlock> instColl(oaBlk->getInsts());
	oaIter<oaInst> iIter(instColl);
	vector<Inst *> Insts = block->getInsts();
	while (oaInst* inst = iIter.getNext()) {
		oaPoint instPoint;
		switch (Insts[i]->getOrient()) {
		case kR0:
			instPoint.set((long) Insts[i]->getCoordX(),
					(long) Insts[i]->getCoordY());
			inst->setOrient(oacR0);
			break;
		case kR90:
			inst->setOrient(oacR90);
			cout << "[ERROR]: kR90!!!" << endl;
			break;
		case kR180:
			inst->setOrient(oacR180);
			instPoint.set(
					(long) (Insts[i]->getCoordX() + Insts[i]->getWidth()),
					(long) (Insts[i]->getCoordY() + Insts[i]->getHeight()));
			break;
		case kR270:
			inst->setOrient(oacR270);
			cout << "[ERROR]: kR270!!!" << endl;
			break;
		case kMY:
			inst->setOrient(oacMY);
			instPoint.set(
					(long) (Insts[i]->getCoordX() + Insts[i]->getWidth()),
					(long) Insts[i]->getCoordY());
			break;
		case kMYR90:
			inst->setOrient(oacMYR90);
			cout << "[ERROR]: kMYR90!!!" << endl;
			break;
		case kMX:
			inst->setOrient(oacMX);
			instPoint.set((long) Insts[i]->getCoordX(),
					(long) (Insts[i]->getCoordY() + Insts[i]->getHeight()));
			break;
		case kMXR90:
			inst->setOrient(oacMXR90);
			cout << "[ERROR]: kMXR90!!!" << endl;
			break;
		default:
			cout << "[ERROR]: unknown orient!!!" << endl;
		}
		inst->setPlacementStatus(oacPlacedPlacementStatus);
		inst->setOrigin(instPoint);
		i++;
	}
	cout << "saving complete" << endl;
	//coordShifting();
}

void Block::parseOA(oaBlock *oaBlk) {
	//==========================For Inst Info===============================
	oaCollection<oaInst, oaBlock> instColl(oaBlk->getInsts());

	//TODO
	oaIter<oaTerm> pIter(oaBlk->getTerms());
	long c = 0;
	while (oaTerm* curTerm = pIter.getNext()) {
		/*InstTerm tempPin;
		 oaString tname;
		 curTerm->getName(DB::getNS(), tname);
		 string ttname(tname);
		 oaPoint tpoint;

		 Term::getOrigin(curTerm, tpoint);
		 // cout<<ttname<<" "<<tpoint.x()<<" "<<tpoint.y()<<endl;

		 tempPin.type = Other;
		 strcpy(tempPin.name, ttname.c_str());
		 tempPin.xOffset = 0;
		 tempPin.yOffset = 0;
		 //tempPin.indexNode = InstId;
		 // tempPin.indexNet = NetId;

		 Inst* tempIO = new Inst();
		 //tempIO->index = InstId;
		 tempIO->isRectangular = true;
		 strcpy(tempIO->name, ttname.c_str());

		 tempIO->xCoord = tpoint.x() * bloat;
		 tempIO->yCoord = tpoint.y() * bloat;
		 tempIO->height = 1;
		 tempIO->width = 1;
		 tempIO->origWidth = 1;
		 tempIO->status = FixNI;
		 tempIO->orient = kR0;
		 tempIO->numPins = 1;
		 tempIO->pins.push_back(tempPin);

		 cout<<tempIO->getName()<<" "<<tempIO->xCoord<<" "<<tempIO->yCoord<<endl;
		 long x;
		 cin>>x;

		 nodes.push_back(tempIO);*/

		c++;
	}

	//cout<<"c = "<<c<<endl;*/

	oaIter<oaInst> iIter(instColl);

	int InstId = 0;
	int NetId = 0;
	int RowId = 0;
	numTotalMoved = 0;
	numTotalFixed = 0;
	numTotalNodes = instColl.getCount();
	numTotalTerminals = 0;
	nodes.clear();
	while (oaInst *inst = iIter.getNext()) {
		//oaPoint point;
		//inst->getOrigin(point);
		Inst * tempNodeL = new Inst();
		switch (InstOA::getOrient(inst)) {
		case oacR0:
			tempNodeL->orient = kR0;
			break;
		case oacR90:
			tempNodeL->orient = kR90;
			break;
		case oacR180:
			tempNodeL->orient = kR180;
			break;
		case oacR270:
			tempNodeL->orient = kR270;
			break;
		case oacMY:
			tempNodeL->orient = kMY;
			break;
		case oacMYR90:
			tempNodeL->orient = kMYR90;
			break;
		case oacMX:
			tempNodeL->orient = kMX;
			break;
		case oacMXR90:
			tempNodeL->orient = kMXR90;
			break;
		}
		oaBox bbox;
		//InstOA::getMasterBoundary(inst,bbox);
		InstOA::getBBox(inst, bbox);

		oaString instName;
		inst->getName(DB::getNS(), instName);
		string tempName(instName);
		strcpy(tempNodeL->name, tempName.c_str());
		tempNodeL->index = InstId;
		tempNodeL->isRectangular = true;
		tempNodeL->width = (long) (bbox.getWidth() * bloat);
		tempNodeL->height = (long) (bbox.getHeight() * bloat);
		tempNodeL->origWidth = ((bbox.getWidth()) * bloat);
		tempNodeL->origHeight = ((bbox.getHeight()) * bloat);

		tempNodeL->numPins = 0;
		if ((inst->getPlacementStatus() != oacFixedPlacementStatus)
				&& (inst->getPlacementStatus() != oacLockedPlacementStatus)) {
			tempNodeL->status = Moved;
			numTotalMoved++;
			tempNodeL->origCoordX = ((bbox.left()) * bloat);
			tempNodeL->origCoordY = ((bbox.bottom()) * bloat);
			tempNodeL->xCoord = tempNodeL->getOrigCoordX();
			tempNodeL->yCoord = tempNodeL->getOrigCoordY();
		} else {
			tempNodeL->status = Fixed;
			tempNodeL->origCoordX = ((bbox.left()) * bloat);
			tempNodeL->origCoordY = ((bbox.bottom()) * bloat);
			tempNodeL->xCoord = (tempNodeL->getOrigCoordX());
			tempNodeL->yCoord = (tempNodeL->getOrigCoordY());
			numTotalFixed++;
			numTotalTerminals++;
		}
		tempNodeL->pins.clear();
		nodes.push_back(tempNodeL);
		InstId++;
		//		cout<<"bbox: "<<bbox.getWidth()<<"  "<<bbox.getHeight()<<endl;
		//		cout<<"tempNodeL: "<<tempNodeL->getWidth()<<"   "<<tempNodeL->getHeight()<<endl;
		//		cout<<"point: "<<point.x()<<" "<<point.y()<<endl;
		//		cout<<"Coord: "<<tempNodeL->getCoordX()<<" "<<tempNodeL->getCoordY()<<endl;

	}

	if (nodeMapIndex.begin() != nodeMapIndex.end()) {
		nodeMapIndex.erase(nodeMapIndex.begin(), nodeMapIndex.end());
	}
	for (long i = 0; i < (long) nodes.size(); i++) {
		pair<map<const char*, long, ltstr>::iterator, bool> valid;
		valid = nodeMapIndex.insert(make_pair(nodes[i]->name, nodes[i]->index));
		if (!valid.second) {
			cout << "[ERROR]: there are this key in the map" << endl;
			cout << "nodes' name:" << nodes[i]->name << " " << "nodes index: "
					<< nodes[i]->index << "***" << endl;
			//return;
		}
	}
	//checkNodeMapIndex();
	cout << "numTotalMoved: " << numTotalMoved << endl;
	cout << "numTotalFixed: " << numTotalFixed << endl;
	cout << "nodes.size()=" << nodes.size() << endl;

	//=================================For Net and Pin Info=================================
	oaIter<oaNet> nIter(oaBlk->getNets());
	numTotalNets = oaBlk->getNets().getCount();
	numTotalPins = 0;
	long numGlobalNets = 0;
	int curNumTotalPins = 0;
	nets.clear();
	while (oaNet *net = nIter.getNext()) {
		if (net->isGlobal()) {
			numGlobalNets++;
			continue;
		}
		curNumTotalPins = net->getInstTerms().getCount()
				+ net->getTerms().getCount();
		//cout<<"///////////////////////////////////////////"<<endl;
		//cout<<"curNumTotalPins: "<<curNumTotalPins<<endl;
		numTotalPins = numTotalPins + curNumTotalPins;
		myNet *tempNetL = new myNet();
		oaString netName;
		net->getName(DB::getNS(), netName);
		string tempNetName(netName);
		strcpy(tempNetL->name, tempNetName.c_str());

		tempNetL->index = NetId;
		tempNetL->numPins = 0;
		//int count=0;
		oaIter<oaInstTerm> pIter(net->getInstTerms());
		oaIter<oaTerm> tIter(net->getTerms());
		while (oaTerm* curTerm = tIter.getNext()) {
			InstTerm tempPin;
			oaString tname;
			curTerm->getName(DB::getNS(), tname);
			string ttname(tname);
			oaPoint tpoint;

			Term::getOrigin(curTerm, tpoint);
			// cout<<ttname<<" "<<tpoint.x()<<" "<<tpoint.y()<<endl;

			tempPin.type = Other;
			strcpy(tempPin.name, ttname.c_str());
			tempPin.xOffset = 0;
			tempPin.yOffset = 0;
			tempPin.indexNode = InstId;
			tempPin.indexNet = NetId;

			Inst* tempIO = new Inst();
			tempIO->index = InstId;
			tempIO->isRectangular = true;
			strcpy(tempIO->name, ttname.c_str());

			tempIO->xCoord = tpoint.x() * bloat;
			tempIO->yCoord = tpoint.y() * bloat;
			tempIO->height = 1;
			tempIO->width = 1;
			tempIO->origWidth = 1;
			tempIO->origHeight = 1;
			tempIO->status = FixNI;
			tempIO->orient = kR0;
			tempIO->numPins = 1;
			tempIO->pins.push_back(tempPin);
			nodes.push_back(tempIO);

			InstId++;

			tempNetL->pins.push_back(tempPin);
			tempNetL->numPins++;

			numTotalFixed++;
			numTotalTerminals++;
			numTotalNodes++;
		}

		while (oaInstTerm *curPin = pIter.getNext()) {
			oaInst *ocell = curPin->getInst();
			InstTerm tempPin;
			oaString curInstName;
			ocell->getName(DB::getNS(), curInstName);
			string tempInstName(curInstName);

			oaTerm *tempTerm = curPin->getTerm();
			oaTermType tempType = tempTerm->getTermType();
			if (tempType == oacInputTermType) {
				tempPin.type = Input;
			} else if (tempType == oacOutputTermType) {
				tempPin.type = Output;
			} else if (tempType == oacInputOutputTermType) {
				tempPin.type = INOUT;
			} else {
				tempPin.type = Other;
			}

			//ocell->getMaster()->getDesign()->getTopBlock()->getBlock()->getTerms()

			//			oaString curInstName;
			//			ocell->getName(DB::getNS(), curInstName);
			//			string tempInstName(curInstName);

			oaString curPinName;
			tempTerm->getName(DB::getNS(), curPinName);
			string tempPinName(curPinName);
			strcpy(tempPin.name, tempPinName.c_str());

			oaPoint pointPin;
			oaPoint pointInst;
			oaBox bbox;
			int termboxW = 0;
			int termboxH = 0;
			int cellboxW = 0;
			int cellboxH = 0;
			if ((ocell->getPlacementStatus() != oacFixedPlacementStatus)
					&& (ocell->getPlacementStatus() != oacLockedPlacementStatus)) {
				Term::getBBox(tempTerm, bbox);
				termboxW = bbox.getWidth();
				termboxH = bbox.getHeight();
				oaBox cellBox;
				InstOA::getBBox(ocell, cellBox);
				InstOA::getOrigin(ocell, pointInst);
				Term::getOrigin(curPin, pointPin);
				//Term::getOrigin(tempTerm,pointPin);
				cellboxW = cellBox.getWidth();
				cellboxH = cellBox.getHeight();

				//cout<<"origin "<<curInstName<<" "<<curPinName<<" "<<pointPin.x() / 2000.0<<" "<<pointPin.y() / 2000.0<<endl;
				//for debug
				/*oaIter<oaPin> pins = curPin->getTerm()->getPins();
				 while (oaPin* pin = pins.getNext()){
				 oaIter<oaPinFig> pinfigs = pin->getFigs();
				 while (oaPinFig* pinfig = pinfigs.getNext()){
				 oaBox bbox1;
				 pinfig->getBBox(bbox1);

				 cout<<"\t"<<(cellBox.left() + bbox1.left()) / 2000.0<<" "<<(cellBox.left() + bbox1.right()) / 2000.0<<" "<<(cellBox.bottom() + bbox1.bottom()) / 2000.0<<" "<<(cellBox.bottom() + bbox1.top()) / 2000.0<<endl;
				 }
				 }*/

				//tempPin.xOffset=(long)(((pointPin.x())+(termboxW>>1)-(cellboxW>>1)-(pointInst.x()))*bloat);
				//tempPin.yOffset=(long)(((pointPin.y())+(termboxH>>1)-(cellboxH>>1)-(pointInst.y()))*bloat);

				long offsetX = (long) (((pointPin.x()) - (cellboxW >> 1)
						- (pointInst.x())) * bloat);
				long offsetY = (long) (((pointPin.y()) - (cellboxH >> 1)
						- (pointInst.y())) * bloat);
				switch (ocell->getOrient()) {
				case oacR0:
					tempPin.xOffset = offsetX;
					tempPin.yOffset = offsetY;
					break;
				case oacR90:
					cout << "[WARNING]: uncommon orientation "
							<< ocell->getOrient() << endl;
					break;
				case oacR180:
					tempPin.xOffset = -offsetX;
					tempPin.yOffset = -offsetY;
					break;
				case oacR270:
					cout << "[WARNING]: uncommon orientation "
							<< ocell->getOrient() << endl;
					break;
				case oacMY:
					tempPin.xOffset = -offsetX;
					tempPin.yOffset = offsetY;
					break;
				case oacMYR90:
					cout << "[WARNING]: uncommon orientation "
							<< ocell->getOrient() << endl;
					break;
				case oacMX:
					tempPin.xOffset = offsetX;
					tempPin.yOffset = -offsetY;
					break;
				case oacMXR90:
					cout << "[WARNING]: uncommon orientation "
							<< ocell->getOrient() << endl;
					break;
				default:
					cout << "unknown orientation!!!! " << ocell->getOrient()
							<< endl;
				}
				//cout<<"\toffset "<<(cellBox.left() + cellboxW / 2) / 2000.0<<" "<<(cellBox.bottom() + cellboxH / 2) / 2000.0<<" "<<tempPin.xOffset / 2000.0<<" "<<tempPin.yOffset / 2000.0<<endl;
			} else {
				Term::getBBox(tempTerm, bbox);
				termboxW = bbox.getWidth();
				termboxH = bbox.getHeight();
				oaBox cellBox;
				InstOA::getBBox(ocell, cellBox);
				InstOA::getOrigin(ocell, pointInst);
				Term::getOrigin(curPin, pointPin);
				cellboxW = cellBox.getWidth();
				cellboxH = cellBox.getHeight();
				tempPin.xOffset = (long) (((pointPin.x()) + (termboxW >> 1)
						- (cellboxW >> 1) - (pointInst.x())) * bloat);
				tempPin.yOffset = (long) (((pointPin.y()) + (termboxH >> 1)
						- (cellboxH >> 1) - (pointInst.y())) * bloat);
			}
			long thisIndex = nodeMapIndex[tempInstName.c_str()];
			tempPin.indexNet = NetId;
			tempPin.indexNode = thisIndex;
			nodes[thisIndex]->pins.push_back(tempPin);
			nodes[thisIndex]->numPins++;
			tempNetL->pins.push_back(tempPin);
			tempNetL->numPins++;
		}
		nets.push_back(tempNetL);
		++NetId;
	}

	numTotalNets -= numGlobalNets;

	if (netMapIndex->begin() != netMapIndex->end()) {
		cout << "before netMapIndex.size()= " << netMapIndex->size() << endl;
		netMapIndex->clear();
	}
	for (long i = 0; i < (long) nets.size(); i++) {
		pair<map<const char*, long, ltstr>::iterator, bool> valid;
		valid = netMapIndex->insert(make_pair(nets[i]->name, nets[i]->index));
		if (!valid.second) {
			cout << "[ERROR]: there are this net in the map " << endl;
			cout << "NET#" << i << ": name = " << nets[i]->getName()
					<< ", index = " << nets[i]->getIndex() << endl;
			return;
		}
	}
	//checkNetMapIndex();
	cout << "numTotalPins: " << numTotalPins << endl;
	cout << "numTotalNets: " << numTotalNets << endl;
	cout << "numGlobalNets: " << numGlobalNets << endl;
	cout << "numTotalTerminals: " << numTotalTerminals << endl;
	//for debug
	/*long count[10] = {0};
	 long p;
	 long step = 30;
	 for (long i = 0; i < nets.size(); ++i){
	 p = nets[i]->getNumTerms() / step;
	 if (p > 9){
	 cout<<"!!!"<<nets[i]->getNumTerms()<<endl;
	 count[9]++;
	 }
	 else{
	 count[p]++;
	 }
	 }
	 for (long i = 0; i < 10; ++i){
	 cout<<i*step<<"-"<<(i+1)*step<<": "<<count[i]<<endl;
	 }*/

	//===============================For Row Info========================================
	oaIter<oaRow> rowIt(oaBlk->getRows());
	int RowNum = oaBlk->getRows().getCount();
	string sdNamePre("sd");
	while (oaRow *CurRow = rowIt.getNext()) {
		myRow *row = new myRow();
		oaBox rowbox;
		CurRow->getBBox(rowbox);
		oaSiteDef* siteDef = CurRow->getSiteDef();

		switch (CurRow->getOrient()) {
		case oacR0:
			row->rowOrient = kR0;
			break;
		case oacR90:
			row->rowOrient = kR90;
			break;
		case oacR180:
			row->rowOrient = kR180;
			break;
		case oacR270:
			row->rowOrient = kR270;
			break;
		case oacMY:
			row->rowOrient = kMY;
			break;
		case oacMYR90:
			row->rowOrient = kMYR90;
			break;
		case oacMX:
			row->rowOrient = kMX;
			break;
		case oacMXR90:
			row->rowOrient = kMXR90;
			break;
		}

		switch (CurRow->getSiteOrient()) {
		case oacR0:
			row->siteOrient = kR0;
			break;
		case oacR90:
			row->siteOrient = kR90;
			break;
		case oacR180:
			row->siteOrient = kR180;
			break;
		case oacR270:
			row->siteOrient = kR270;
			break;
		case oacMY:
			row->siteOrient = kMY;
			break;
		case oacMYR90:
			row->siteOrient = kMYR90;
			break;
		case oacMX:
			row->siteOrient = kMX;
			break;
		case oacMXR90:
			row->siteOrient = kMXR90;
			break;
		}

		oaPoint point;
		CurRow->getOrigin(point);
		row->xCoord = (long) (((point.x()) * bloat));
		row->yCoord = (long) (((point.y()) * bloat));
		row->yCoordOrig = (long) (((point.y()) * bloat));
		row->siteWidth = (long) ((siteDef->getWidth()) * bloat);
		row->height = (long) ((rowbox.getHeight()) * bloat);
		row->siteSpacing = row->siteWidth;
		//row->siteSpacing=CurRow->getSiteSpacing()*bloat;
		row->numSites = CurRow->getNumSites();
		row->symmetry = 'Y';
		string tempName;
		stringstream s2d;
		s2d << RowId;
		s2d >> tempName;
		string sdName = sdNamePre + tempName;
		strcpy(row->name, sdName.c_str());
		//cout<<"row->name: "<<row->name<<endl;
		rows.push_back(row);
		RowId++;
		//		cout<<"row Info :"<<row->getCoordX()<<"     "<<row->getCoordY()<<endl;

		//		cout<<"parser Info: rowHeight= "<<rows[0]->getHeight()<<"  siteWidth= "<<rows[0]->getSiteWidth()<<" siteSpacing= "<<rows[0]->getSiteSpace()<<endl;
	}

	//resize cell width to multiples of sites
	long siteSpacing = rows[0]->getSiteSpace();
	//cout<<"!!! site = "<<siteSpacing<<endl;
	for (long i = 0; i < numTotalNodes; ++i) {
		Inst* inst = nodes[i];
		if (inst->getWidth() % siteSpacing != 0 && inst->getStatus() == Moved) {
			cout << inst->getWidth() << " " << siteSpacing << endl;
			long rate = inst->getWidth() / siteSpacing + 1;
			inst->setWidth(rate * siteSpacing);
			cout << rate * siteSpacing << endl;
		}
	}

	//in y-direction, if there is space between rows, create fixed macros to fill spaces
	/*long bottom = rows[0]->getCoordY();
	 for (long i = 1; i < RowNum; ++i){
	 if (rows[i]->getCoordY() - rows[i-1]->getCoordY() != rows[i-1]->height){
	 Inst* inst = new Inst();
	 string temp = string("FixedRowSpace_") + Itostr(i);
	 //cout<<"new fixed block "<<temp<<endl;
	 strcpy(inst->name, temp.c_str());
	 inst->index = InstId;
	 inst->xCoord = rows[i-1]->xCoord;
	 inst->yCoord = rows[i-1]->yCoord + rows[i-1]->height;
	 inst->width = rows[i-1]->getWidth();
	 inst->height = rows[i]->yCoord - rows[i-1]->yCoord - rows[i-1]->height;
	 inst->status = Fixed;
	 inst->orient = kR0;
	 inst->isRectangular = true;
	 nodes.push_back(inst);

	 cout<<"new fixed inst "<<inst->xCoord<<" "<<inst->yCoord<<" "<<inst->width<<" "<<inst->height<<endl;

	 InstId++;
	 numTotalFixed++;
	 numTotalNodes++;
	 }
	 }*/

	//compress rows
	for (long i = 1; i < RowNum; ++i) {
		if (rows[i]->getCoordY() - rows[i - 1]->getCoordY()
				!= rows[i - 1]->height) {
			rows[i]->yCoord = rows[i - 1]->yCoord + rows[i - 1]->height;
		}
		//cout<<"rows "<<i<<", y = "<<rows[i]->getCoordY()<<" "<<rows[i]->getCoordYOrigin()<<endl;
	}

	assert(RowNum==RowId);
	numTotalRows = RowNum;
	//cout<<"id:"<<RowId<<endl;
	cout << "NumTotalRows: " << numTotalRows << endl;
	cout << "SiteWidth: " << rows[0]->getSiteWidth() << "   SiteSpacing: "
			<< rows[0]->getSiteSpace() << "   RowHeight: "
			<< rows[0]->getHeight() << endl;
	//coordOffset = 5 * rows[0]->getHeight();
	//coordOffset = 0;
	//coordShifting();

	clearName();

}

/*void Block::coordShifting(){
 for (long i = 0; i < nodes.size(); ++i){
 nodes[i]->xCoord += coordOffset;
 nodes[i]->yCoord += coordOffset;
 nodes[i]->origCoordX += coordOffset;
 nodes[i]->origCoordY += coordOffset;
 }

 for (long i = 0; i < rows.size(); ++i){
 rows[i]->xCoord += coordOffset;
 rows[i]->yCoord += coordOffset;
 }
 }

 void Block::coordRestore(){
 for (long i = 0; i < nodes.size(); ++i){
 nodes[i]->xCoord -= coordOffset;
 nodes[i]->yCoord -= coordOffset;
 nodes[i]->origCoordX -= coordOffset;
 nodes[i]->origCoordY -= coordOffset;
 }

 for (long i = 0; i < rows.size(); ++i){
 rows[i]->xCoord -= coordOffset;
 rows[i]->yCoord -= coordOffset;
 }
 }
 */

void Block::parseLefDef(string techFile, string cellFile, string defFile) {
	cout << "************ benchmark parser ************" << endl;
	time_t begin = clock();

	//parse tech.lef
	loadTechLef(techFile);

	//parse cells.lef
	loadCellLef(cellFile);

	if (moduleMapIndex.begin() != moduleMapIndex.end()) {
		moduleMapIndex.erase(moduleMapIndex.begin(), moduleMapIndex.end());
	}
	for (long i = 0; i < modules.size(); i++) {
		pair<map<const char*, long, ltstr>::iterator, bool> valid;
		valid = moduleMapIndex.insert(
				make_pair(modules[i]->name, modules[i]->index));
		if (!valid.second) {
			cout << "Error : there are this key int the map" << endl;
			cout << "modules' name:" << modules[i]->name << " "
					<< "modules index:" << modules[i]->index << "***" << endl;
			return;
		}
	}

	if (layerMapIndex.begin() != layerMapIndex.end()) {
		layerMapIndex.erase(layerMapIndex.begin(), layerMapIndex.end());
	}
	for (long i = 0; i < layers.size(); i++) {
		pair<map<const char*, long, ltstr>::iterator, bool> valid;
		valid = layerMapIndex.insert(
				make_pair(layers[i]->name, layers[i]->index));
		if (!valid.second) {
			cout << "Error : there are this key int the map" << endl;
			cout << "layer's name:" << layers[i]->name << " "
					<< "layer's index:" << layers[i]->index << "***" << endl;
			return;
		}
	}

	//parse def file
	loadDef(defFile);

	cout << "There are " << modules.size() << " modules." << endl;
	cout << "There are " << nodes.size() << " insts." << endl;
	cout << "There are " << nets.size() << " nets." << endl;
	cout << "There are " << rows.size() << " rows." << endl;

	if (netMapIndex->begin() != netMapIndex->end()) {
		netMapIndex->clear();
	}
	for (unsigned long i = 0; i < nets.size(); i++) {
		//netMapIndex[tempN] = nets[i]->index;
		pair<map<const char*, long, ltstr>::iterator, bool> valid;
		valid = netMapIndex->insert(make_pair(nets[i]->name, nets[i]->index));
		if (!valid.second) {
			cout << "Error : there are this key int the map" << endl;
			return;
		}
	}

	if (nonRectMapIndex.begin() != nonRectMapIndex.end()) {
		nonRectMapIndex.erase(nonRectMapIndex.begin(), nonRectMapIndex.end());
	}
	//assert(nonRectMapIndex.begin() ==nonRectMapIndex.end());
	for (unsigned long i = 0; i < nonRectangularNodes.size(); i++) {
		pair<map<const char*, long, ltstr>::iterator, bool> valid;
		valid = nonRectMapIndex.insert(
				make_pair(nonRectangularNodes[i]->name,
						nonRectangularNodes[i]->index));
		if (!valid.second) {
			cout << "Error : there are this key int the map" << endl;
			return;
		}
	}

	//for debug
	//checkNets();

	cout << "parser elapses " << double(clock() - begin) / CLOCKS_PER_SEC
			<< " sec." << endl;
	return;
}

void Block::loadCellLef(string cellFile) {
	cout << "parsing " << cellFile << " ..." << endl;
	ifstream fin;
	fin.open(cellFile.c_str());
	if (!fin) {
		cout << "error!  \"" << cellFile << "\" does not exist!" << endl;
		exit(0);
	}

	string temp = "";
	long index = 1; //modules[0] is coresite

	while (fin >> temp) {
		if (temp != "MACRO") {
			continue;
		} else {
			Module* newModule = new Module();
			newModule->index = index;
			newModule->isRectangular = true;
			fin >> newModule->name;
			fin >> temp;
			while (temp != "END") {
				//cout<<temp<<endl;
				if (temp == "CLASS") {
					fin >> temp; //"CORE"
					if (temp == "CORE") {
						newModule->status = Moved;
					} else if (temp == "BLOCK") {
						newModule->status = Fixed;
					} else {
						cout << "unknown module class!!! " << temp << endl;
					}
					fin >> temp; //;
				} else if (temp == "ORIGIN") {
					getline(fin, temp); //"0 0;"
				} else if (temp == "SIZE") {
					fin >> newModule->width;
					fin >> temp; //"BY"
					fin >> newModule->height;
					fin >> temp; //";"
				} else if (temp == "SYMMETRY") {
					getline(fin, temp); //TODO
				} else if (temp == "SITE") {
					//TODO
					fin >> temp;
					if (temp != "core" && temp != "CoreSite") {
						cout << "unknown module SITE!!! " << temp << endl;
					}
					fin >> temp; //";"

				} else if (temp == "PIN") {
					InstTerm newTerm;
					fin >> newTerm.name;
					fin >> temp;
					//END PINS
					while (temp != "END") {
						if (temp == "DIRECTION") {
							fin >> temp;
							if (temp == "INPUT") {
								newTerm.type = Input;
							} else if (temp == "OUTPUT") {
								newTerm.type = Output;
							} else if (temp == "INOUT") {
								newTerm.type = INOUT;
							} else {
								cout << "unknown pin direction!!! " << temp
										<< endl;
								newTerm.type = Other;
							}
							getline(fin, temp); //" ;"
						} else if (temp == "USE") {
							//TODO
							fin >> temp;
							if (temp != "SIGNAL" && temp != "GROUND" && temp
									!= "POWER") {
								cout << "unknown pin USE!!! " << temp << endl;
							}
							fin >> temp; //" ;"
						} else if (temp == "PORT") {
							fin >> temp;
							while (temp != "END") {
								if (temp == "LAYER") {
									//TODO
									fin >> temp;
									newTerm.layer = getLayerIndex(temp);
									getline(fin, temp); //";"
								} else if (temp == "RECT") {
									fin >> newTerm.xOffset;
									fin >> temp;
									fin >> newTerm.yOffset;
									fin >> temp;
									fin >> temp; //" ;"
								} else if (temp == "#") {
									getline(fin, temp);
								} else {
									cout << "unknown PORT statement!!! "
											<< temp << endl;

								}
								fin >> temp; //first string of next line
							}
						} else if (temp == "TAPERRULE") {
							fin >> temp; //rule name
							fin >> temp; //";"
						} else if (temp == "ANTENNADIFFAREA" || temp == "SHAPE"
								|| temp == "ANTENNAGATEAREA") {
							//TODO
							getline(fin, temp);
						} else {
							cout << "unknown PIN statement!!! " << temp << endl;
						}
						fin >> temp;
					}

					fin >> temp; // pin name

					newModule->pins.push_back(newTerm);
				} else if (temp == "OBS") {
					//TODO
					fin >> temp;
					while (temp != "END") {//"END"
						getline(fin, temp);
						fin >> temp;
					}
				} else if (temp == "PROPERTY" || temp == "EDGETYPE" || temp
						== "\"") {
					//TODO
					getline(fin, temp);
				} else if (temp == "FOREIGN") {
					//TODO
					getline(fin, temp);
				}

				else {
					cout << "unknown MODULE statement!!! " << temp << endl;
					getline(fin, temp);
					//long x;
					//cin >> x;
				}
				fin >> temp; // first string of next line
			}
			newModule->numPins = newModule->pins.size();
			modules.push_back(newModule);
			index++;
		}
	}
	fin.close();
}

void Block::loadTechLef(string techFile) {
	cout << "parsing " << techFile << " ..." << endl;
	ifstream fin;
	fin.open(techFile.c_str());
	if (!fin) {
		cout << "error!  \"" << techFile << "\" does not exist!" << endl;
		exit(0);
	}
	string temp = "";
	long layerIndex = 0;
	while (1) {
		fin >> temp;
		if (temp == "#") {
			getline(fin, temp);
		} else if (temp == "PROPERTYDEFINITIONS") {
			//TODO
			while (1) {
				fin >> temp;
				if (temp == "END") {
					getline(fin, temp);
					break;
				} else {

				}
			}
		} else if (temp == "VIA") {
			//TODO
			fin >> temp; // via name
			while (1) {
				fin >> temp;
				if (temp == "END") {
					getline(fin, temp);
					break;
				} else {

				}
			}
		} else if (temp == "VIARULE") {
			//TODO
			fin >> temp; // via name
			while (1) {
				fin >> temp;
				if (temp == "END") {
					getline(fin, temp);
					break;
				} else {

				}
			}
		} else if (temp == "MACRO") {
			fin >> temp; // macro name
			string mark = temp;
			while (1) {
				if (temp == "END") {
					fin >> temp;
					if (temp == mark) {
						break;
					}
				} else {
					fin >> temp;
				}

			}
		} else if (temp == "LAYER") {
			Layer* newLayer = new Layer();
			fin >> newLayer->name; // layer name
			fin >> temp;
			if (temp == "TYPE") {
				fin >> temp;
				if (temp == "ROUTING") {
					newLayer->type = Routing;
					newLayer->index = layerIndex;
				} else if (temp == "CUT") {
					newLayer->type = Cut;
				} else {
					cout << "unknown layer type!!! " << temp << endl;
					newLayer->type = OtherLayerType;
				}
				getline(fin, temp); //";"
			} else {
				cout << "first statement in LAYER is not TYPE!!! " << temp
						<< endl;
			}
			if (newLayer->type == Routing) {
				while (1) {
					fin >> temp;
					if (temp == "DIRECTION") {
						fin >> temp;
						if (temp == "HORIZONTAL") {
							newLayer->direction = kX;
						} else if (temp == "VERTICAL") {
							newLayer->direction = kY;
						} else {
							cout << "unkown layer direction!!! " << temp
									<< endl;
						}
						getline(fin, temp);//";"
					} else if (temp == "PITCH") {
						fin >> newLayer->pitch;
						getline(fin, temp);//";"
					} else if (temp == "OFFSET") {
						fin >> newLayer->offset;
						getline(fin, temp);//";"
					} else if (temp == "PROPERTY") {
						//TODO
						getline(fin, temp);
					} else if (temp == "WIDTH") {
						fin >> newLayer->width;
						getline(fin, temp);
					} else if (temp == "MAXWIDTH") {
						fin >> newLayer->maxWidth;
						getline(fin, temp);
					} else if (temp == "SPACINGTABLE") {
						//TODO
						while (1) {
							fin >> temp;
							if (temp == ";") {
								break;
							} else {

							}
						}
					} else if (temp == "END") {
						getline(fin, temp);
						break;
					} else {
						//cout<<"unknown statement in LAYER!!! "<<temp<<endl;
					}
				}
				layers.push_back(newLayer);
				layerIndex++;
			} else if (newLayer->type == Cut) {
				//TODO
				while (1) {
					fin >> temp;
					if (temp == "END") {
						getline(fin, temp);
						break;
					} else {

					}
				}
				delete newLayer;
			} else {
				//TODO
				while (1) {
					fin >> temp;
					if (temp == "END") {
						getline(fin, temp);
						break;
					} else {

					}
				}
				delete newLayer;
			}
		} else if (temp == "SITE") {
			//"SITE" core
			Module* siteModule = new Module();
			siteModule->index = 0;
			fin >> siteModule->name;
			assert(siteModule->getName() == "core" || siteModule->getName() == "CoreSite");
			while (1) {
				fin >> temp;
				if (temp == "SIZE") {
					fin >> siteModule->width;
					fin >> temp;//"BY"
					fin >> siteModule->height;

					siteModule->isRectangular = true;
					siteModule->orient = kR0;
					siteModule->numPins = 0;
					modules.push_back(siteModule);
				} else if (temp == "END") {
					//END core
					getline(fin, temp);
					break;
				} else {

				}
			}
		} else if (temp == "END") {
			//"END" LIBRARY
			fin >> temp;
			if (temp == "LIBRARY") {
				break;
			}
		} else {
			//cout<<"unknown lef statement!!! "<<temp<<endl;
		}
	}
	fin.close();

}

void Block::loadDef(string defFile) {
	cout << "parsing " << defFile << " ..." << endl;
	ifstream defIn;
	defIn.open(defFile.c_str());
	if (!defIn) {
		cout << "error!  \"" << defFile << "\" does not exist!" << endl;
		exit(0);
	}

	string temp = "";
	long tempInt = 0;
	string c;
	defIn >> temp; //first string
	while (temp != "END") { //"END" DESIGN
		//cout<<temp<<endl;
		if (temp == ";") {
			//do nothing
		} else if (temp.substr(0, 1) == "#") { //if first char is #, ignore this line
			getline(defIn, temp);
		} else if (temp == "UNITS") {
			defIn >> temp >> temp; // "DISTANCE MICRONS"
			defIn >> bloat;
			getline(defIn, temp);
		} else if (temp == "DIEAREA") {
			//TODO
			getline(defIn, temp);
		} else if (temp == "ROW") {
			myRow* row = new myRow();
			defIn >> row->name;
			defIn >> temp; //module name of site

			Module* siteModule = modules[moduleMapIndex[temp.c_str()]];
			row->siteWidth = siteModule->getWidth() * bloat;
			row->height = siteModule->getHeight() * bloat;

			defIn >> row->xCoord;
			defIn >> row->yCoord;
			row->yCoordOrig = row->yCoord;
			defIn >> temp; //orientation

			if (temp == "N") {
				row->rowOrient = kR0;
				row->siteOrient = kR0;
			} else if (temp == "FS") {
				row->rowOrient = kMX;
				row->siteOrient = kMX;
			} else {
				cout << "unknown row orientation!! " << temp << endl;
			}

			defIn >> temp; //"DO"
			defIn >> row->numSites;
			defIn >> temp >> temp >> temp; //"BY 1 STEP"
			defIn >> row->siteSpacing;
			assert(row->siteSpacing == row->siteWidth);

			rows.push_back(row);

			getline(defIn, temp);
		} else if (temp == "TRACKS") {
			//TODO
			getline(defIn, temp);
		} else if (temp == "VIAS") {
			//TODO
			while (temp != "END VIAS") {
				getline(defIn, temp);
			}
		} else if (temp == "NONDEFAULTRULES") {
			//TODO
			while (temp != "END NONDEFAULTRULES") {
				getline(defIn, temp);
			}
		} else if (temp == "COMPONENTS") {
			defIn >> numTotalNodes;
			defIn >> temp >> temp; // ; -
			long index = 0;
			Inst* newInst = NULL;
			while (1) {
				if (temp == "END") {
					break;
				}
				if (temp == "-") {
					newInst = new Inst();
					newInst->index = index;

					defIn >> newInst->name;
					defIn >> temp; //module name

					Module* instModule = modules[moduleMapIndex[temp.c_str()]];
					newInst->height = instModule->getHeight() * bloat;
					newInst->width = instModule->getWidth() * bloat;
					newInst->origWidth = newInst->width;
					newInst->origHeight = newInst->height;
					newInst->numPins = instModule->getNumInstTerms();
					newInst->orient = kR0;
					newInst->isRectangular = instModule->isRect();
					newInst->status = instModule->getStatus();
					vector<InstTerm> terms = instModule->getInstTerms();
					for (long i = 0; i < terms.size(); ++i) {
						InstTerm newTerm;
						strcpy(newTerm.name, terms[i].name);
						newTerm.indexNet = 0;
						newTerm.indexNode = newInst->index;
						newTerm.type = terms[i].type;
						newTerm.xOffset = terms[i].xOffset * bloat;
						newTerm.yOffset = terms[i].yOffset * bloat;
						newTerm.layer = terms[i].layer;
						newInst->pins.push_back(newTerm);
					}
				} else if (temp == "+") {
					defIn >> temp;
					if (temp == "PLACED" || temp == "FIXED") {
						defIn >> temp; // (
						defIn >> newInst->xCoord;
						newInst->origCoordX = newInst->xCoord;
						defIn >> newInst->yCoord;
						newInst->origCoordY = newInst->yCoord;
						defIn >> temp; // )
						defIn >> temp; // inst orientatio
						if (temp == "N") {
							newInst->orient = kR0;
						} else if (temp == "S") {
							newInst->orient = kR180;
						} else if (temp == "W") {
							newInst->orient = kR90;
						} else if (temp == "E") {
							newInst->orient = kR270;
						} else if (temp == "FN") {
							newInst->orient = kMY;
						} else if (temp == "FS") {
							newInst->orient = kMX;
						} else if (temp == "FW") {
							newInst->orient = kMXR90;
						} else if (temp == "FE") {
							newInst->orient = kMYR90;
						} else {
							cout << "unknown inst orientation!! " << temp
									<< endl;
						}
					} else if (temp == "UNPLACED") {
						newInst->setStatus(Moved);
						newInst->xCoord = 0;
						newInst->yCoord = 0;
						newInst->origCoordX = newInst->xCoord;
						newInst->origCoordY = newInst->yCoord;
					} else if (temp == "SOURCE") {
						//TODO
						defIn >> temp;
					} else {
						cout << "unknown inst status!!! " << temp << endl;
					}
				} else if (temp == ";") {
					assert(newInst != NULL);
					nodes.push_back(newInst);
					index++;
				} else {
					cout << "strange format !!! " << temp << endl;
					cout << "pause" << endl;
					long x;
					cin >> x;

				}
				defIn >> temp; // first string of next line
			}
			assert(numTotalNodes == nodes.size());
			getline(defIn, temp);
		} else if (temp == "PINS") {
			defIn >> temp; // number of pins
			defIn >> temp >> temp; // ; -
			long index = numTotalNodes;

			while (temp != "END") {//"END" PINS
				if (temp == "-") {
					Inst* newInst = new Inst();
					InstTerm term;
					term.xOffset = 0;
					term.yOffset = 0;

					newInst->index = index;
					newInst->numPins = 1;
					newInst->isRectangular = true;
					term.indexNode = index;
					newInst->pins.push_back(term);

					defIn >> newInst->name;
					strcpy(term.name, newInst->name);

					defIn >> temp; // "+"
					defIn >> temp; // "NET"
					assert(temp == "NET");

					defIn >> temp; //TODO net name

					defIn >> temp >> temp; // "+ DIRECTION"

					defIn >> temp; //pin direction
					if (temp == "INPUT") {
						term.type = Input;
					} else if (temp == "OUTPUT") {
						term.type = Output;
					} else if (temp == "INOUT") {
						term.type = INOUT;
					} else {
						cout << "unknown pin direction!!! " << temp << endl;
						term.type = Other;
					}

					defIn >> temp; // "+"
					defIn >> temp; //pin status

					if (temp == "PLACED" || temp == "FIXED") {
						//TODO which status should be set?
						newInst->setStatus(FixNI);

						defIn >> temp; // "("
						defIn >> newInst->xCoord;
						newInst->origCoordX = newInst->xCoord;
						defIn >> newInst->yCoord;
						newInst->origCoordY = newInst->yCoord;
						defIn >> temp; // ")"
						defIn >> temp; //pin orientation
						if (temp == "N") {
							newInst->orient = kR0;
						} else if (temp == "S") {
							newInst->orient = kR180;
						} else if (temp == "W") {
							newInst->orient = kR90;
						} else if (temp == "E") {
							newInst->orient = kR270;
						} else if (temp == "FN") {
							newInst->orient = kMY;
						} else if (temp == "FS") {
							newInst->orient = kMX;
						} else if (temp == "FW") {
							newInst->orient = kMXR90;
						} else if (temp == "FE") {
							newInst->orient = kMYR90;
						} else {
							cout << "unknown inst orientation!! " << temp
									<< endl;
						}
					} else {
						cout << "unknown pin status!!! " << temp << endl;
					}

					defIn >> temp; // "+"
					defIn >> temp; //

					if (temp == "LAYER") {
						long xl, xr, yb, yt;
						defIn >> temp; //TODO layer name
						defIn >> temp; // (
						defIn >> xl >> yb;
						defIn >> temp >> temp; //") ("
						defIn >> xr >> yt;
						defIn >> temp >> temp; //") ;"
						newInst->width = xr - xl;
						newInst->height = yt - yb;
						newInst->origWidth = newInst->width;
						newInst->origHeight = newInst->height;
					}
					nodes.push_back(newInst);
					index++;
					numTotalNodes++;
				} else {
					cout << "strange format !!! " << temp << endl;
					cout << "pause1" << endl;
					long x;
					cin >> x;
				}
				defIn >> temp;
			}

			//build nodeMap
			//cout<<"building node map..."<<endl;
			if (nodeMapIndex.begin() != nodeMapIndex.end()) {
				nodeMapIndex.erase(nodeMapIndex.begin(), nodeMapIndex.end());
			}
			for (unsigned long i = 0; i < nodes.size(); i++) {
				pair<map<const char*, long, ltstr>::iterator, bool> valid;
				valid = nodeMapIndex.insert(
						make_pair(nodes[i]->name, nodes[i]->index));
				if (!valid.second) {
					cout << "Error : there are this key int the map" << endl;
					cout << "nodes' name:" << nodes[i]->name << " "
							<< "nodes index:" << nodes[i]->index << "***"
							<< endl;
					return;
				}
			}

			defIn >> temp; // END "PINS"
		} else if (temp == "BLOCKAGES") {
			//TODO
			while (temp != "END BLOCKAGES") {
				getline(defIn, temp);
			}

		} else if (temp == "SPECIALNETS") {
			//TODO
			defIn >> temp;
			while (1) {
				if (temp == "END") { //"END" SPECIALNETS
					getline(defIn, temp);
					break;
				} else if (temp == "-") {
					defIn >> temp; //net name
					defIn >> temp;
					while (1) {
						//cout<<temp<<endl;
						if (temp == "+") {

						} else if (temp == "USE") {
							defIn >> temp;
						} else if (temp == "RECT") {
							//cout<<"rect!!!"<<endl;
							getline(defIn, temp);
						} else if (temp == ";") {
							break;
						} else if (temp == "NEW" || temp == "ROUTED") {
							long width;
							defIn >> temp;//layer name
							defIn >> width; // width
							if (width != 0) {
								long layerIndex = layerMapIndex[temp.c_str()];
								Rect newRect;
								defIn >> temp >> temp >> temp >> temp; // "+ SHAPE STRIPE ("
								defIn >> newRect.xl >> newRect.yb;
								defIn >> temp >> temp; // ") ("
								defIn >> temp;
								if (temp == "*") {
									//assert(layers[layerIndex]->getDirection() == kY);
									defIn >> newRect.yt;
									newRect.xl -= width / 2;
									newRect.xr = newRect.xl + width;
									getline(defIn, temp);
								} else {
									//assert(layers[layerIndex]->getDirection() == kX);
									newRect.xr = atoi(temp.c_str());
									newRect.yb -= width / 2;
									newRect.yt = newRect.yb + width;
									getline(defIn, temp);
								}
								layers[layerIndex]->blocks.push_back(newRect);
							} else {
								getline(defIn, temp);
							}
						} else {
							getline(defIn, temp);
						}
						defIn >> temp;
					}
				} else {

				}
				defIn >> temp; //first string of next line
			}
		} else if (temp == "NETS") {
			defIn >> numTotalNets;
			defIn >> temp >> temp; //"; -"
			long index = 0;
			myNet* newNet = NULL;
			while (1) {
				if (temp == "END") {//"END" NETS
					defIn >> temp;
					break;
				} else if (temp == "-") {
					newNet = new myNet();
					defIn >> newNet->name;

					newNet->index = index;
				} else if (temp == "(") { // a new term
					defIn >> temp;// inst name or "PIN"
					if (temp == "PIN") {
						defIn >> temp; //pin name
						Inst* instNow = nodes[nodeMapIndex[temp.c_str()]];
						assert(instNow->numPins == 1);
						vector<InstTerm>& terms = instNow->getInstTerms();
						terms[0].indexNet = index;
						newNet->pins.push_back(terms[0]);
					} else {
						Inst* instNow = nodes[nodeMapIndex[temp.c_str()]];

						defIn >> temp;// inst term name
						vector<InstTerm>& terms = instNow->getInstTerms();
						for (long i = 0; i < instNow->getNumInstTerms(); ++i) {
							if (temp == terms[i].getName()) {
								terms[i].indexNet = index;
								newNet->pins.push_back(terms[i]);
								break;
							}
							if (i == instNow->getNumInstTerms() - 1) {
								cout << "[ERROR]: can't find term " << temp
										<< " on inst " << instNow->getName()
										<< endl;
								long x;
								cin >> x;
							}
						}
					}
					defIn >> temp; //")"
				} else if (temp == "+") {
					defIn >> temp;
					if (temp == "NONDEFAULTRULE") {
						defIn >> temp;
						newNet->weight = 5;
					}
				} else if (temp == ";") {
					newNet->numPins = newNet->pins.size();
					sort(newNet->pins.begin(), newNet->pins.end(), lessIndex);
					nets.push_back(newNet);
					index++;
				} else {
					cout << "strange format!!! " << temp << endl;
					cout << "pause2" << endl;
					long x;
					cin >> x;
				}
				defIn >> temp; //first string of next statement
			}
		} else {
			//cout<<"unknown def statement!!! "<<temp<<endl;
			getline(defIn, temp);
		}
		defIn >> temp;
	}
	numTotalRows = rows.size();

	defIn.close();

	//bloat data in layer
	for (long i = 0; i < layers.size(); ++i) {
		layers[i]->maxWidth *= bloat;
		layers[i]->offset *= bloat;
		layers[i]->pitch *= bloat;
		layers[i]->spacing *= bloat;
		layers[i]->width *= bloat;
	}

	insertBlocks();
}

void Block::reloadDef(string defFile) {
	ifstream defIn;
	defIn.open(defFile.c_str());
	if (!defIn) {
		cout << "error!  \"" << defFile << "\" does not exist!" << endl;
		exit(0);
	}
	string temp = "";
	long tempInt = 0;
	string c;
	defIn >> temp; //first string
	while (1) { //"END" DESIGN
		//cout<<temp<<endl;

		if (temp == "END") {
			defIn >> temp;
			if (temp == "DESIGN") {
				break;
			} else {
				getline(defIn, temp);
			}
		} else if (temp == ";") {
			//do nothing
		} else if (temp.substr(0, 1) == "#") { //if first char is #, ignore this line
			getline(defIn, temp);
		} else if (temp == "COMPONENTS") {
			defIn >> temp;
			defIn >> temp >> temp; // ; -
			long index = 0;
			Inst* newInst;

			while (1) {
				if (temp == "END") {
					break;
				}
				if (temp == "-") {
					newInst = nodes[index];

					defIn >> temp;
					defIn >> temp; //module name
				} else if (temp == "+") {
					defIn >> temp;
					if (temp == "PLACED" || temp == "FIXED") {
						defIn >> temp; // (
						defIn >> newInst->xCoord;
						newInst->origCoordX = newInst->xCoord;
						defIn >> newInst->yCoord;
						newInst->origCoordY = newInst->yCoord;
						defIn >> temp; // )
						defIn >> temp; // inst orientatio
					} else if (temp == "SOURCE") {
						//TODO
						defIn >> temp;
					} else {
						cout << "unknown inst status!!! " << temp << endl;
					}
				} else if (temp == ";") {
					assert(newInst != NULL);
					//nodes.push_back(newInst);
					index++;
				} else {
					cout << "strange format !!! " << temp << endl;
					cout << "pause" << endl;
					long x;
					cin >> x;

				}
				defIn >> temp; // first string of next line
			}
			assert(numTotalNodes == nodes.size());
			getline(defIn, temp);
		} else {
			getline(defIn, temp);
		}
		defIn >> temp;
	}
}

void Block::saveDefForRDP(string defFile, string origDefFile) {
	ifstream defIn;
	ofstream defOut;
	long index = -1;

	defIn.open(origDefFile.c_str());
	if (!defIn) {
		cout << "error!  can not open \"" << origDefFile << "\"" << endl;
	}

	defOut.open(defFile.c_str());

	string temp = "";

	while (1) {
		getline(defIn, temp);
		defOut << temp << endl;
		if (temp.substr(0, 4) == "PINS") {
			while (1) {
				defIn >> temp;
				if (temp == "END") {
					getline(defIn, temp);
					defOut << "END PINS" << endl;
					break;
				} else if (temp == "-") {
					defOut << "\t- ";
					getline(defIn, temp);
					defOut << temp << endl;
				} else if (temp == "+") {
					defOut << "\t\t+ ";
					defIn >> temp;
					if (temp == "PLACED") {
						defOut << "FIXED ";
					} else {
						defOut << temp << " ";
					}
					getline(defIn, temp);
					defOut << temp << endl;
				} else {
					cout << "unknown pin statement!!! " << temp << endl;
				}
			}
		} else if (temp.substr(0, 10) == "COMPONENTS") {
			while (1) {
				defIn >> temp;

				if (temp == "-") {
					index++;
					defOut << "\t- ";
					for (long i = 0; i < 2; ++i) { //inst module
						defIn >> temp;
						defOut << temp << " ";

					}
				} else if (temp == "+") {
					defOut << "\t\t+ ";
					defIn >> temp;

					// if the status of current inst is UNPLACED, change it to PLACED
					if (temp == "UNPLACED") {
						defOut << "PLACED ( "
								<< (long) nodes[index]->getCoordX() << " "
								<< (long) nodes[index]->getCoordY() << " ) ";
						if (nodes[index]->getOrient() == kR0) {
							defOut << "N" << " ; ";
						} else if (nodes[index]->getOrient() == kR180) {
							defOut << "S" << " ; ";
						} else if (nodes[index]->getOrient() == kR90) {
							defOut << "W" << " ; ";
						} else if (nodes[index]->getOrient() == kR270) {
							defOut << "E" << " ; ";
						} else if (nodes[index]->getOrient() == kMY) {
							defOut << "FN" << " ; ";
						} else if (nodes[index]->getOrient() == kMX) {
							defOut << "FS" << " ; ";
						} else if (nodes[index]->getOrient() == kMXR90) {
							defOut << "FW" << " ; ";
						} else if (nodes[index]->getOrient() == kMYR90) {
							defOut << "FE" << " ; ";
						} else {
							cout << "unknown inst orientation!!! "
									<< nodes[index]->getOrient() << endl;
						}
						defIn >> temp; //";"
					} else if (temp == "PLACED") {
						if (nodes[index]->getStatus() == Fixed) {
							defOut << "FIXED ( "
									<< (long) nodes[index]->getCoordX() << " "
									<< (long) nodes[index]->getCoordY()
									<< " ) ";
						} else {
							defOut << "PLACED ( "
									<< (long) nodes[index]->getCoordX() << " "
									<< (long) nodes[index]->getCoordY()
									<< " ) ";
						}

						if (nodes[index]->getOrient() == kR0) {
							defOut << "N" << " ; ";
						} else if (nodes[index]->getOrient() == kR180) {
							defOut << "S" << " ; ";
						} else if (nodes[index]->getOrient() == kR90) {
							defOut << "W" << " ; ";
						} else if (nodes[index]->getOrient() == kR270) {
							defOut << "E" << " ; ";
						} else if (nodes[index]->getOrient() == kMY) {
							defOut << "FN" << " ; ";
						} else if (nodes[index]->getOrient() == kMX) {
							defOut << "FS" << " ; ";
						} else if (nodes[index]->getOrient() == kMXR90) {
							defOut << "FW" << " ; ";
						} else if (nodes[index]->getOrient() == kMYR90) {
							defOut << "FE" << " ; ";
						} else {
							cout << "unknown inst orientation!!! "
									<< nodes[index]->getOrient() << endl;
						}
						getline(defIn, temp); //";"
					} else {
						defOut << temp << " ";
						getline(defIn, temp);
						defOut << temp;
					}
				} else if (temp == "END") {
					break;
				} else {
					cout << temp << endl;
				}
				defOut << endl;
			}
			defIn >> temp;
			defOut << "END COMPONENTS" << endl;
		} else if (temp == "END DESIGN") {
			break;
		} else {

		}
	}

	defIn.close();
	defOut.close();
}

void Block::saveDef(string defFile, string origDefFile) {
	ifstream defIn;
	ofstream defOut;
	long index = -1;

	defIn.open(origDefFile.c_str());
	if (!defIn) {
		cout << "error!  can not open \"" << origDefFile << "\"" << endl;
	}

	defOut.open(defFile.c_str());

	string temp = "";

	while (temp.substr(0, 10) != "COMPONENTS") {
		getline(defIn, temp);
		defOut << temp << endl;
	}

	while (1) {
		defIn >> temp;

		if (temp == "-") {
			index++;
			defOut << "\t- ";
			for (long i = 0; i < 2; ++i) { //inst module
				defIn >> temp;
				defOut << temp << " ";

			}
		} else if (temp == "+") {
			defOut << "\t\t+ ";
			defIn >> temp;

			// if the status of current inst is UNPLACED, change it to PLACED
			if (temp == "UNPLACED") {
				defOut << "PLACED ( " << (long) nodes[index]->getCoordX()
						<< " " << (long) nodes[index]->getCoordY() << " ) ";
				if (nodes[index]->getOrient() == kR0) {
					defOut << "N" << " ; ";
				} else if (nodes[index]->getOrient() == kR180) {
					defOut << "S" << " ; ";
				} else if (nodes[index]->getOrient() == kR90) {
					defOut << "W" << " ; ";
				} else if (nodes[index]->getOrient() == kR270) {
					defOut << "E" << " ; ";
				} else if (nodes[index]->getOrient() == kMY) {
					defOut << "FN" << " ; ";
				} else if (nodes[index]->getOrient() == kMX) {
					defOut << "FS" << " ; ";
				} else if (nodes[index]->getOrient() == kMXR90) {
					defOut << "FW" << " ; ";
				} else if (nodes[index]->getOrient() == kMYR90) {
					defOut << "FE" << " ; ";
				} else {
					cout << "unknown inst orientation!!! "
							<< nodes[index]->getOrient() << endl;
				}
				defIn >> temp; //";"
			} else {
				defOut << temp << " ";
				getline(defIn, temp);
				defOut << temp;
			}
		} else if (temp == "END") {
			break;
		} else {
			cout << temp << endl;
		}
		defOut << endl;
	}
	defIn >> temp;
	defOut << "END COMPONENTS" << endl;

	while (temp != "END DESIGN") {
		getline(defIn, temp);
		defOut << temp << endl;
	}
	defIn.close();
	defOut.close();
}

/*void Block::insertBlocks(){
 vector<Rect> blocks = layers[1]->getBlocks();
 for (long i = 0; i < blocks.size(); ++i){
 Inst* newInst = new Inst();
 newInst->index = nodes.size();
 newInst->isRectangular = true;
 //cout<<"jhere"<<endl;
 string s("blockage" + Itostr(newInst->index));
 //cout<<s<<endl;
 strcpy(newInst->name, s.c_str());
 //cout<<"111"<<endl;
 newInst->numPins = 0;
 newInst->orient = kR0;
 newInst->status = Fixed;
 newInst->width = blocks[i].right() - blocks[i].left();
 newInst->origWidth = newInst->width;

 newInst->xCoord = blocks[i].left();
 newInst->yCoord = blocks[i].bottom();
 newInst->height = blocks[i].top() - blocks[i].bottom();
 newInst->origHeight = newInst->height;
 //cout<<newInst->getName()<<" "<<newInst->getCoordX()<<" "<<newInst->getWidth()<<" "<<newInst->getCoordY()<<" "<<newInst->getHeight()<<endl;
 nodes.push_back(newInst);
 numTotalNodes++;
 }
 }*/

void Block::insertBlocks() {
	//for (long k = 1; k < layers.size(); ++k){
	for (long k = 1; k < 2; ++k) {
		vector<Rect> blocks = layers[k]->getBlocks();
		for (long i = 0; i < blocks.size(); ++i) {
			Inst* newInst = new Inst();
			newInst->index = nodes.size();
			newInst->isRectangular = true;
			//cout<<"jhere"<<endl;
			string s("blockage" + Itostr(newInst->index));
			//cout<<s<<endl;
			strcpy(newInst->name, s.c_str());
			//cout<<"111"<<endl;
			newInst->numPins = 0;
			newInst->orient = kR0;
			newInst->status = Fixed;
			newInst->width = blocks[i].right() - blocks[i].left();
			newInst->origWidth = newInst->width;

			newInst->xCoord = blocks[i].left();
			newInst->yCoord = blocks[i].bottom();
			newInst->height = blocks[i].top() - blocks[i].bottom();
			newInst->origHeight = newInst->height;
			//cout<<newInst->getName()<<" "<<newInst->getCoordX()<<" "<<newInst->getWidth()<<" "<<newInst->getCoordY()<<" "<<newInst->getHeight()<<endl;
			nodes.push_back(newInst);
			numTotalNodes++;
		}
	}
}

//add by ZhouQ
void Block::writeName() {
	cout << "now write names" << endl;
	ofstream outPin;
	ofstream outNet;
	ofstream outInst;
	ofstream outRow;
	ofstream outNonRect;

	//cout<<"write myRow name"<<endl;
	outRow.open("rowName.tmp");
	//outRow<<rows.size()<<endl;
	for (long i = 0; i < rows.size(); ++i) {
		outRow << rows[i]->name << endl;
		delete[] rows[i]->name;
	}
	outRow.close();

	//cout<<"write Inst name"<<endl;
	outInst.open("instName.tmp");
	//outInst<<nodes.size()<<endl;
	//cout<<nodes.size()<<endl;
	for (long i = 0; i < nodes.size(); ++i) {
		outInst << nodes[i]->name << endl;
		delete[] nodes[i]->name;
	}
	outInst.close();

	//cout<<"write myNet and Pin name"<<endl;
	outNet.open("netName.tmp");
	outPin.open("pinName.tmp");
	//outNet<<nets.size()<<endl;
	//outPin<<numTotalPins<<endl;
	for (long i = 0; i < nets.size(); ++i) {
		outNet << nets[i]->name << endl;
		for (long j = 0; j < nets[i]->pins.size(); ++j) {
			outPin << nets[i]->pins[j].name << endl;
			delete[] nets[i]->pins[j].name;
		}
		delete[] nets[i]->name;
	}
	outNet.close();
	outPin.close();

	//cout<<"write NonRect name"<<endl;
	outNonRect.open("NonRectName.tmp");
	for (long i = 0; i < nonRectangularNodes.size(); ++i) {
		for (long j = 0; j < nonRectangularNodes[i]->numRectangular; ++j) {
			delete[] nonRectangularNodes[i]->shapes[j].name;
		}
		delete[] nonRectangularNodes[i]->name;
	}
	outNonRect.close();
}

void Block::clearName() {
	for (long i = 0; i < rows.size(); ++i) {
		delete[] rows[i]->name;
	}
	for (long i = 0; i < nodes.size(); ++i) {
		delete[] nodes[i]->name;
	}
	for (long i = 0; i < nets.size(); ++i) {
		for (long j = 0; j < nets[i]->pins.size(); ++j) {
			delete[] nets[i]->pins[j].name;
		}
		delete[] nets[i]->name;
	}
	for (long i = 0; i < nonRectangularNodes.size(); ++i) {
		for (long j = 0; j < nonRectangularNodes[i]->numRectangular; ++j) {
			delete[] nonRectangularNodes[i]->shapes[j].name;
		}
		delete[] nonRectangularNodes[i]->name;
	}
}

void Block::checkNonRectMapIndex() {
	//test has created nonTectangularMapIndex
	cout << "now check the nonTectMapIndex ... " << endl;
	ofstream mapOut;
	mapOut.open("mapNonRectangular.log", ios::out);
	map<const char*, long, ltstr>::iterator iterBegin = nonRectMapIndex.begin();
	iterBegin = nonRectMapIndex.begin();
	while (iterBegin != nonRectMapIndex.end()) {
		mapOut << "name : " << iterBegin->first << "  index : "
				<< iterBegin->second << endl;
		++iterBegin;
	}
	mapOut.close();
	return;
}

void Block::checkNodeMapIndex() {
	//test has created nodeMapIndex
	cout << "now check the nodeMapIndex ... " << endl;
	ofstream mapOut;
	mapOut.open("mapNodes.log", ios::out);
	map<const char*, long, ltstr>::iterator iterBegin = nodeMapIndex.begin();
	iterBegin = nodeMapIndex.begin();
	while (iterBegin != nodeMapIndex.end()) {
		mapOut << "name : " << iterBegin->first << "  index : "
				<< iterBegin->second << endl;
		++iterBegin;
	}
	mapOut.close();
	return;
}

void Block::checkNetMapIndex() {
	//test has created netMapIndex
	cout << "now check the netMapIndex ... " << endl;
	ofstream mapOut;
	mapOut.open("mapNets.log", ios::out);
	map<const char*, long, ltstr>::iterator iterBegin = netMapIndex->begin();
	iterBegin = netMapIndex->begin();
	while (iterBegin != netMapIndex->end()) {
		mapOut << "name : " << iterBegin->first << "  index : "
				<< iterBegin->second << endl;
		++iterBegin;
	}
	mapOut.close();
	return;
}

void Block::checkShapes() {
	ofstream outShapes;
	outShapes.open("shapesCheck.log", ios::out);
	cout << "now checking shapes..." << endl;
	cout << " nonRectangularNodes.size() = " << nonRectangularNodes.size()
			<< endl;

	for (unsigned long i = 0; i < nonRectangularNodes.size(); i++) {
		outShapes << "*************************************************"
				<< endl;
		outShapes << "    shapesName: " << nonRectangularNodes[i]->name << endl;
		outShapes << "    numShapes = "
				<< nonRectangularNodes[i]->numRectangular << endl;
		for (unsigned long j = 0; j < nonRectangularNodes[i]->numRectangular; j++) {
			outShapes << "    id = " << j << endl;
			outShapes << "            shapeName: "
					<< nonRectangularNodes[i]->shapes[j].name << endl;
			outShapes << "            xCoord: "
					<< nonRectangularNodes[i]->shapes[j].xCoord << endl;
			outShapes << "            yCoord: "
					<< nonRectangularNodes[i]->shapes[j].yCoord << endl;
			outShapes << "            width: "
					<< nonRectangularNodes[i]->shapes[j].width << endl;
			outShapes << "            height: "
					<< nonRectangularNodes[i]->shapes[j].height << endl;
		}
	}

	outShapes.close();
	return;
}

void Block::checkNodes() {
	ofstream outShapes;
	outShapes.open("nodes.log", ios::out);
	cout << "now checking nodes..." << endl;
	cout << " nodes.size() = " << nodes.size() << endl;

	for (unsigned long i = 0; i < nodes.size(); i++) {
		outShapes << "*************************************************"
				<< endl;
		outShapes << "	nodeName: " << nodes[i]->name << endl;
		outShapes << "		xCoord: " << nodes[i]->getCoordX() << endl;
		outShapes << "		yCoord: " << nodes[i]->getCoordY() << endl;
		outShapes << "		width: " << nodes[i]->getWidth() << endl;
		outShapes << "		height: " << nodes[i]->getHeight() << endl;
	}

	outShapes.close();
	return;
}

void Block::checkNets() {
	cout << "now checking..." << endl;
	for (long i = 0; i < nets.size(); ++i) {
		myNet* net = nets[i];
		vector<InstTerm> terms = net->getTerms();
		for (long j = 0; j < terms.size(); ++j) {
			long index1 = terms[j].getIndexInst();
			for (long k = j + 1; k < terms.size(); ++k) {
				long index2 = terms[k].getIndexInst();
				if (index1 == index2) {
					cout << "[WARNING]: Net " << net->getName()
							<< " has two pins " << terms[j].getName()
							<< " and " << terms[k].getName()
							<< " on the same inst " << nodes[index1]->getName()
							<< endl;
				}
			}
		}
	}
}

void Block::checkRows() {
	ofstream outShapes;
	outShapes.open("rows.log", ios::out);
	cout << "now checking rows..." << endl;
	cout << " rows.size() = " << rows.size() << endl;

	for (unsigned long i = 0; i < rows.size(); i++) {
		outShapes << "*************************************************"
				<< endl;
		outShapes << "    rowName: " << rows[i]->name << endl;
		outShapes << "            height =  " << rows[i]->getHeight() << endl;
		outShapes << "            x =  " << rows[i]->getCoordX() << endl;
		outShapes << "            y =  " << rows[i]->getCoordY() << endl;
		outShapes << "            sw =  " << rows[i]->getSiteWidth() << endl;
		outShapes << "            sp =  " << rows[i]->getSiteSpace() << endl;
	}

	outShapes.close();
	return;
}

