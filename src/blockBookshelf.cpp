/*
 * blockBookshelf.cpp
 *
 *  Created on: Apr 23, 2014
 *      Author: thueda
 */

#include <block.h>

void Block::parseBookShelf(string file) {
	time_t begin = clock();
	string plFile = file + ".pl";
	//for debug
//	string plFile = "adaptec1.sol.2.pl";

	string nodesFile(file + ".nodes");
	string netsFile(file + ".nets");
	string sclFile(file + ".scl");
	string shapesFile(file + ".shapes");

	//parse .scl
	loadScl(sclFile.c_str());
	//parse .pl and .nodes
	loadNodePl(plFile.c_str(), nodesFile.c_str());
	if (nodeMapIndex.begin() != nodeMapIndex.end()) {
		nodeMapIndex.erase(nodeMapIndex.begin(), nodeMapIndex.end());
	}
	for (long i = 0; i < (long) nodes.size(); i++) {
		pair<map<const char*, long, ltstr>::iterator, bool> valid;
		valid = nodeMapIndex.insert(make_pair(nodes[i]->name, nodes[i]->index));
		if (!valid.second) {
			cout << "Error : there are this key int the map" << endl;
			return;
		}
	}

#ifdef DEBUG
	checkNodeMapIndex();
#endif
	//parse .nets
	loadNets(netsFile.c_str());
	netMapIndex = new map<const char*, long, ltstr> ;
	if (netMapIndex->begin() != netMapIndex->end()) {
#ifdef DEBUG
		cout<<"before netMapIndex.size() = "<<netMapIndex->size()<<endl;
#endif
		netMapIndex->clear();
	}
	for (long i = 0; i < (long) nets.size(); i++) {
		pair<map<const char*, long, ltstr>::iterator, bool> valid;
		valid = netMapIndex->insert(make_pair(nets[i]->name, nets[i]->index));
		if (!valid.second) {
			cout << "Error : there are this key int the map" << endl;
			return;
		}
	}

#ifdef DEBUG
	checkNetMapIndex();
#endif

	//parse .shapes
	loadShapes(shapesFile.c_str());
#ifdef DEBUG
	checkShapes();
#endif
	if (nonRectMapIndex.begin() != nonRectMapIndex.end()) {
		nonRectMapIndex.erase(nonRectMapIndex.begin(), nonRectMapIndex.end());
	}
	for (long i = 0; i < (long) nonRectangularNodes.size(); i++) {
		pair<map<const char*, long, ltstr>::iterator, bool> valid;
		valid = nonRectMapIndex.insert(
				make_pair(nonRectangularNodes[i]->name,
						nonRectangularNodes[i]->index));
		if (!valid.second) {
			cout << "Error : there are this key int the map" << endl;
			return;
		}
	}
#ifdef DEBUG
	checkNonRectMapIndex();
#endif

	//split nonRectangular nodes
	splitNonRectNode();

#ifdef DEBUG
	cout<<"check nodes..."<<endl;
	checkNodes();
	cout<<"check nets..."<<endl;
	checkNets();
	cout<<"check rows..."<<endl;
	checkRows();
#endif

	cout << "[INFO] : Total parseBookShelf " << double(clock() - begin)
			/ CLOCKS_PER_SEC << " sec." << endl;
	return;
}

void Block::splitNonRectNode() {
	cout << "now splitNonRectNode..." << endl;
	long numAddNodes = 0;
	long addIndex = nodes.size();
	for (long i = 0; i < (long) nonRectangularNodes.size(); i++) {
		numAddNodes += (long) nonRectangularNodes[i]->numRectangular;

		long thisIndex = nodeMapIndex[nonRectangularNodes[i]->name];

		double centerInstX = nodes[thisIndex]->xCoord
				+ ((nodes[thisIndex]->width) >> 1);
		double centerInstY = nodes[thisIndex]->yCoord
				+ ((nodes[thisIndex]->height) >> 1);

		long sumPins = 0;

		//deal with pins that on the common edge of both nodes
		int nodePins = (int) nodes[thisIndex]->numPins;
		bool *matched = new bool[nodePins];
		for (long f = 0; f < nodes[thisIndex]->numPins; f++) {
			matched[f] = false;
		}

		//test
		for (long k = 0; k < nodes[thisIndex]->numPins; k++) {
			long xOffset = nodes[thisIndex]->pins[k].getOffsetX();
			long yOffset = nodes[thisIndex]->pins[k].getOffsetY();

			double termX = xOffset + centerInstX;
			double termY = yOffset + centerInstY;
			bool vectory = false;

			for (long j = 0; j < nonRectangularNodes[i]->numRectangular; j++) {
				long width = nonRectangularNodes[i]->shapes[j].width;
				long height = nonRectangularNodes[i]->shapes[j].height;
				double xCoord = nonRectangularNodes[i]->shapes[j].xCoord;
				double yCoord = nonRectangularNodes[i]->shapes[j].yCoord;
				if (termX < xCoord || termX > (xCoord + (long) width) || termY
						< yCoord || termY > (yCoord + (long) height)) {
					continue;
				} else {
					vectory = true;
					break;
				}
			}

			if (!vectory) {
				cout << "node : " << i << endl;
				cout << "pin : " << k << endl;
				cout << "centerInstX = " << centerInstX << endl;
				cout << "centerInstY = " << centerInstY << endl;
				cout << "xOffset = " << xOffset << endl;
				cout << "yOffset = " << yOffset << endl;
				cout << "termX = " << termX << endl;
				cout << "termY = " << termY << endl;
				//assert(vectory);
			}
		}

		//split the nonRectangularNodes
		for (long j = 0; j < nonRectangularNodes[i]->numRectangular; j++) {
			Inst *addNode = new Inst();

			string tempString;
			stringstream d2s;
			d2s << j;
			d2s >> tempString;
			string suffix = "_" + tempString;

			strcpy(addNode->name, nonRectangularNodes[i]->name);
			strcat(addNode->name, suffix.c_str());

			addNode->index = addIndex;
			addNode->isRectangular = true;
			addNode->width = nonRectangularNodes[i]->shapes[j].width;
			addNode->height = nonRectangularNodes[i]->shapes[j].height;
			addNode->xCoord = nonRectangularNodes[i]->shapes[j].xCoord;
			addNode->yCoord = nonRectangularNodes[i]->shapes[j].yCoord;
			addNode->orient = nodes[thisIndex]->orient;
			addNode->status = nodes[thisIndex]->status;

			addNode->pins.clear();

			long numPins = 0;
			for (long k = 0; k < nodes[thisIndex]->numPins; k++) {
				if (matched[k]) {
					continue;
				}

				long xOffset = nodes[thisIndex]->pins[k].getOffsetX();
				long yOffset = nodes[thisIndex]->pins[k].getOffsetY();

				double termX = xOffset + centerInstX;
				double termY = yOffset + centerInstY;

				if (termX < addNode->xCoord || termX > addNode->xCoord
						+ (long) (addNode->width) || termY < addNode->yCoord
						|| termY > addNode->yCoord + (long) (addNode->height)) {
					continue;
				} else {
					matched[k] = true;

					InstTerm pin;

					//strcpy(pin.name,nodes[thisIndex]->pins[k].name);
					strcpy(pin.name, addNode->name);
					pin.type = nodes[thisIndex]->pins[k].type;
					assert((termX - (addNode->xCoord + ((addNode->width) >> 1))) == (long)(termX - (addNode->xCoord + ((addNode->width) >> 1))));
					assert((termY - (addNode->yCoord + ((addNode->height) >> 1))) == (long)(termY - (addNode->yCoord + ((addNode->height) >> 1))));
					pin.xOffset = (long) (termX - (addNode->xCoord
							+ ((addNode->width) >> 1)));
					pin.yOffset = (long) (termY - (addNode->yCoord
							+ ((addNode->height) >> 1)));
					pin.indexNet = nodes[thisIndex]->pins[k].indexNet;
					pin.indexNode = addIndex;

					//amend nets
					vector<InstTerm>::iterator pos =
							nets[pin.indexNet]->pins.begin();
					bool vectory = false;
					for (long l = 0; l < nets[pin.indexNet]->numPins; l++) {
						if (!strcmp(nets[pin.indexNet]->pins[l].name,
								nonRectangularNodes[i]->name)) {
							delete[] pos->name;
							nets[pin.indexNet]->pins.erase(pos);
							vectory = true;
							break;
						}
						pos++;
					}
					if (!vectory) {
						//assert(vectory);
					}

					nets[pin.indexNet]->pins.push_back(pin);
					//amend nodes
					addNode->pins.push_back(pin);
					numPins++;
					sumPins++;
				}
			}

			addNode->numPins = numPins;
			nodes.push_back(addNode);
			++numTotalNodes;
			addIndex++;
		}
		//assert(sumPins == nodes[thisIndex]->numPins);

		delete[] matched;
	}
	long totalNum1 = 0;
	long totalNum2 = 0;
	for (long id = 0; id < (long) nonRectangularNodes.size(); id++) {
		totalNum1 += nonRectangularNodes[id]->shapes.size();
		totalNum2 += (long) nonRectangularNodes[id]->numRectangular;
	}
	numAddNodes -= nonRectangularNodes.size();
	return;
}

void Block::loadShapes(const char *shapesFile) {
	cout << "now loadShapes..." << endl;
	ifstream shapesIn;
	shapesIn.open(shapesFile, ios::in);
	if (!shapesIn) {
		cout << "ERROR: .shapes file could not be opened successfully" << endl;
		return;
	}

	nonRectangularNodes.clear();

	char tempShapes[50];
	char c;

	while (!shapesIn.eof()) {
		if (shapesIn.eof()) {
			break;
		}
		if (shapesIn.peek() == '#') {
			string temp;
			getline(shapesIn, temp);
			//cout<<temp<<endl;
		}
		if (shapesIn.peek() == '\n' || shapesIn.peek() == '\r') {
			shapesIn.get();
			continue;
		}

		shapesIn >> tempShapes;
		if (!strcmp(tempShapes, "NumNonRectangularNodes")) {
#ifdef DEBUG
			cout<<"matched NumNonRectangularNodes"<<endl;
#endif
			shapesIn >> c >> numNonRectangularNodes;
			break;
		}
	}

	long id = 0;

	while (!shapesIn.eof()) {
		if (shapesIn.peek() == '\n' || shapesIn.peek() == '\r') {
			shapesIn.get();
			continue;
		}
#ifdef DEBUG
		cout<<"*******************************   id = "<<id<<endl;
#endif

		Shape *nonRectangularNode = new Shape();
		//===========hjytest=================
		//	cout<<"Shape size is:"<<sizeof(
		long numShapes = 0;
		shapesIn >> tempShapes >> c >> numShapes;

		if (shapesIn.eof()) {
			delete[] nonRectangularNode->name;
			delete nonRectangularNode;//hjyadd
			break;
		}
#ifdef DEBUG
		cout<<tempShapes<<"  "<<numShapes<<endl;
#endif
		//assert(tempShapes[0] == 'o');
		strcpy(nonRectangularNode->name, tempShapes);
		long thisIndex = nodeMapIndex[tempShapes];
		nodes[thisIndex]->isRectangular = false;
		for (long i = 0; i < numShapes; i++) {
			Inst node;
			shapesIn >> tempShapes;
			strcpy(node.name, tempShapes);
			shapesIn >> node.xCoord >> node.yCoord >> node.width >> node.height;
			node.xCoord *= bloat;
			node.yCoord *= bloat;
			node.width *= bloat;
			node.height *= bloat;
			nonRectangularNode->shapes.push_back(node);
		}
		nonRectangularNode->index = id;
		nonRectangularNode->numRectangular = numShapes;
		nonRectangularNodes.push_back(nonRectangularNode);
		id++;
	}
	shapesIn.close();
	return;
}

void Block::loadNodePl(const char *plFile, const char *nodeFile) {
	cout << "now loadNodePl..." << endl;
	ifstream nodeIn, plIn;
	//ofstream WPl_new;
	//WPl_new.open("WPl.txt",ios::app|ios::out);
	//if(!WPl_new)
	//cout<<"To make The WPl failed!"<<endl;
	nodeIn.open(nodeFile, ios::in);
	if (!nodeIn) {
		cout << "ERROR: .nodes file could not be opened successfully" << endl;
		exit(1);
	}
	plIn.open(plFile, ios::in);
	if (!plIn) {
		cout << "ERROR: .pl file could not be opened successfully" << endl;
		exit(1);
	}

	char tempNode[128] = { '\0' }, tempPl[128] = { '\0' };
	char c;
	unsigned long width, height;
	//long x,y;
	double x, y;
	string orient;
	unsigned long id = 0;

	nodes.clear();

	//preprocess the before rows of the pnr*.pl

	while (!plIn.eof()) {
		for (int i = 0; i < 4; ++i) {
			string temp;
			getline(plIn, temp);
		}

		plIn >> tempPl;
		break;
	}

	//preprocess the before rows of the pnr*.nodes
	while (!nodeIn.eof()) {
		for (int i = 0; i < 4; ++i) {
			string temp;
			getline(nodeIn, temp);
		}
		if (nodeIn.peek() == '\n' || nodeIn.peek() == '\r') {
			nodeIn.get();
		}
		nodeIn >> tempNode;
		if (!strcmp(tempNode, "NumNodes")) {
			nodeIn >> c >> numTotalNodes;
		}
		nodeIn >> tempNode;
		//===================20121203hjy==========================
		//		if(!strcmp(tempNode,"NumTermInals")) {
		if (!strcmp(tempNode, "NumTerminals")) {
			//=======================================================
			nodeIn >> c >> numTotalTerminals;
		}
		string temp;
		getline(nodeIn, temp);
		nodeIn >> tempNode;
		break;
	}

	//save the width,height,coordinate(x,y) of the node in the array
	Inst *tempNodeL2 = NULL;
#ifdef DEBUG
	cout<<id<<" : "<<tempPl<<" vs "<<tempNode<<endl;
#endif
	do {

		if (!strcmp(tempPl, "/FIXED")) //tempPl is /FIXED,tempNode is terminal
		{
			tempNodeL2->status = Fixed;
			plIn >> tempPl;
			nodeIn >> tempNode;
			if (plIn.eof()) {
				break;
			}
			if (nodeIn.eof()) {
				break;
			}
		} else if (!strcmp(tempPl, "/FIXED_NI"))//tempPl is /FIXED_NI,tempNode is terminal_NI
		{
			tempNodeL2->status = FixNI;
			plIn >> tempPl;
			nodeIn >> tempNode;
			if (plIn.eof()) {
				break;
			}
			if (nodeIn.eof()) {
				break;
			}
		}

		else if (tempNodeL2) {

			tempNodeL2->status = Moved;
		}

		if (nodeIn.peek() == '\n' || nodeIn.peek() == '\r') {
			nodeIn.get();
			continue;
		}
		if (plIn.peek() == '\n' || plIn.peek() == '\r') {
			plIn.get();
			continue;
		}

		//assert(tempNode[0] == 'o' || tempNode[0] == 'p');
		//assert(tempPl[0] == 'o' || tempPl[0] == 'p');
		plIn >> x >> y >> c >> orient;

		//WPl_new<<" "<<x<<" "<<y<<c<<" "<<orient;

		nodeIn >> width >> height;
		Inst *tempNodeL = new Inst();
		strcpy(tempNodeL->name, tempNode);
		tempNodeL->index = id;
		tempNodeL->isRectangular = true;
		tempNodeL->width = width * bloat;
		tempNodeL->height = height * bloat;
		tempNodeL->xCoord = x * bloat;
		tempNodeL->yCoord = y * bloat;
		tempNodeL->numPins = 0;
		if (orient == "N") {
			tempNodeL->orient = kR0;
		}

		tempNodeL->pins.clear();

		tempNodeL2 = tempNodeL;
		nodes.push_back(tempNodeL);
#ifdef DEBUG
		cout<<"parse node's id = "<<id<<endl;
#endif
		++id;
	} while ((nodeIn >> tempNode) && (plIn >> tempPl));//read the status like FIXED or FIXED_NI;terminal or terminal_NI

	nodeIn.close();
	plIn.close();

	//WPl_new.close();

	return;
}

void Block::loadNets(const char *netFile) {
	cout << "now loadNets..." << endl;
	ifstream netIn;
	netIn.open(netFile, ios::in);
	if (!netIn) {
		cout << "ERROR: .nets file could not be opened successfully" << endl;
		exit(1);
	}
	nets.clear();

	string tempNet, instName;
	char c, pinType[5];
	double xoffset, yoffset;
	unsigned long pinsNum;
	unsigned long id = 0;
	unsigned long numpins = 0;

	while (netIn >> tempNet) {
		if (tempNet == "UCLA" || tempNet[0] == '#' || tempNet.size() == 0) {
			if (tempNet.size()) {
				getline(netIn, tempNet);
			}
			continue;
		}
		if (tempNet == "NumNets") {
			netIn >> c >> numTotalNets;
			continue;
		}
		if (tempNet == "NumPins") {
			netIn >> c >> numTotalPins;
			continue;
		}
		if (tempNet.size()) {
			break;
		}
	}
	do {
		if (tempNet == "NetDegree") {
			netIn >> c >> pinsNum >> tempNet;
			/*if (pinsNum > 1){
			 fout<<"NetDegree "<<c<<" "<<pinsNum<<" "<<tempNet<<endl;
			 isValid = true;
			 numValidNets++;
			 }
			 else{
			 isValid = false;
			 }*/

			myNet *tempNetL = new myNet();
			strcpy(tempNetL->name, tempNet.c_str());
			tempNetL->index = id;
			tempNetL->numPins = 0;
			for (unsigned long i = 0; i < pinsNum; i++) {
				if (netIn.peek() == '\n' || netIn.peek() == '\r') {
					netIn.get();
					//continue;
				}

				netIn >> instName >> pinType >> c >> xoffset >> yoffset;
				/*if (isValid){
				 fout<<"\t"<<instName<<" "<<pinType<<" "<<c<<" "<<xoffset<<" "<<yoffset<<endl;
				 numValidPins++;
				 }*/
				InstTerm tempPin;
				if (!strcmp(pinType, "I")) {
					tempPin.type = Input;
				} else if (!strcmp(pinType, "O")) {
					tempPin.type = Output;
				} else {
					//cout<<"Error :: there are other pin type"<<endl;
					tempPin.type = Other;
				}
				strcpy(tempPin.name, instName.c_str());
				tempPin.xOffset = (long) (xoffset * bloat);
				tempPin.yOffset = (long) (yoffset * bloat);
				unsigned long thisIndex = nodeMapIndex[instName.c_str()];
				//cout<<instName<<"    "<<thisIndex<<endl;
				tempPin.indexNet = id;
				tempPin.indexNode = thisIndex;
				nodes[thisIndex]->pins.push_back(tempPin);
				nodes[thisIndex]->numPins++;

				tempNetL->pins.push_back(tempPin);
				tempNetL->numPins++;

				numpins++;

				//assert(pinsNum>0);

			}
			nets.push_back(tempNetL);

#ifdef DEBUG
			//cout<<"parse net's id = "<<id<<endl;
#endif
			++id;

		}
	} while (netIn >> tempNet);
	//assert(numpins == numTotalPins);
	//cout<<numValidNets<<endl;
	//cout<<numValidPins<<endl;
	//fout.close();
	netIn.close();
	return;
}

void Block::loadScl(const char *sclFile) {
	//cout<<"now loadScl..."<<endl;
#ifdef DEBUG
	ofstream out;
	out.open("testScl.log",ios::out);
#endif
	ifstream sclIn;
	sclIn.open(sclFile, ios::in);
	if (!sclIn) {
		cout << "ERROR: .scl file could not be opened successfully" << endl;
		exit(1);
	}

	string tempScl;
	char c;

	string sdNamePre("sd");
	unsigned long id = 0;

	while (sclIn >> tempScl) {
		if (sclIn.eof()) {
			break;
		}
		if (tempScl.size() == 0 || tempScl[0] == '#' || strcmp(tempScl.c_str(),
				"UCLA") == 0) {
			if (tempScl.size()) {
				getline(sclIn, tempScl);
			}
			continue;
		}
		if (!strcmp(tempScl.c_str(), "NumRows")) {
#ifdef DEBUG
			out<<"matched NumRows"<<endl;
#endif
			sclIn >> c >> numTotalRows;
			break;
		}
	}
	while (!sclIn.eof()) {
#ifdef DEBUG
		out<<"*******************************"<<endl;
#endif
		sclIn >> tempScl;
		if (sclIn.eof()) {
			break;
		}

		myRow *row = new myRow();
		if (!strcmp(tempScl.c_str(), "CoreRow")) {
#ifdef DEBUG
			out<<"matched CoreRow"<<endl;
#endif
			sclIn >> tempScl;
			if (!strcmp(tempScl.c_str(), "Horizontal")) {
#ifdef DEBUG
				out<<"matched Horizontal"<<endl;
#endif
				row->rowOrient = kR0;
			} else {
				cout << "Warning : there is other row orient" << endl;
			}
		}
		sclIn >> tempScl;
		if (!strcmp(tempScl.c_str(), "Coordinate")) {
#ifdef DEBUG
			out<<"matched Coordinate"<<endl;
#endif
			sclIn >> c >> (row->yCoord);
			(row->yCoord) *= bloat;
		}

		sclIn >> tempScl;
		if (!strcmp(tempScl.c_str(), "Height")) {
#ifdef DEBUG
			out<<"matched Height"<<endl;
#endif
			sclIn >> c >> (row->height);
			(row->height) *= bloat;
		}

		sclIn >> tempScl;
		if (!strcmp(tempScl.c_str(), "Sitewidth")) {
#ifdef DEBUG
			out<<"matched Sitewidth"<<endl;
#endif
			sclIn >> c >> (row->siteWidth);
			(row->siteWidth) *= bloat;
		}

		sclIn >> tempScl;
		if (!strcmp(tempScl.c_str(), "Sitespacing")) {
#ifdef DEBUG
			out<<"matched Sitespacing"<<endl;
#endif
			sclIn >> c >> (row->siteSpacing);
			(row->siteSpacing) *= bloat;
			//(row->siteSpacing) -= (row->siteWidth);
		}

		sclIn >> tempScl;
		if (!strcmp(tempScl.c_str(), "Siteorient")) {
#ifdef DEBUG
			out<<"matched Siteorient"<<endl;
#endif
			sclIn >> c >> tempScl;
			row->siteOrient = kR0;
		}

		sclIn >> tempScl;
		if (!strcmp(tempScl.c_str(), "Sitesymmetry")) {
#ifdef DEBUG
			out<<"matched Sitesymmetry"<<endl;
#endif
			sclIn >> c >> (row->symmetry);
		}
		sclIn >> tempScl;
		if (!strcmp(tempScl.c_str(), "SubrowOrigin")) {
#ifdef DEBUG
			out<<"matched SubrowOrigin"<<endl;
#endif
			sclIn >> c >> (row->xCoord);
			(row->xCoord) *= bloat;
		}

		sclIn >> tempScl;
		if (!strcmp(tempScl.c_str(), "NumSites")) {
#ifdef DEBUG
			out<<"matched NumSites"<<endl;
#endif
			sclIn >> c >> (row->numSites);
		}

		sclIn >> tempScl;
		if (!strcmp(tempScl.c_str(), "End")) {
#ifdef DEBUG
			out<<"matched End"<<endl;
#endif
			string tempName;
			stringstream s2d;
			s2d << id;
			s2d >> tempName;
			string sdName = sdNamePre + tempName;
			strcpy(row->name, sdName.c_str());
			rows.push_back(row);
			id++;
		}
	}
#ifdef DEBUG
	out<<"numRows = "<<id<<"  vs  "<<numTotalRows<<endl;
	out.close();
#endif

	return;
}
void Block::savePl(const char *filename, const char *oldPlFile) {
	ofstream outPl;
	//ifstream inPlName;
	string pl;
	string name;
	myPoint myPoint;

	if (filename == NULL) {
		pl = "pnrSave.pl";
	} else {
		pl = filename;
		if (pl.size() > 3) {
			const char *p = filename + pl.size() - 3;
			if (strcmp(p, ".pl") != 0) {
				pl += ".pl";
			}
		} else {
			pl += ".pl";
		}
	}
	outPl.open(pl.c_str());
	if (!outPl) {
		cout << "ERROR in saving .pl file" << endl;
		return;
	}
	cout << "Saving " << filename << " as output .pl file" << endl;

	time_t ltime;
	time(&ltime);
	struct passwd* pwd = getpwuid(getuid());
	outPl << "UCLA pl 1.0" << endl;
	outPl << "# Created  :  " << ctime(&ltime);
	outPl << "# User     :  " << pwd->pw_name << " from Tsinghua university"
			<< endl << endl;

	//assert(bloat == 10);
	long numNodes = nodes.size();
	for (long i = 0; i < numNodes; ++i) {
		Inst *inst = nodes[i];
		if (!(inst->isRect())) {
			continue;
		}
		if (inst->getStatus() == Moved) {
			//inPlName>>name;
			name = inst->getName();
			myPoint = inst->getOrigin();
			long xx = (int) myPoint.coordX();
			long x = (int) (xx / bloat);
			long yy = (int) myPoint.coordY();
			long y = (int) (yy / bloat);
			outPl << setw(15) << setiosflags(ios::right) << name << setw(15)
					<< setiosflags(ios::right) << x << setw(15) << setiosflags(
					ios::right) << y << " : ";
			switch (inst->getOrient()) {
			case kR0:
				outPl << "N" << endl;
				break;
			case kR90:
				outPl << "W" << endl;
				break;
			case kR180:
				outPl << "S" << endl;
				break;
			case kR270:
				outPl << "E" << endl;
				break;
			case kMY:
				outPl << "FN" << endl;
				break;
			case kMYR90:
				outPl << "FW" << endl;
				break;
			case kMX:
				outPl << "FS" << endl;
				break;
			case kMXR90:
				outPl << "FE" << endl;
				break;
			default:
				break;
			}
		}
	}

	ifstream plIn;
	plIn.open(oldPlFile, ios::in);
	if (!plIn) {
		cout << "ERROR in opening .pl file" << endl;
		return;
	}

	string instName, tempPl, orient;
	char c;
	double x, y;
	while (plIn >> tempPl) {
		if (tempPl == "UCLA" || tempPl.size() == 0 || tempPl[0] == '#') {
			if (tempPl.size()) {
				getline(plIn, tempPl);
			}
			continue;
		}
		if (tempPl[0] != '/') {
			instName = tempPl;
			plIn >> x >> y >> c >> orient;
		} else if (tempPl == "/FIXED" || tempPl == "/FIXED_NI") {
			outPl << setw(15) << setiosflags(ios::right) << instName
					<< setw(15) << setiosflags(ios::right) << x << setw(15)
					<< setiosflags(ios::right) << y << " : " << orient << "  "
					<< tempPl << endl;
		}
	}
	plIn.close();
	outPl.close();
	//inPlName.close();
	return;
}

void Block::savePlAccurate(const char *filename, const char *oldPlFile) {
	ofstream outPl;
	//ifstream inPlName;
	string pl;
	string name;
	myPoint myPoint;

	if (filename == NULL) {
		pl = "pnrSave.pl";
	} else {
		pl = filename;
		if (pl.size() > 3) {
			const char *p = filename + pl.size() - 3;
			if (strcmp(p, ".pl") != 0) {
				pl += ".pl";
			}
		} else {
			pl += ".pl";
		}
	}
	outPl.open(pl.c_str());
	if (!outPl) {
		cout << "ERROR in saving .pl file" << endl;
		return;
	}
	cout << "Saving " << filename << " as output .pl file" << endl;

	time_t ltime;
	time(&ltime);
	struct passwd* pwd = getpwuid(getuid());
	outPl << "UCLA pl 1.0" << endl;
	outPl << "# Created  :  " << ctime(&ltime);
	outPl << "# User     :  " << pwd->pw_name << " from Tsinghua university"
			<< endl << endl;

	//assert(bloat == 10);
	long numNodes = nodes.size();
	for (long i = 0; i < numNodes; ++i) {
		Inst *inst = nodes[i];
		if (!(inst->isRect())) {
			continue;
		}
		if (inst->getStatus() == Moved) {
			//inPlName>>name;
			name = inst->getName();
			myPoint = inst->getOrigin();
			double xx = myPoint.coordX();
			double x = xx / bloat;
			double yy = myPoint.coordY();
			double y = yy / bloat;
			outPl <<setprecision(12) <<setw(15) << setiosflags(ios::right) << name << setw(15)
					<< setiosflags(ios::right) << x << setw(15) << setiosflags(
					ios::right) << y << " : ";
			switch (inst->getOrient()) {
			case kR0:
				outPl << "N" << endl;
				break;
			case kR90:
				outPl << "W" << endl;
				break;
			case kR180:
				outPl << "S" << endl;
				break;
			case kR270:
				outPl << "E" << endl;
				break;
			case kMY:
				outPl << "FN" << endl;
				break;
			case kMYR90:
				outPl << "FW" << endl;
				break;
			case kMX:
				outPl << "FS" << endl;
				break;
			case kMXR90:
				outPl << "FE" << endl;
				break;
			default:
				break;
			}
		}
	}

	ifstream plIn;
	plIn.open(oldPlFile, ios::in);
	if (!plIn) {
		cout << "ERROR in opening .pl file" << endl;
		return;
	}

	string instName, tempPl, orient;
	char c;
	double x, y;
	while (plIn >> tempPl) {
		if (tempPl == "UCLA" || tempPl.size() == 0 || tempPl[0] == '#') {
			if (tempPl.size()) {
				getline(plIn, tempPl);
			}
			continue;
		}
		if (tempPl[0] != '/') {
			instName = tempPl;
			plIn >> x >> y >> c >> orient;
		} else if (tempPl == "/FIXED" || tempPl == "/FIXED_NI") {
			outPl << setw(15) << setiosflags(ios::right) << instName
					<< setw(15) << setiosflags(ios::right) << x << setw(15)
					<< setiosflags(ios::right) << y << " : " << orient << "  "
					<< tempPl << endl;
		}
	}
	plIn.close();
	outPl.close();
	//inPlName.close();
	return;
}
