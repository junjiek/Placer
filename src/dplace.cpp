#include <algorithm>
#include <time.h>
#include <signal.h>
#include <iostream>
#include <set>
#include "dplace.h"
#include <time.h>
#include "ldpUtility.h"

using namespace std;

bool cmpRowBottom(myRow* row1, myRow* row2)
{
		return row1->getCoordY() < row2->getCoordY();
}

inline bool Compare(const fdplInstSort &inst1, const fdplInstSort &inst2)
{
		return inst1.wl < inst2.wl;
}

int minMoveGD(Line *lines, size_t num, int lower, int upper);
int (*minMoveLine)(Line *lines, size_t num, int lower, int upper) = minMoveGD;

fdplDetailPlace::fdplDetailPlace(Block *blk) : myPlacement(blk)
{
}

void fdplDetailPlace::dplace()
{
		// region
		setRegion();
		rowArea = getRegion();
		blockX = rowArea.left();
		blockY = rowArea.bottom();
		blockW = rowArea.width();
		blockH = rowArea.height();

		//cout<<"myRow Region {"<<rowArea.left()<<", "
		//  <<rowArea.right()<<", "
		//  <<rowArea.bottom()<<", "
		//  <<rowArea.top()<<"}"
		//  <<endl;
		// row
		rows = plcTopBlock->getRows();
		//cout<<"Rows: "<<rows.size()<<endl;
		sort(rows.begin(), rows.end(), cmpRowBottom);
		for (unsigned long i = 0; i < rows.size(); i++)
		{
				myRow *row = rows[i];
				/*cout<<"myRow "<<i<<"  x: "<<row->getCoordX()
				  <<"  y: "<<row->getCoordY()
				  <<"  width: "<<row->getWidth()
				  <<"  height: "<<row->getHeight()
				  <<"  siteWidth: "<<row->getSiteWidth()
				  <<"  siteSpace: "<<row->getSiteSpace()
				  <<"  siteNum: "<<row->getNumSites()<<endl;*/
				rowIndexMap[rows[i]] = i;
		}
		rowGap = 0;
		// net
		nets = plcTopBlock->getNets();
		// inst
		insts = plcTopBlock->getInsts();
		//guiFile_LG("gplace.gnu");//hjy20130117add====================
		//cout<<"Insts: "<<insts.size()<<endl;

		// change the position of cells, commented by Zhongdong
		/*
		   for (unsigned long i = 0; i < insts.size(); i++) {
		   if (insts[i]->getStatus() == Moved) {
		   long x = rand() % blockW + blockX;
		   long y = rand() % blockH + blockY;
		   insts[i]->setOrigin(x, y);
		   }
		   }*/

		clock_t begin, end;
		begin = clock();
		init();    //TODO: what does this mean?
		cout<<"init Successfully!"<<endl;

		//cout<<" stdCells: "<<stdCells.size()<<" pins: "<<pins.size()<<endl;
		//cout << "begin format HPWL = " << myPlacement::getHPWL(plcTopBlock) << endl;
		format();  //TODO: what does this mean?
		cout<<"format successfully!"<<endl;

		//guiFile("format.gnu");
		//cout << "after format HPWL = " << myPlacement::getHPWL(plcTopBlock) << endl;

		try{
				legalize();
		}catch(...)
		{
				cout<<"Legalize EXP:"<<endl;
		}

		refineOrient();

		//guiFile("dplace.gnu");
		//cout << "finish the detail place HPWL = " << myPlacement::getHPWL(plcTopBlock) << endl;
		end = clock();
		//cout << "Total CPU: " << end - begin <<endl; 
		cout<<"Total CPU :"<<double(end-begin)/CLOCKS_PER_SEC<<endl;

}

void fdplDetailPlace::format()
{
		myPoint point;
		Rect Box;
		for (unsigned long i = 0; i < instOfRows.size(); i++)
		{
				for (unsigned long j = 0; j < instOfRows[i]->size(); j++)
				{
						instOfRows[i]->at(j).type &= ~fdplcSiteInstType;
				}
				instOutRows[i]->clear();
		}
		for (vector<Inst*>::iterator it = pins.begin(); it != pins.end(); it++)
		{
				insertFixed(*it);
		}
		for (unsigned long i = 0; i < instOfRows.size(); i++)
		{
				instOutRows[i]->clear();
		}
		//===========================hjy20130416========================
		/*for(vector<Inst*>::iterator it=Macros.begin();it!=Macros.end();it++)
		  {
		  LdpUtility::getBBox((*it), Box);
		  point=(*it)->getOrigin();
		  if(Box.left()<rowArea.left())
		  {
		  point.setCoordX(rowArea.left());
		  }
		  else if(Box.right()>rowArea.right())
		  {
		  point.setCoordX(rowArea.right()-Box.width());
		  }
		  if(Box.bottom()<rowArea.bottom())
		  {
		  point.setCoordY(rowArea.bottom());
		  }
		  else if(Box.top()>rowArea.top())
		  {
		  point.setCoordY(rowArea.top()-Box.height());
		  }
		  setOrigin((*it),point);
		  insert(*it);

		  }*/
		//===============================================================
		//========================hjy20130530===============================
		/*vector<fdplInstSort> sortStdInsts(stdCells.size());
		for (unsigned long i = 0; i < stdCells.size(); i++)
		{
			sortStdInsts[i].inst = stdCells[i];
			//sortStdInsts[i].wl = getHPWL(stdCells[i]);
			sortStdInsts[i].wl=stdCells[i]->getWidth();//20130516

		}
		sort(sortStdInsts.begin(), sortStdInsts.end(), Compare);
		for (unsigned long i = 0, j = stdCells.size() - 1;i < stdCells.size(); i++, j--)
		{
			stdCells[i] = sortStdInsts[j].inst;
		}*/

		sort(stdCells.begin(),stdCells.end(),fdplDetailPlace::CompareW);
		//===================================================================
		for (vector<Inst*>::iterator it = stdCells.begin(); it != stdCells.end(); it++)
		{
				if ((*it)->getCoordX() < rowArea.left()) {
						(*it)->setCoordX(rowArea.left());
				} else if ((*it)->getCoordX() + (*it)->getWidth()
								> rowArea.right()) {
						(*it)->setCoordX(rowArea.right() - (*it)->getWidth());
				}
				if ((*it)->getCoordY() < rowArea.bottom()) {
						(*it)->setCoordY(rowArea.bottom());
				} else if ((*it)->getCoordY() + (*it)->getHeight()
								> rowArea.top()) {
						(*it)->setCoordY(rowArea.top() - (*it)->getHeight());
				}
				insert(*it);
		}

}

void fdplDetailPlace::legalize()
{
		vector<Inst*> over;
		vector<Inst*> bigInsts;
		vector<Inst*> invalidInsts;
		unsigned long totalInstArea;
		unsigned long totalUseSiteArea;
		double rateOfArea;

		setDefaultArg();
		//cout<<"setDefaultArg successfully!"<<endl;
		//cout << "begin legalize HPWL = " << myPlacement::getHPWL(plcTopBlock) << endl;
		rateOfArea = getRateOfArea(totalInstArea, totalUseSiteArea);
		//cout << "overlap Stat. need sites " << totalInstArea
		//<< " occupy sites " << totalUseSiteArea
		//<< " overlap rate = " << rateOfArea << endl;
		//cout<<"rateOfArea successfully!"<<endl;
		//==========================hjy20130416===============================
		if (Macros.size() > 0)
		{
				cout << "Need to legalize the Macros!" << endl;
				/*try
				  {
				  placeMacros();
				  }
				  catch (...)
				  {
				  cout << "legalize macros failed" << endl;
				  }
				  for (vector<Inst*>::iterator it = macros.begin();it != macros.end(); it++)
				  {
				  if (insert(*it) != fdplcValidPlace)
				  {
				//removeInst(*it);
				cout << (*it)->getName() << " can't e legalized" << endl;
				}
				}
				for (size_t i = 0; i < instOfRows.size(); i++)
				{
				instOutRows[i]->clear();
				}
				setPlacementStatus(macros, oacLockedPlacementStatus);
				cout << "Finish legalize macros HPWL = " << getHPWL() << endl;
				rateOfArea = getRateOfArea(totalInstArea, totalUseSiteArea, macros);
				cout << "Macros overlap Stat. need sites " << totalInstArea
				<< " occupy sites " << totalUseSiteArea
				<< " overlap rate = " << rateOfArea << endl;
				rateOfArea = getRateOfArea(totalInstArea, totalUseSiteArea);
				cout << "overlap Stat. need sites " << totalInstArea
				<< " occupy sites " << totalUseSiteArea
				<< " overlap rate = " << rateOfArea << endl;*/
		}

		//====================================================================
		placeStds();

		//=============hjy20130115add==========================
		if(visible_LG)
		{
				string LG="LG";
				string guiName=string("LG_place.")+LG+string(".gnu");
				guiFile_LG(guiName.c_str());
		}
		//cout << "finish the legalize HPWL = " << myPlacement::getHPWL(plcTopBlock) << endl;
		cout << "finish the legalize HPWL = " << myPlacement::getHPWL(plcTopBlock) << endl;

}

void fdplDetailPlace::setDefaultArg()
{
		//offsetWeight = 1.0;
		//overlapWeight = 2.3;//original is 2.0
	    //maxSegLenCof = 6.0;
		double t=0.29;
	    offsetWeight = t;
		overlapWeight =1.0-t;
		maxSegLenCof = 6.0;

		ifstream in("arg.ini");
		map<string, string> content;
		map<string, string>::iterator it;;
		string key;
		string value;
		while (in >> key >> value) {
				content[key] = value;
		}
		in.close();
		if ((it = content.find("offsetWeight")) != content.end()) {
				sscanf(it->second.c_str(), "%lf", &offsetWeight);
		}
		if ((it = content.find("overlapWeight")) != content.end()) {
				sscanf(it->second.c_str(), "%lf", &overlapWeight);
		}
		if ((it = content.find("maxSegLenCof")) != content.end()) {
				sscanf(it->second.c_str(), "%lf", &maxSegLenCof);
		}
}

double fdplDetailPlace::getRateOfArea(unsigned long &totalInstArea, 
				unsigned long &useSiteArea) 
{
		Rect box;
		//================hjy20130407===================
		//set<unsigned long> sites;
		unsigned long sites;
		//==============================================
		fdplSiteContainer loc;
		myRow *row;
		unsigned long siteIndex;
		unsigned long numSites;
		unsigned long numInstSites;
		unsigned long rowIndex;

		totalInstArea = useSiteArea = 0;
		for (unsigned long i = 0; i < insts.size(); i++)
		{
				//======================hjy20130407===================

				/*getSite(insts[i], loc);
				  while (row = loc.getNext(siteIndex))
				  {
				  numSites = row->getNumSites();
				  rowIndex = getIndex(row);
				  numInstSites = getNumSite(insts[i], row);
				  totalInstArea += numInstSites;
				  numInstSites = min(numInstSites, numSites - siteIndex);
				  for (unsigned long j = 0; j < numInstSites; j++)
				  {
				//sites.insert(rowIndex * numSites + siteIndex + j);
				sites.insert(rowIndex+numSites+siteIndex+j);
				}
				}*/
				unsigned long SiteTest;
				SiteTest=getSite(insts[i],loc);
				//cout<<"getSite successfully!"<<endl;
				while (row = loc.getNext(siteIndex))
				{
						numSites = row->getNumSites();
						rowIndex = getIndex(row);
						numInstSites = getNumSite(insts[i], row);//Numsites Insts occupy
						totalInstArea += numInstSites;
						numInstSites = min(numInstSites, numSites - siteIndex);
						unsigned long J;
						//================hjy20130407===========================
						sites=0;
						for(unsigned long j=0;j<numInstSites;j++ )
						{
								sites++;
						}
						//=======================================================

				}

		}
		//====================hjy20130407=====================
		//useSiteArea = sites.size();
		useSiteArea=sites;
		//====================================================
		//cout<<"getRateOfArea successfully!"<<endl;
		return ((1. * totalInstArea) / useSiteArea);
}

void fdplDetailPlace::removeInst(Inst *inst)
{
		myRow *row(0);
		unsigned long index(0);
		unsigned long rowIndex(0);
		fdplSiteContainer siteCon;
		unsigned long num;
		unsigned long numSites;
		vector<fdplSiteContent> *p;

		if (getSite(inst, siteCon) == 0) {
				return;
		}
		while (row = siteCon.getNext(index))
		{
				rowIndex = getIndex(row);
				if (rowIndex >= rows.size())
				{
						throw "Error : overflow";
				}
				num = getNumSite(inst, row);
				numSites = row->getNumSites();
				p = instOfRows[rowIndex];
				for (vector<fdplSiteContent>::size_type i = 0;index + i < numSites && i < num; i++)
				{
						if (((*p)[index + i].type & fdplcSiteInstType) &&(*p)[index + i].obj.inst == inst)
						{
								(*p)[index + i].type &= ~fdplcSiteInstType;
								(*p)[index + i].obj.inst = 0;
						}
				}
				vector<Inst*>::iterator ot = find(instOutRows[rowIndex]->begin(), 
								instOutRows[rowIndex]->end(), inst);
				//====================hjy20130502=============================
				/*if (ot != instOutRows[rowIndex]->end())
				{
						*ot = instOutRows[rowIndex]->back();
						instOutRows[rowIndex]->pop_back();
				}*/
				if(ot!=instOutRows[rowIndex]->end())
				{
					instOutRows[rowIndex]->erase(ot);
				}
				//=============================================================
		}
}

void fdplDetailPlace::insertFixed(Inst *inst) 
{
		fdplSiteContainer loc;
		getSite(inst, loc);
		unsigned long index;
		myRow *temp;
		while (temp = loc.getNext(index))
		{
				unsigned long num = getNumSite(inst, temp);
				unsigned long rowIndex = getIndex(temp);
				for (unsigned long i = 0; index + i < instOfRows[rowIndex]->size() && i < num; i++)
				{
						(*instOfRows[rowIndex])[index + i].obj.inst = inst;
						(*instOfRows[rowIndex])[index + i].type = fdplcSiteBlockageType;
				}
		}
}


unsigned long fdplDetailPlace::insertInstToRow(Inst *inst, myRow *row,
				unsigned long siteIndex)
{
		myPoint origin;
		fdplSiteContainer loc;
		removeInst(inst);
		origin = getROrigin(row, siteIndex);
		inst->setOrigin(origin);

		getSite(inst, loc);//get the row and firs_siteIndex the inst/macro occupy
		myRow *temp;
		unsigned long index;
		while (temp = loc.getNext(index))//get the CurRow and firstSiteIndex
		{
				bool out(false);
				unsigned long num = getNumSite(inst, temp);
				unsigned long rowIndex = getIndex(temp);
				//for (unsigned long i = 0; index + i < instOfRows[rowIndex]->size() && i < num; i++)
				for (vector<fdplSiteContent>::size_type i = 0;index + i < instOfRows[rowIndex]->size() && i < num; i++)
				{
						if ((*instOfRows[rowIndex])[index + i].type == fdplcSiteFreeType)
						{
								(*instOfRows[rowIndex])[index + i].obj.inst = inst;
								(*instOfRows[rowIndex])[index + i].type = fdplcSiteInstType;
								//cout<<"Insert "<<(*instOfRows[rowIndex])[index + i].type<<endl;
						}
						//=====================================================================
								//out = true;
								//========================hjy20130417========================
								/*if ((*instOfRows[rowIndex])[index + i].type & fdplcSiteBlockageType)
								  {
								  res |= fdplcOverlapWithBlockagePlace;
								  continue;
								  }
								  Inst *exInst((*instOfRows[rowIndex])[index + i].obj.inst);
								  switch (exInst->getPlacementStatus())
								  {
								  case oacFixedPlacementStatus:
								  case oacLockedPlacementStatus:
								  res |= fdplcOverlapWithFixedPlace;
								  break;
								  default:
								  res |= fdplcOverlapPlace;
								  break;
								  }*/
								//=============================================================
						else
						{
							out=true;

						}
				}

				if (index + num > instOfRows[rowIndex]->size()) {
						out = true;
				}
				if (out) {
						instOutRows[rowIndex]->push_back(inst);
				}

		}

		//cout<<"insertInstToRow successfully!"<<endl;
		//return 0;
		return 0;
}



/*
unsigned long fdplDetailPlace::insertInstToRow(Inst *inst, myRow *row, 
				unsigned long siteIndex)
{
		myPoint origin;
		fdplSiteContainer loc;
		removeInst(inst);
		origin = getROrigin(row, siteIndex);
		inst->setOrigin(origin);
		getSite(inst, loc);//get the row and firs_siteIndex the inst/macro occupy
		myRow *temp;
		unsigned long index;

		//*************************************
		vector<fdplFreeSite> *freeSitesCur;
		freeSitesCur = new vector<fdplFreeSite> [rows.size()];
		unsigned long maxSegLenCur;
		unsigned long **occupySitesCur;
		setDefaultArg();
		//ofstream K;
		//K.open("K.txt",ios::out|ios::app);
		//occupySitesCur=new unsigned long* [rows.size()];
		for (unsigned long i = 0; i < rows.size(); i++)
		{//The word "rows" means all rows in the chip
			maxSegLenCur = static_cast<unsigned long>(rows[i]->getNumSites() / maxSegLenCof);
			getFreeSite(rows[i], freeSitesCur[i], maxSegLenCur);
			//occupySitesCur[i] = new unsigned long[rows[i]->getNumSites()];

		}
		//**************************************
		while (temp = loc.getNext(index))//get the CurRow and firstSiteIndex
		{
				bool out(false);
				bool overlapMOV(false);//====hjy20130503=======
				bool overlapFIX(false);//====hjy20130503=======
				bool FLAG(false);//=====hjy20130507=====
				unsigned long num = getNumSite(inst, temp);
				unsigned long rowIndex = getIndex(temp);
				//unsigned long siteWidthCur = temp->getSiteSpace();
				//maxSegLenCur = static_cast<unsigned long>(rows[rowIndex]->getNumSites() / maxSegLenCof);
				//cout<<"freeSiteCur.size():"<<freeSitesCur[rowIndex].size()<<endl;

				for (unsigned long i = 0; index + i < instOfRows[rowIndex]->size() && i < num; i++)
				//for (unsigned long i = 0; index + i < sizeInst && i < num; i++)
				{

						if ((*instOfRows[rowIndex])[index + i].type == fdplcSiteFreeType)
						{
							    //occupySitesCur[rowIndex][index+i]=0;
								(*instOfRows[rowIndex])[index + i].obj.inst = inst;
								(*instOfRows[rowIndex])[index + i].type = fdplcSiteInstType;


								//cout<<"Insert "<<(*instOfRows[rowIndex])[index + i].type<<endl;     
						}
						else
						{
								if((*instOfRows[rowIndex])[index+i].type==fdplcSiteInstType)
								{
									overlapMOV=true;
									//occupySitesCur[rowIndex][index+i]=1;
								}
								if((*instOfRows[rowIndex])[index+i].type==fdplcSiteBlockageType)
								{
									overlapFIX=true;
									//occupySitesCur[rowIndex][index+i]=1;
								}
						}
				}
				if((!overlapMOV)&&(!overlapFIX))
				{
					freeSitesCur[rowIndex][index].width-=inst->getWidth();

				}
				if(overlapMOV||overlapFIX)
				{
					    instOutRows[rowIndex]->push_back(inst);
						//removeInst(inst);
						unsigned long DisIndex=9000000;
						//getFreeSite(temp,freeSitesCur[rowIndex],maxSegLenCur);
						unsigned long optSiteCur=freeSitesCur[rowIndex][0].start;
						unsigned long optSiteCur1=freeSitesCur[rowIndex][0].end;
						//cout<<"optSiteCur:"<<optSiteCur<<"***"<<endl;
						//cout<<"optSiteCur1:"<<optSiteCur<<"***"<<endl;
						//cout<<"freeSiteCur.size():"<<freeSitesCur[rowIndex].size()<<"^^^"<<endl;

						for(unsigned long j=0;j<freeSitesCur[rowIndex].size();j++)
						{
								//bool flag(false);
								if(freeSitesCur[rowIndex][j].width>=inst->getWidth())
								{
										FLAG=true;
										long tempDis=(freeSitesCur[rowIndex][j].start+freeSitesCur[rowIndex][j].end)/2-(index+inst->getWidth()/2);
										//tempDis=abs(tempDis);
										if(DisIndex>abs(tempDis))
										{
												DisIndex=abs(tempDis);
												optSiteCur=freeSitesCur[rowIndex][j].start;
												optSiteCur1=freeSitesCur[rowIndex][j].end;
										}

								}

						}
						if(optSiteCur>index)
						{
								unsigned long newIndex=0;
								newIndex=optSiteCur;
								origin = getROrigin(temp, newIndex);
								inst->setOrigin(origin);
								getSite(inst, loc);
								removeInst(inst);
								continue;
						}
						if(optSiteCur1<index+num)
						{
								unsigned long newIndex=0;
								newIndex=optSiteCur1-num;
								origin=getROrigin(temp,newIndex);
								inst->setOrigin(origin);
								getSite(inst,loc);
								removeInst(inst);
								continue;
						}
						if(!FLAG)
						{
								out=true;
						}
						//out=true;
				}
				if (index + num > instOfRows[rowIndex]->size())
				{
					out = true;
				}
				if (out)
				{
					instOutRows[rowIndex]->push_back(inst);
				}
		}

		//K.close();
		//cout<<"insertInstToRow successfully!"<<endl;
		return 0;
}
*/


unsigned long fdplDetailPlace::insert(Inst *inst)
{
		fdplSiteContainer loc;

		getSite(inst, loc);
		unsigned long index;
		myRow *row;
		row = loc.getNext(index);
		if (!row)
		{
				myPoint point;
				point = inst->getOrigin();
				switch (inst->getStatus())
				{
						case Fixed:
						case FixNI:
								return 0;
								break;
						default:
								if (point.coordX() < rowArea.left()) {
										point.setCoordX(rowArea.left());
								}
								if (point.coordX() > rowArea.right()) {
										point.setCoordX(rowArea.right() - inst->getWidth());
								}
								if (point.coordY() < rowArea.bottom()) {
										point.setCoordY(rowArea.bottom());
								}
								if (point.coordY() > rowArea.top()) {
										point.setCoordY(rowArea.top() - inst->getHeight());
								}
								row = getSite(point, index);
								if (!row) {
										throw "Error : can't find the row";
								}
								break;
				}
		}

		//cout<<"Before insertInstToRow "<<endl;
		return insertInstToRow(inst, row, index);
}

void fdplDetailPlace::placeStds()
{
		vector<Inst*> over;
		unsigned long num(2);
		vector<Inst*> invalidInsts;
		unsigned long totalInstArea;
		unsigned long totalUseSiteArea;
		double rateOfArea;

		num = 10;
		unsigned long centerX=(rowArea.left() >> 1)+(rowArea.right() >> 1);//hjy20130515=====
		unsigned long centerY=(rowArea.top() >> 1)+(rowArea.bottom() >> 1);//hjy20130516=====
		for (unsigned long i = 0; i < num; i++)
		{
				//cout << "iterator [ " << i + 1 << " ]" << endl;
				//cout << "choice the overlap std cells" << endl;
				getInvalidStdInsts(invalidInsts);
				cout<<"getInvalidStdInsts successfully!"<<endl;


				//cout << "finish the choice the overlap std cells HPWL = "
				//    << myPlacement::getHPWL(plcTopBlock) << endl;
				//according the HPWL of std cells sort the std cells

				//================hjy20130530==========================
				/*
				vector<fdplInstSort> sortInsts(invalidInsts.size());
				for (unsigned long i = 0; i < invalidInsts.size(); i++)
				{
						sortInsts[i].inst = invalidInsts[i];
						//sortInsts[i].wl = getHPWL(invalidInsts[i]);
						sortInsts[i].wl=invalidInsts[i]->getNumInstTerms();
						//sortInsts[i].wl=invalidInsts[i]->getWidth();
				}
				sort(sortInsts.begin(), sortInsts.end(), Compare);
				//cout<<"sort in_placeStds successfully!"<<endl;
				for (unsigned long i = 0, j = invalidInsts.size() - 1;i < invalidInsts.size(); i++, j--)
				{
						invalidInsts[i] = sortInsts[j].inst;
				}*/
				sort(invalidInsts.begin(),invalidInsts.end(),fdplDetailPlace::CompareNumTerms);
        		//=========================================================
				for (unsigned long i = 0; i < instOfRows.size(); i++) {
						instOutRows[i]->clear();
				}

				//cout << "batch legalize invalid insts : " 
				//    << invalidInsts.size() << endl;
				place(invalidInsts, over);

				//cout << "finish batch legalize invalid insts HPWL = " 
				//    << myPlacement::getHPWL(plcTopBlock) << endl;
				rateOfArea = getRateOfArea(totalInstArea, totalUseSiteArea);
				//cout << "overlap Stat. need sites " << totalInstArea
				//    << " occupy sites " << totalUseSiteArea
				//    << " overlap rate = " << rateOfArea << endl;
				if (over.size() > 0)
				{
						maxSegLenCof -= 1.;
						if (maxSegLenCof < 1.)
						{
								maxSegLenCof = 1.;
						}
				}
				else
				{
						break;
				}
		}
		if (over.size() > 0)
		{
				cout << "place failure" << endl;
				return;
		} else
		{
				setDefaultArg();
		}
}

bool fdplDetailPlace::place(myRow *row, vector<Inst*> &throwInsts)
{
		vector<Inst*> instsInRow;
		unsigned long startIndex;
		unsigned long headSite;
		unsigned long tailSite;
		myPoint rowOrigin;
		myPoint origin;
		myPoint point;
		unsigned long numSites;
		unsigned long instsLen;
		unsigned long rowIndex = getIndex(row);
		unsigned long siteIndex;


		getInst(row, instsInRow);

		for (unsigned long i = 0; i < instsInRow.size(); i++)
		{
				switch (instsInRow[i]->getStatus())
				{
						case Fixed:
						case FixNI:
								instsInRow[i--] = instsInRow.back();
								instsInRow.pop_back();
								break;
						default:
								removeInst(instsInRow[i]);
								break;
				}
		}

		//cout<<"for_place(row*) successfully!"<<endl;

		throwInsts.clear();
		//cout<<"throwInsts.clear successfully!"<<endl;

		rowOrigin = row->getOrigin();
		numSites = row->getNumSites();
		headSite = tailSite = 0;

		//sort(instsInRow.begin(), instsInRow.end(), fdplDetailPlace::xCompare);
		sort(instsInRow.begin(),instsInRow.end(),fdplDetailPlace::xCenter);//===20130513======
		//cout<<"sort in_place(row*) successfully!"<<endl;
		//==========================hjy20130516=======================
		/*vector<fdplInstSort>sortInstCur(instsInRow.size());
		unsigned long centerx=(rowArea.left() >> 1)+(rowArea.right() >> 1);
		for(unsigned long i=0;i<instsInRow.size();i++)
		{
			sortInstCur[i].inst=instsInRow[i];
			long dis=instsInRow[i]->getCoordX()-centerx;
			if(dis<0)
				sortInstCur[i].wl=-dis;
			else
				sortInstCur[i].wl=dis;
		}
		sort(sortInstCur.begin(),sortInstCur.end(),Compare);
		for(unsigned long i=0,j=instsInRow.size()-1;i<instsInRow.size();i++,j--)
		{
			instsInRow[i]=sortInstCur[j].inst;
		}*/
		//==============================================================
		startIndex = 0;
		while (tailSite < numSites && startIndex < instsInRow.size())
		{

				for (headSite = tailSite; headSite < numSites; headSite++) {
						if (instOfRows[rowIndex]->at(headSite).type == fdplcSiteFreeType) {
								break;
						}
				}

				if (headSite >= numSites) {
						cout << "Warnning: " << __FILE__ << ":" << __LINE__
								<< "row can't contains all the insts" << endl;
				}

				for (tailSite = headSite + 1; tailSite < numSites; tailSite++) {
						if (instOfRows[rowIndex]->at(tailSite).type != fdplcSiteFreeType) {
								break;
						}
				}

				instsLen = 0;
				vector<Inst*> segInsts;
				while (startIndex < instsInRow.size())
				{
						origin = instsInRow[startIndex]->getOrigin();
						siteIndex = getSiteOffset(row, origin.coordX() - rowOrigin.coordX());
						if (siteIndex < headSite)
						{
								throwInsts.push_back(instsInRow[startIndex]);
						}
						else if (siteIndex >= tailSite)
						{
								break;
						}
						else
						{
								segInsts.push_back(instsInRow[startIndex]);
						}
						startIndex++;
				}
				//cout<<"SegInsts size:"<<segInsts.size()<<endl;
				if (segInsts.size() > 0) {
						segmentPlace(rowIndex, headSite, tailSite, segInsts);//question is here20130125
				}
				//cout<<"SegInsts after size:"<<segInsts.size()<<endl;

		}

		//cout<<"while successfully!"<<endl;
		if (startIndex < instsInRow.size()) {
				throwInsts.insert(throwInsts.end(), instsInRow.begin() + startIndex,
								instsInRow.end());
		}
		//cout<<"throwInsts.insert successfully!"<<endl;

		if (instOutRows[rowIndex]->size()) {
				string name;
				cout << "Error : " << __FILE__ << ":" << __LINE__ << endl;
				cout << "row[" << rowIndex << "] instOutRows.size() = "
						<< instOutRows[rowIndex]->size() << endl;
				for (unsigned long i = 0; i < instOutRows[rowIndex]->size(); i++) {
						Inst *inst = instOutRows[rowIndex]->at(i);
						//printStatus(inst);
				}
				//throw "Error";
		}

		for (unsigned long i = 0; i < throwInsts.size(); i++) {
				insert(throwInsts[i]);
		}

		return true;
}

bool fdplDetailPlace::place(vector<Inst*> &src, vector<Inst*> &failure)
{
		vector<fdplFreeSite> *freeSites;
		unsigned long **occupySites;
		vector<unsigned long> indexs;
		fdplSiteContainer loc;
		Inst *inst;
		myRow *row;
		unsigned long rowIndex;
		unsigned long siteIndex;
		unsigned long dstSiteIndex;
		myRow *optRow;
		unsigned long optSiteIndex;
		unsigned long curSiteIndex;
		double optDist;
		double curDist;
		unsigned long maxSegLen;
		myRow *curRow;
		myRow *nextRow;
		myRow *preRow;
		unsigned long instWidth;
		unsigned long maxInstWidth;
		set<myRow*> needPlace;
		unsigned long numOverlap;
		unsigned long numSites;


		failure.clear();
		freeSites = new vector<fdplFreeSite> [rows.size()];
		occupySites = new unsigned long* [rows.size()];
		maxInstWidth = 0;
		for (unsigned long i = 0; i < src.size(); i++)
		{

				removeInst(src[i]);
				instWidth = src[i]->getWidth();
				if (maxInstWidth < instWidth) {
						maxInstWidth = instWidth;
				}
		}
		//cout<<"for1 successfully in_place!"<<endl;


		//cout << "remove insts and place them to their opt row HPWL = "
		//<< myPlacement::getHPWL(plcTopBlock) << endl;



		//cout<<"for 2 rows.size :"<<rows.size()<<"^^^^^^^^^"<<endl;

		for (unsigned long i = 0; i < rows.size(); i++)
		{//The word "rows" means all rows in the chip

				maxSegLen = static_cast<unsigned long>(rows[i]->getNumSites() / maxSegLenCof);

					getFreeSite(rows[i], freeSites[i], maxSegLen);
					try
					{
						occupySites[i] = new unsigned long[rows[i]->getNumSites()];
					}
					catch(...)
					{
						cout<<"question is here!current i= "<<i<<endl;
										//cout<<"current j= "<<j<<endl;
						}



				for (unsigned long j = 0; j < instOfRows[i]->size(); j++)
				{
						if (instOfRows[i]->at(j).type)
						{
								occupySites[i][j] = 1;
						}
						else
						{
								occupySites[i][j] = 0;
						}
				}


		}
		//cout<<"for2 successfully in_place!"<<endl;


		unsigned long num(0);
		for (unsigned long i = 0; i < src.size(); i++)
		{
				unsigned long optFreeSiteIndex;
				unsigned long optRowIndex;
				unsigned long instNumSites;
				unsigned long siteWidth;

				inst = src[i];
				getSite(src[i], loc);

				row = loc.getNext(siteIndex);

				if (!row)
				{
						failure.push_back(src[i]);
						continue;
				}
				instWidth = src[i]->getWidth();
				optDist = 1.e100;
				optRow = 0;
				curRow = row;
				nextRow = row;
				preRow = getPreRow(row);
				double DisWL=2.e100;//===hjy20130510
				myRow* optRow1(0);//hjy20130510
				unsigned long optRowIndex1(0);//hjy20130514
				unsigned long optFreeSiteIndex1(0);//hjy20130514
				unsigned long optSiteIndex1(0);////hjy20130510

				while (curRow)
				{
						rowIndex = getIndex(curRow);
						dstSiteIndex = getSite(row, siteIndex, curRow);//getSite(row,index,res)
						numSites = curRow->getNumSites();
						instNumSites = getNumSite(inst, curRow);
						siteWidth = curRow->getSiteSpace();
						for (unsigned long j = 0; j < freeSites[rowIndex].size(); j++)
						{
								if (freeSites[rowIndex][j].width >= instWidth)
								{
										if (freeSites[rowIndex][j].start <= dstSiteIndex &&
														freeSites[rowIndex][j].end >= dstSiteIndex + 
														instNumSites)
										{
												curDist = 0;
												curSiteIndex = dstSiteIndex;
										} else if (freeSites[rowIndex][j].start > dstSiteIndex)
										{
												curDist = offsetWeight * (freeSites[rowIndex][j].start -
																dstSiteIndex) * siteWidth;
												curSiteIndex = freeSites[rowIndex][j].start;
										} else
										{
												curDist = offsetWeight * (dstSiteIndex  + instNumSites -
																freeSites[rowIndex][j].end) * siteWidth;
												curSiteIndex = freeSites[rowIndex][j].end - 
														instNumSites;
										}
										curDist += offsetWeight * abs(getRowDist(row, curRow));
										numOverlap = 0;
										unsigned long k = curSiteIndex;
										for (k = curSiteIndex; k < curSiteIndex + instNumSites && 
														k < numSites; k++)
										{
												numOverlap += occupySites[rowIndex][k];
										}
										if (k == numSites)
										{
												numOverlap += curSiteIndex + instNumSites - numSites; 
										}
										curDist += overlapWeight * numOverlap * siteWidth;

										if (curDist < optDist)
										{
												optDist = curDist;
												optRow = curRow;
												optSiteIndex = curSiteIndex;
												optRowIndex = rowIndex;
												optFreeSiteIndex = j;
												//==================hjy20130522=======================

												//insertInstToRow(src[i], optRow, optSiteIndex);
												long coordx=optSiteIndex*optRow->getSiteSpace()+optRow->getCoordX();
												src[i]->setCoordX(coordx);
												long coordy=optRow->getCoordY();
												src[i]->setCoordY(coordy);
												double WL1=getHPWL(src[i]);

												if(WL1<DisWL)
												{
													DisWL=WL1;
													optRow1=optRow;
													optRowIndex1=optRowIndex;
													optSiteIndex1=optSiteIndex;
													optFreeSiteIndex1=optFreeSiteIndex;
												}
												else{
													DisWL=WL1;
													optRow1=optRow;
													optRowIndex1=optRowIndex;
													optSiteIndex1=optSiteIndex;
													optFreeSiteIndex1=optFreeSiteIndex;
												}
												//removeInst(src[i]);
												//====================================================
										}

								}
						}

						if (optDist <= offsetWeight * abs(getRowDist(row, curRow)))
						{
								break;
						}
						if (curRow == nextRow)
						{
								curRow = preRow;
								nextRow = getNextRow(nextRow);
								if (!curRow)
								{
										curRow = nextRow;
								}
						}
						else if (curRow == preRow)
						{
								curRow = nextRow;
								preRow = getPreRow(preRow);
								if (!curRow)
								{
										curRow = preRow;
								}
						}
				}
				//if (!optRow) {
						//failure.push_back(src[i]);
						//continue;
				//}
				//insertInstToRow(src[i], optRow, optSiteIndex);
				//freeSites[optRowIndex][optFreeSiteIndex].width -= src[i]->getWidth();
				//needPlace.insert(optRow);
				//dstSiteIndex = optSiteIndex + getNumSite(inst, optRow);
				//for (unsigned long j = optSiteIndex; j < dstSiteIndex; j++)
				//{
						//occupySites[optRowIndex][j]++;
				//}
				//==================hjy20130514==================
				if (!optRow1)
				{
					failure.push_back(src[i]);
					continue;
				}
				insertInstToRow(src[i], optRow1, optSiteIndex1);
				freeSites[optRowIndex1][optFreeSiteIndex1].width -= src[i]->getWidth();
				needPlace.insert(optRow1);
				dstSiteIndex = optSiteIndex1 + getNumSite(inst, optRow1);
				for (unsigned long j = optSiteIndex1; j < dstSiteIndex; j++)
				{
					occupySites[optRowIndex1][j]++;
				}
				//===============================================
				num++;
		}

		//cout<<"for3 successfully in_place!"<<endl;

		//cout << num << " insts to optRow HPWL : " << myPlacement::getHPWL(plcTopBlock) << endl;
		cout << needPlace.size() << " rows need to relegalize" << endl;
		cout << failure.size() << " insts failure to find opt row" << endl;
		double rateOfArea;
		unsigned long totalInstArea;
		unsigned long totalUseSiteArea;
		rateOfArea = getRateOfArea(totalInstArea, totalUseSiteArea);
		//cout << "rateOfArea = " << rateOfArea
		//<< " instArea = " << totalInstArea
		//<< " useSiteArea = " << totalUseSiteArea << endl;

//*****************************************************************
		vector<Inst*> temp;

		cout<<"needPlace size: "<<needPlace.size()<<endl;

		for (set<myRow*>::iterator it = needPlace.begin();
						it != needPlace.end(); it++) {
				place(*it, temp);//question is here20130125
				failure.insert(failure.end(), temp.begin(), temp.end());

		}
//******************************************************************
		//cout<<"count: "<<count<<endl;
		//cout<<"for4 successfully in_place!"<<endl;



		delete[] freeSites;
		for (unsigned long i = 0; i < rows.size(); i++) {
				delete[] occupySites[i];
		}
		delete[] occupySites;
		//cout<<"for5 successfully in_place!"<<endl;


		//cout << failure.size() << " insts failure to place" << endl;


		for (unsigned long i = 0; i < failure.size(); i++)
		{
				insert(failure[i]);
		}
		//cout<<"for6 successfully in_place!"<<endl;


		for (unsigned long i = 0; i < src.size(); i++) {
				myPoint origin;
				string name;
				name = src[i]->getName();
				origin = src[i]->getOrigin();
				getSite(src[i], loc);
				row = loc.getNext(siteIndex);
				name = row->getName();
		}
		//cout<<"for7 successfully in_place!"<<endl;


		return true;
}
unsigned long fdplDetailPlace::getIndex(myRow *row) 
		//cost O(n) time. n = numbers of row, need to improve
{
		long index;

		map<myRow*, unsigned long>::const_iterator it(rowIndexMap.find(row));
		if (it != rowIndexMap.end()) {
				return it->second;
		} else {
				return rows.size();
		}
		index = rows.size() * (row->getCoordY() - rowArea.bottom()) / rowArea.height();
		if (index < 0) {
				index = -index;
		}
		if (index < rows.size() && rows[index] == row) {
				return index;
		}
		index = rows.size() - index - 1;
		if (index >= 0 && rows[index] == row) {
				return index;
		}
		index = rows.size() * (row->getCoordX() - rowArea.left()) / rowArea.width();
		if (index < 0) {
				index = -index;
		}
		if (index < rows.size() && rows[index] == row) {
				return index;
		}
		index = rows.size() - index - 1;
		if (index >= 0 && rows[index] == row) {
				return index;
		}

		for (unsigned long i = 0; i < rows.size(); i++) {
				if (row == rows[i]) {
						return i;
				}
		}
		return rows.size();
}

//get a myRow and the site in this myRow base on the origin
//|_______________________________________________________
//|_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _|rowGap
//|                                                       |ROW_N+1
//|                                                       |
//|_______________________________________________________|y_n+1
//|_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _|
//|                                                       |ROW_N
//|                                                       |
//|_______________________________________________________|y_n
//|_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _|
//|                                                       |ROW_N-1
//|                                                       |
//|_______________________________________________________|y_n-1
//x0                                                      x1
//if x0 <= origin.coordX() <= x1 and y_n - rowGap <= origin.coordY() < y_n+1 - rowGap
//return ROW_N
myRow *fdplDetailPlace::getSite(myPoint &origin, unsigned long &siteIndex) 
{
		long x(origin.coordX());
		long y(origin.coordY());
		myRow *row(0);
		if (!contains(rowArea, origin)) {
				return 0;
		}
		unsigned long rowIndex;

		rowIndex = rows.size() * (y - rowArea.bottom()) / rowArea.height();
		//cout<<"rowIndex: "<<rowIndex<<endl;

		if (rowIndex == rows.size()) {
				rowIndex--;
		}

		if (rowIndex < rows.size())
		{
				if (x >= rows[rowIndex]->getCoordX() && x <= rows[rowIndex]->getCoordX() + rows[rowIndex]->getWidth())
				{
						if (y >= rows[rowIndex]->getCoordY())
						{
								if (rowIndex == rows.size() - 1)
								{
										if (y < rows[rowIndex]->getCoordY() + rows[rowIndex]->getHeight() + rowGap)
										{
												row = rows[rowIndex];
										}
								}
								else
								{
										if (y < rows[rowIndex + 1]->getCoordY() - rowGap)
										{
												row = rows[rowIndex];
										}
										else if (y <= rows[rowIndex + 1]->getCoordY())
										{
												row = rows[rowIndex + 1];
										}
								}
						}
						else
						{
								if (y >= rows[rowIndex]->getCoordY() - rowGap)
								{
										row = rows[rowIndex];
								}
						}
						if (row)
						{
								//siteIndex = (unsigned long)(((row->getNumSites())*(x - rows[rowIndex]->getCoordX())) / rows[rowIndex]->getWidth());

								//============hjy20130402============================

								siteIndex=(unsigned long)(x-rows[rowIndex]->getCoordX())/rows[rowIndex]->getSiteSpace();
								//siteIndex=(unsigned long)((x-rows[rowIndex]->getCoordX())*(row->getNumSites()/rows[rowIndex]->getWidth()));
								//if((x==185650)&&(siteIndex==8))
								//  cout<<"siteIndex:"<<siteIndex<<" row->getNumSite:"<<row->getNumSites()<<" row->x:"<<rows[rowIndex]->getCoordX()<<" rowWidth:"<<rows[rowIndex]->getWidth()<<"spacing:"<<rows[rowIndex]->getSiteSpace()<<endl;
								//siteIndex=(unsigned long)((row->getNumSites())*(x-row->getCoordX())/row->getWidth());
								//siteIndex=(row->getNumSites())*((unsigned long)(x-row->getCoordX()))/row->getWidth();
								//siteIndex=(unsigned long)(x-row->getCoordX())/10;
								//unsigned long spacing;
								//spacing=row->getSiteSpace();
								//if(spacing!=10)
								//if((x==185650)&&(spacing!=10))
								//cout<<"rows[rowIndex]->getSiteSpacing: "<<spacing<<", x:"<<x<<", rowIndex:"<<rowIndex<<"row.NumSites:"<<row->getNumSites()<<endl;
								//===================================================
								return row;
						}
				}
		}

		for (vector<myRow*>::const_iterator it = rows.begin();it != rows.end(); it++)
		{
				if (y >= (*it)->getCoordY() && y < (*it)->getCoordY() + (*it)->getHeight())
				{
						row = *it;
						break;
				}
		}
		if (!row)
		{
				long min(10000000);
				long temp;
				for (vector<myRow*>::const_iterator it = rows.begin();it != rows.end(); it++)
				{
						temp = x - (*it)->getCoordX();
						if ((temp > 0 && temp < min) || (temp < 0 && -temp < min))
						{
								min = (temp < 0 ? -temp : temp);
								row = *it;
						}
						temp = x - ((*it)->getCoordX() + (*it)->getWidth());
						if ((temp > 0 && temp < min) || (temp < 0 && -temp < min))
						{
								min = (temp < 0 ? -temp : temp);
								row = *it;
						}
						temp = y - (*it)->getCoordY();
						if ((temp > 0 && temp < min) || (temp < 0 && -temp < min))
						{
								min = (temp < 0 ? -temp : temp);
								row = *it;
						}
						temp = y - ((*it)->getCoordY() + (*it)->getHeight());
						if ((temp > 0 && temp < min) || (temp < 0 && -temp < min))
						{
								min = (temp < 0 ? -temp : temp);
								row = *it;
						}
				}
		}
		if (!row)
		{
				throw "Error : No row";
		}
		siteIndex = (x - row->getCoordX()) / row->getSiteSpace();
		return row;
}

//get the fdplSiteContainer of inst, 
unsigned long fdplDetailPlace::getSite(Inst *inst, 
				fdplSiteContainer &siteCon) 
{
		siteCon.clear();
		unsigned long index;
		myPoint origin;
		myRow *row;

		origin = inst->getOrigin();

		row = getSite(origin, index);//get the origin's siteIndex and row
		//cout<<"myRow: "<<row<<endl;
		if (row == 0)
		{
				switch (inst->getStatus()) {
						case Fixed:
						case FixNI:
								return 0;
						default:
								if (origin.coordX() < rowArea.left()) {
										origin.setCoordX(rowArea.left() + 1);
								}
								if (origin.coordX() > rowArea.right()) {
										origin.setCoordX(rowArea.right() - 1);
								}
								if (origin.coordY() < rowArea.bottom()) {
										origin.setCoordY(rowArea.bottom() + 1);
								}
								if (origin.coordY() > rowArea.top()) {
										origin.setCoordY(rowArea.top() - 1);
								}
								row = getSite(origin, index);
								break;
				}
		}
		siteCon.addSite(row, index);
		if (row->getHeight() < inst->getHeight())
		{
				long height;
				myRow *nRow;
				height = inst->getHeight() - row->getHeight();
				nRow = getNextRow(row);
				if (nRow)
				{
						height = inst->getHeight() - getRowDist(row, nRow);
				}
				while (height > 0 && nRow)
				{
						index = getSite(row, index, nRow);
						siteCon.addSite(nRow, index);
						row = nRow;
						nRow = getNextRow(row);
						if (nRow == 0)
						{
								break;
						}
						height -= getRowDist(row, nRow);
				}
		}
		//==========hjy20130402==============
		//return index;

		//=====================================
		return siteCon.size();
}


unsigned long fdplDetailPlace::getSite(myRow *row, unsigned long index,
				myRow *res) 
{
		long len(0);
		long width;
		myPoint point;
		point = getROrigin(row, index);

		if (row->getCoordX() == res->getCoordX()) {
				return index;
		} 
		len = point.coordX() - res->getCoordX();
		width = res->getSiteSpace();
		if (len < 0) {
				len = -len;
		}
		return (len / width);
}


myPoint fdplDetailPlace::getROrigin(myRow *row, unsigned long siteIndex)
{
		myPoint origin;
		long offset;

		row->getOrigin();
		offset = siteIndex * (long)(row->getSiteSpace());
		origin.setCoordX(row->getCoordX() + offset);
		origin.setCoordY(row->getCoordY());
		return origin;
}

		bool
fdplDetailPlace::contains(Rect &rect, myPoint &point) 
{
		return rect.left() <= point.coordX() && rect.right() >= point.coordX()
				&& rect.bottom() <= point.coordY() && rect.top() >= point.coordY();
}

myRow *fdplDetailPlace::getNextRow(myRow *row) 
{
		vector<myRow*> *p(NULL);
		myPoint bOrigin;
		myPoint eOrigin;
		myPoint origin;
		unsigned long index;

		origin = row->getOrigin();
		p = &rows;
		if (rows.size() > 1) {
				bOrigin = rows[0]->getOrigin();
				eOrigin = rows[rows.size() - 1]->getOrigin();
				index = (rows.size() - 1) * (origin.coordY() - bOrigin.coordY()) / 
						(eOrigin.coordY() - bOrigin.coordY());
				if (index < rows.size() && rows[index] == row) {
						if (index < rows.size() - 1) {
								return rows[index + 1];
						} else {
								return NULL;
						}
				}
		} else {
				return NULL;
		}

		vector<myRow*>::const_iterator it;
		for (it = p->begin(); it != p->end(); it++) {
				if (*it == row) {
						break;
				}
		}
		if (it == p->end() || it + 1 == p->end()) {
				return 0;
		}
		return *(it + 1);
}

myRow *fdplDetailPlace::getPreRow(myRow *row) 
{
		vector<myRow*> *p(0);
		myPoint bOrigin;
		myPoint eOrigin;
		myPoint origin;
		unsigned long index;

		origin = row->getOrigin();
		p = &rows;
		if (rows.size() > 1) {
				bOrigin = rows[0]->getOrigin();
				eOrigin = rows[rows.size() - 1]->getOrigin();
				index = (rows.size() - 1) * (origin.coordY() - bOrigin.coordY()) / 
						(eOrigin.coordY() - bOrigin.coordY());
				if (index < rows.size() && rows[index] == row) {
						if (index > 0) {
								return rows[index - 1];
						} else {
								return 0;
						}
				}
		} else {
				return 0;
		}

		vector<myRow*>::const_iterator it;
		for (it = p->begin(); it != p->end(); it++) {
				if (*it == row) {
						break;
				}
		}
		if (it == p->end() || it == p->begin()) {
				return 0;
		}
		return *(it - 1);
}

long fdplDetailPlace::getRowDist(myRow *row1, myRow *row2) 
{
		myPoint point1;
		myPoint point2;

		point1 = row1->getOrigin();
		point2 = row2->getOrigin();
		return point2.coordY() - point1.coordY();
}

inline unsigned long fdplDetailPlace::getNumSite(Inst *inst, 
				myRow *row)
{
		unsigned long width;
		unsigned long siteWidth;

		width = inst->getWidth();
		siteWidth = row->getSiteSpace();
		return width / siteWidth + (width % siteWidth == 0 ? 0: 1);
}



unsigned long fdplDetailPlace::getFreeSite(myRow *row, 
				vector<fdplFreeSite> &freeSites, unsigned long maxNumSites)
{
		unsigned long rowIndex;
		vector<fdplSiteContent> *p;
		fdplFreeSite fSite;
		Inst *preInst(0);
		Inst *inst;
		bool isMaxLen;
		bool isBorder;

		rowIndex = getIndex(row);
		p = instOfRows[rowIndex];
		freeSites.clear();
		fSite.start = 0;
		fSite.end = 0;
		fSite.width = 0;
		for (unsigned long i = 0; i < p->size(); i++)
		{
				isBorder = false;
				isMaxLen = false;
				inst = p->at(i).obj.inst;
				if (static_cast<unsigned long>(i - fSite.start) == maxNumSites) {
						isMaxLen = true;
				} else if (p->at(i).type == fdplcSiteBlockageType ||
								p->at(i).type == fdplcSiteInstType) {
						isBorder = true;
				} else if (p->at(i).type == fdplcSiteFreeType) {
						fSite.width++;
				} 
				if (isBorder || isMaxLen) {
						fSite.end = i;
						if (fSite.width > 0) {
								fSite.width *= row->getSiteSpace();
								freeSites.push_back(fSite);
						}
						if (isBorder) {
								fSite.start = i + 1;
						} else {
								fSite.start = i--;
						}
						fSite.width = 0;
				}
		}
		if (fSite.width > 0)
		{
				fSite.end = p->size();
				fSite.width *= row->getSiteSpace();
				freeSites.push_back(fSite);
		}
		return freeSites.size();
}


//================================hjy20130505====================================
/*
unsigned long fdplDetailPlace::getFreeSite(myRow *row,
				vector<fdplFreeSite> &freeSites, unsigned long maxNumSites)
{
		unsigned long rowIndex;
		vector<fdplSiteContent> *p;
		fdplFreeSite fSite;
		Inst *preInst(0);
		Inst *inst;
		bool isMaxLen;
		bool isBorder;

		rowIndex = getIndex(row);
		p = instOfRows[rowIndex];
		freeSites.clear();
		fSite.start = 0;
		fSite.end = 0;
		fSite.width = 0;
		for (unsigned long i = 0; i < p->size(); i++)
		{
				isBorder = false;
				isMaxLen = false;
				inst = p->at(i).obj.inst;
				if (static_cast<unsigned long>(i - fSite.start) == maxNumSites)
				{
						isMaxLen = true;
				}
				else if (p->at(i).type == fdplcSiteBlockageType)
				{
						isBorder = true;
				}
				else if (p->at(i).type == fdplcSiteFreeType||p->at(i).type==fdplcSiteInstType)
				{
						fSite.width++;
				}
				if (isBorder || isMaxLen)
				{
						fSite.end = i;
						if (fSite.width > 0) {
								fSite.width *= row->getSiteSpace();
								freeSites.push_back(fSite);
						}
						if (isBorder)
						{
								fSite.start = i + 1;
						}
						else
						{
								fSite.start = i--;
						}
						fSite.width = 0;
				}
		}
		if (fSite.width > 0) {
				fSite.end = p->size();
				fSite.width *= row->getSiteSpace();
				freeSites.push_back(fSite);
		}
		return freeSites.size();
}
*/
//===========================================================================





//=============================hjy20130503=====================================

unsigned long fdplDetailPlace::getInvalidStdInsts(vector<Inst*> &invalidStdInsts)
{
		invalidStdInsts.clear();
		for (unsigned long i = 0; i < instOutRows.size() ; i++) {
				for (unsigned long j = 0; j < instOutRows[i]->size() ; j++) {
						invalidStdInsts.push_back(instOutRows[i]->at(j));
				}
		}
		//cout << "get " << invalidStdInsts.size() << " invalid std cells" << endl;
		return invalidStdInsts.size();
}


/*
unsigned long fdplDetailPlace::getInvalidStdInsts(vector<Inst*> &invalidStdInsts)
{
	vector<Inst*> stdInsts;
	invalidStdInsts.clear();
	stdInsts=stdCells;
	//sort(stdInsts.begin(),stdInsts.end(),fdplDetailPlace::xCompare);
	for(vector<Inst*>::iterator ite=stdInsts.begin();ite!=stdInsts.end();ite++)
	{
		removeInst(*ite);
	}
	for(vector<Inst*>::iterator ite=stdInsts.begin();ite!=stdInsts.end();ite++)
	{
		if(insert(*ite)!=PlaceValid)
		{
			//removeInst(*it);
			invalidStdInsts.push_back(*ite);
		}

	}
	return invalidStdInsts.size();
}
*/
//==============================================================================================



unsigned long fdplDetailPlace::getInst(myRow *row, vector<Inst*> &res) 
{
		unsigned long index;

		res.clear();
		index = getIndex(row);
		if (index >= rows.size()) {
				return 0;
		}
		set<Inst*> temp;
		vector<fdplSiteContent> *p(0);
		vector<fdplSiteContent>::const_iterator it;
		p = instOfRows[index];
		for (it = p->begin(); it != p->end(); it++) {
				if (it->type & fdplcSiteInstType) {
						temp.insert(it->obj.inst);
				}
		}
		temp.insert(instOutRows[index]->begin(), instOutRows[index]->end());
		if (temp.size() > 0) {
				res.insert(res.begin(), temp.begin(), temp.end());
				sort(res.begin(), res.end(), xCompare);
		}
		return res.size();
}

inline unsigned long fdplDetailPlace::getSiteOffset(myRow *row, long offset)
{
		unsigned long siteWidth;
		unsigned long res;

		siteWidth = row->getSiteSpace();
		if (offset < 0) {
				res = -((-offset) / siteWidth) - ((-offset) % siteWidth == 0 ? 0 : 1);
		} else {
				res = offset / siteWidth + (offset % siteWidth == 0 ? 0 : 1);
		}
		return res;
}

void fdplDetailPlace::segmentPlace(unsigned long rowIndex, unsigned long startIndex, 
				unsigned long endIndex, vector<Inst*> &instsInSeg)
{
		Line *lines = new Line[instsInSeg.size()];
		myRow *row(rows[rowIndex]);
		myPoint origin;

		origin = getROrigin(row, 0);
		for (size_t i = 0; i < instsInSeg.size(); i++)
		{
				removeInst(instsInSeg[i]);
				lines[i].x = getSiteOffset(row,instsInSeg[i]->getCoordX()-origin.coordX());
				lines[i].w = getNumSite(instsInSeg[i], row);

		}

		int move;
		if ((move = minMoveLine(lines, instsInSeg.size(), startIndex, endIndex))
						< 0) {
				cout << "can't remove the overlap"
						<< __FILE__ << ":" << __LINE__ << endl;
				throw "Error";
		}
		myPoint point;

		for (size_t i = 0; i < instsInSeg.size(); i++)
		{

				origin = instsInSeg[i]->getOrigin();//insts' coordinate

				point = getROrigin(row, lines[i].x);
				origin.setCoordX(point.coordX());
				instsInSeg[i]->setOrigin(origin);
				insert(instsInSeg[i]);

				for (unsigned long j = lines[i].x; j < lines[i].x + lines[i].w; j++)
				{

						if ((!((*instOfRows[rowIndex])[j].type & fdplcSiteInstType)) ||
										((*instOfRows[rowIndex])[j].obj.inst != instsInSeg[i]))
						{//both are 1,so question is here20130125

								cout<<"instsInSeg["<<i<<"].name="<<instsInSeg[i]->getName()<<endl;
								cout<<"i="<<i<<" j="<<j<<" rowIndex="<<rowIndex<<endl<<endl;
								cout<<"SitesNum: "<<row->getNumSites()<<endl;
								cout << "Error: "<< __FILE__ << ":" << __LINE__ << endl;

								//printStatus(instsInSeg[i]);


								/*	for (size_t k = 0; k < instsInSeg.size(); k++)
									{
									cout << "index = " << k
									<< " x = " << lines[k].x
									<< " w = " << lines[k].w
									<< endl;
									}
								 */
								throw "Error";
						}
				}


		}

		delete[] lines;
}

int minMoveGD(Line *lines, size_t num, int lower, int upper)
{
		size_t first(0);
		size_t last(num - 1);
		size_t head;
		size_t tail;
		int len(0);
		int left(lower);
		int right(upper);
		int totalMove(0);
		IndexSort<int> *leftIndex;
		int overlap(0);

		if (num < 1) {
				return true;
		}
		leftIndex = new IndexSort<int>[num];
		for (size_t i = 0; i < num; i++) {
				leftIndex[i].index = i;
				leftIndex[i].val = lines[i].x;
				len += lines[i].w;
		}
		if (len > upper - lower) {
				return -1;
		}
		sort(leftIndex, leftIndex + num, IndexSort<int>::compare);
		for (size_t i = 0; i < num; i++) {
				head = leftIndex[i].index;
				int rb = lines[head].x + lines[head].w;
				for (size_t j = i + 1; j < num; j++) {
						tail = leftIndex[j].index;
						if (lines[tail].x < rb) {
								overlap += rb - lines[tail].x;
						} else {
								break;
						}
				}
		}
		while (first < last) {
				head = leftIndex[first++].index;
				tail = leftIndex[last--].index;
				lines[head].x < lower ? lines[head].x = lower : 0;
				lines[tail].x + lines[tail].w > upper ? 
						lines[tail].x = upper - lines[tail].w : 0;

				left = lines[head].x;
				right = lines[tail].x + lines[tail].w;
				if (right - left < len) {
						int leftMove(0);
						int rightMove(0);
						int gap(len - (right - left));
						int cost[2] = {1, 1};
						(cost[0] + cost[1] == 0) ? cost[0] = cost[1] = 1 : 0;
						leftMove = gap * cost[1] / (cost[0] + cost[1]);
						rightMove = gap - leftMove;
						if (leftMove > left - lower) {
								leftMove = left - lower;
								rightMove = gap - leftMove;
						} else if (rightMove > upper - right) {
								rightMove = upper - right;
								leftMove = gap - rightMove;
						}
						lines[head].x -= leftMove;
						lines[tail].x += rightMove;
				}
				lower = lines[head].x + lines[head].w;
				upper = lines[tail].x;
				len -= lines[head].w + lines[tail].w;
		}
		if (first == last) {
				head = leftIndex[first].index;
				lines[head].x < lower ? lines[head].x = lower : 0;
				lines[head].x + lines[head].w > upper ?
						lines[head].x = upper - lines[head].w : 0;
		}
		totalMove = 0;
		for (size_t i = 0; i < num; i++) {
				len = lines[leftIndex[i].index].x - leftIndex[i].val;
				totalMove += (len > 0 ? len : -len);
		}
		delete[] leftIndex;
		return totalMove;
}

void fdplDetailPlace::printStatus(Inst *inst)
{
		myRow *row;
		fdplSiteContainer loc;
		unsigned long rowIndex;
		unsigned long siteIndex;
		unsigned long endSiteIndex;
		string name;
		vector<fdplSiteContent> *p;
		myPoint origin;

		name = inst->getName();
		origin = inst->getOrigin();
		cout << "inst : " << name 
				<< " origin(" << origin.coordX() << "," << origin.coordY() << ")"
				<< " w = " << inst->getWidth() 
				<< " h = " << inst->getHeight()
				<< " pStatus = " << inst->getStatus() << endl;
		getSite(inst, loc);
		while ((row = loc.getNext(siteIndex)))
		{
				name = row->getName();
				rowIndex = getIndex(row);
				p = instOfRows[rowIndex];
				//endSiteIndex = siteIndex + getNumSite(inst, row);
				endSiteIndex = siteIndex + getNumSite(inst, row);
				cout << "row : " << name << " total site = " << row->getNumSites()
						<< " inst Site " << siteIndex << "-" << endSiteIndex << endl;
				for (unsigned long i = siteIndex;i < endSiteIndex && i < (unsigned long)(row->getNumSites()); i++)
				{
						if ((*p)[i].type == 0)
						{
								cout << i << ":";
								cout << "NULL,";
						}
						else if((*p)[i].type == fdplcSiteBlockageType)
						{
								cout << i << ":";
								cout << "BLK,";
						}
						else if ((*p)[i].type == fdplcSiteInstType)
						{
								cout << i << ":";
								if ((*p)[i].obj.inst)
								{
										name = (*p)[i].obj.inst->getName();
										cout << name << ",";
								}
								else
								{
										cout <<"NULL,";
								}
						}
				}
				cout << endl;
		}
		cout<<endl<<endl;
}

inline fdplInstSort::fdplInstSort() :
		inst(0)
{
}

inline fdplInstSort::fdplInstSort(Inst *cell) :
		inst(cell)
{
}

inline void fdplSiteContainer::addSite(myRow *row, unsigned long siteIndex)
{
		//cout<<"addSite: "<<siteIndex<<endl;
		rows.push_back(row);
		rowSiteIndex.push_back(siteIndex);
}
inline unsigned long fdplSiteContainer::getCurIndex()
{
		return curIndex;
}
inline myRow *fdplSiteContainer::getNext(unsigned long &siteIndex)
{
		//cout<<"curIndex: "<<curIndex<<" ";

		if (curIndex >= rows.size())
		{
				return 0;
		}
		siteIndex = rowSiteIndex[curIndex];
		//cout<<"siteIndex: "<<siteIndex<<endl;
		return rows[curIndex++];
}
inline myRow *fdplSiteContainer::first(unsigned long &siteIndex)
{
		if (rows.size() == 0) {
				return 0;
		}
		siteIndex = rowSiteIndex[0];
		return rows[0];
}
inline myRow *fdplSiteContainer::last(unsigned long &siteIndex)
{
		if (rows.size() == 0) {
				return 0;
		}
		siteIndex = rowSiteIndex.back();
		return rows.back();
}

inline unsigned long fdplSiteContainer::size()
{
		return rows.size();
}
inline void fdplSiteContainer::reset()
{
		curIndex = 0;
}
inline void fdplSiteContainer::clear()
{
		rows.clear();
		rowSiteIndex.clear();
		curIndex = 0;
}

inline fdplSiteContent::fdplSiteContent():
		type(0)
{
		obj.inst = 0;
}

void fdplDetailPlace::init()
{
		vector<fdplSiteContent> *p(0);
		vector<Inst*> *op(0);
		instOfRows.resize(rows.size());
		instOutRows.resize(rows.size());
		if ((p = new vector<fdplSiteContent>[rows.size()]) == 0) {
				throw "Error : lack memory";
		}
		if ((op = new vector<Inst*>[rows.size()]) == 0) {
				throw "Error : lack memory";
		}
		for (unsigned long i = 0; i < rows.size(); i++) {
				instOfRows[i] = p + i;
				p[i].resize(rows[i]->getNumSites());
				for (unsigned long j = 0; j < p[i].size(); j++) {
						p[i][j] = fdplSiteContent();
				}
				instOutRows[i] = op + i;
		}

		for (vector<Inst*>::iterator it = insts.begin();it != insts.end(); it++)
		{

				if ((*it)->getStatus() == Fixed ||(*it)->getStatus() == FixNI)
				{
						pins.push_back(*it);
				}
				//=======================hjy20130416=======================
				/*else
				  {
				  stdCells.push_back(*it);
				  }*/
				else if((*it)->getHeight()>plcTopBlock->getRows()[0]->getHeight())
				{
						Macros.push_back(*it);
				}
				else
				{
						stdCells.push_back(*it);
				}
				//===========================================================
		}
		cout<<"pins.size: "<<pins.size()<<endl;
		cout<<"Macros.size:"<<Macros.size()<<endl;
		cout<<"stdCells.size:"<<stdCells.size()<<endl;
}

void fdplDetailPlace::refineOrient(){
	for (long i = 0; i < plcTopBlock->getNumRows(); ++i){
		myRow* rowNow = plcTopBlock->getRows()[i];
		rowNow->setCoordY(rowNow->getCoordYOrigin());
		vector<Inst*> instOfThisRow;
		getInst(rowNow, instOfThisRow);
		for (long j = 0; j < instOfThisRow.size(); ++j){
			Inst* inst = instOfThisRow[j];
			inst->setCoordY(rowNow->getCoordYOrigin());
			switch (rowNow->getSiteOrient()){
			case kR0:
				if (inst->getOrient() == kR0 || inst->getOrient() == kMY){
					//do nothing
				}
				else if (inst->getOrient() == kR180){
					setOrient(inst, kMY);
				}
				else if (inst->getOrient() == kMX){
					setOrient(inst, kR0);
				}
				else{
					cout<<"[WARNING]: uncommon inst orientation!!! "<<inst->getOrient()<<endl;
				}
				break;
			case kR90:
				cout<<"[WARNING]: (in dplace.cpp) uncommon site orientation kR90!!!"<<endl;
				//setOrient(inst, kR90);
				break;
			case kR180:
				//setOrient(inst, kR180);
				cout<<"[WARNING]: (in dplace.cpp) uncommon site orientation kR180!!!"<<endl;
				break;
			case kR270:
				//setOrient(inst, kR270);
				cout<<"[WARNING]: (in dplace.cpp) uncommon site orientation kR270!!!"<<endl;
				break;
			case kMY:
				//setOrient(inst, kMY);
				cout<<"[WARNING]: (in dplace.cpp) uncommon site orientation kMY!!!"<<endl;
				break;
			case kMYR90:
				//setOrient(inst, kMYR90);
				cout<<"[WARNING]: (in dplace.cpp) uncommon site orientation kMYR90!!!"<<endl;
				break;
			case kMX:
				if (inst->getOrient() == kR180 || inst->getOrient() == kMX){
					//do nothing
				}
				else if (inst->getOrient() == kR0){
					setOrient(inst, kMX);
				}
				else if (inst->getOrient() == kMY){
					setOrient(inst, kR180);
				}
				else{
					cout<<"[WARNING]: uncommon inst orientation!!! "<<inst->getOrient()<<endl;
				}
				setOrient(inst, kMX);
				break;
			case kMXR90:
				//setOrient(inst, kMXR90);
				cout<<"[WARNING]: (in dplace.cpp) uncommon site orientation kMXR90!!!"<<endl;
				break;
			default:
				cout<<"unknown row orient!!! "<<rowNow->getSiteOrient()<<endl;
			}
		}
	}
}



//==================hjy20130115add=======================
void fdplDetailPlace::guiFile_LG(const char *fname){
		//==========================================================
		//void fdplDetailPlace::guiFile(char* fname) {
		ofstream gpFile(fname);
		string cell = string(fname) + "_cell";
		gpFile << "set title \" ------Placement Result------ \"" << endl;
		gpFile << "set xrange [" << blockX << ":"
				<< blockX + blockW << "]" << endl; 
		gpFile << "set yrange [" << blockY << ":"
				<< blockY + blockH << "]" << endl;
		/*gpFile << "set xtics [" << blockX << "," 
		  << blockX + blockW << "]" << endl;        
		  gpFile << "set ytics [" << blockY << ":" 
		  << blockY + blockH << "]" << endl*/;
		gpFile << "plot \""<<cell<<"\" with steps" << endl;

		const char* fname_cell = cell.c_str();
		ofstream cFile(fname_cell);
		gpFile << endl << endl;
		unsigned j;
		for(j = 0; j < insts.size(); j++)
		{
				cFile <<"# Cell Name "<<insts[j] -> getName()<<endl;

				cFile << setw(10) << insts[j] -> getCoordX() << "  "
						<< setw(10) << insts[j] -> getCoordY() << "   "
						<< endl;
				cFile << setw(10) << insts[j] -> getCoordX() + insts[j] -> getWidth() << "  "
						<< setw(10) << insts[j] -> getCoordY() + insts[j] -> getHeight() << "   "
						<< endl;
				cFile << setw(10) << insts[j] -> getCoordX() + insts[j] -> getWidth() << "  "
						<< setw(10) << insts[j] -> getCoordY() + insts[j] -> getHeight() << "   "
						<< endl;
				cFile << setw(10) << insts[j] -> getCoordX() << "  "
						<< setw(10) << insts[j] -> getCoordY() << "   "
						<< endl;
				cFile << endl << endl;
		}
		cFile.close();
		gpFile.close();
}

//=============hjy20130417============================
/*void fdplDetailPlace::placeMacros()
  {
  vector<Inst*> buf;
  int rowHeight;
  vector<int> snapRows;
  fdplSiteContainer loc;
  fdplInstCluster instsCluster;

  for (vector<Inst*>::iterator it = macros.begin();
  it != macros.end(); it++) {
  removeInst(*it);
  }
  rowHeight = getHeight(rows[0]);
  instsCluster.setMergeDist(2 * rowHeight + 1);
  for (map<int, vector<Inst*>* >::reverse_iterator it =
  macrosByHeight.rbegin(); it != macrosByHeight.rend(); it++) {
  cout << "invalid myRow:";
  for (unsigned long i = 0; i < rows.size(); i++) {
  if (instOutRows[i]->size() > 0) {
  cout << i << " ";
  }
  }cout <<endl;
  for (map<int, vector<Inst*>* >::reverse_iterator j =
  macrosByHeight.rbegin(); j != it; j++) {
  setPlacementStatus(*(j->second), oacPlacedPlacementStatus);
  }
  cout << "place macros Height = " << it->first
  << " Num = " << it->second->size() << endl;
  cout << "remove loverlap with more big macros" << endl;
  removeMacrosOverlap(it->first);
  for (map<int, vector<Inst*>* >::reverse_iterator j =
  macrosByHeight.rbegin(); j != it; j++) {
  setPlacementStatus(*(j->second), oacLockedPlacementStatus);
  }
  cout << "begin to place..." << endl;
  placeMacros(it->first);
  cout << "finish place the macros Height = " << it->first << endl;
  }
  }

  void fdplDetailPlace::placeMacros(int macroHeight)
  {
  vector<Inst*> *pMacros;
  vector<Inst*> buf;
  vector<Inst*> invalidInsts;;
  vector<int> snapRows;
  int rowHeight;
  fdplInstCluster instsCluster;

  if (macrosByHeight.find(macroHeight) == macrosByHeight.end()) {
  return;
  }
  pMacros = macrosByHeight[macroHeight];
  rowHeight = getHeight(rows[0]);
  instsCluster.setMergeDist(2 * rowHeight + 1);

  int num = 1;
  while (num <= 5) {
  cout << "iterator [" << num++ << "]" << endl;
  buf.clear();
  buf.insert(buf.end(), pMacros->begin(), pMacros->end());
  if (getInvalidMacros(buf, invalidInsts) == 0) {
  break;
  }
  cout << invalidInsts.size() << " insts is invalid" << endl;
  cout << "begin cluster..." << endl;
  instsCluster.init(pMacros->begin(), pMacros->end(), instsCluster,
  &fdplInstCluster::compare);
  cout << "get " << instsCluster.getNumCluster() << " cluster" << endl;
  for (size_t i = 0; i < instsCluster.getNumCluster(); i++) {
  cout << "place cluster " << i + 1 << " NumInsts = "
<< instsCluster.getCluster()[i]->size()<< endl;
buf.clear();
buf.insert(buf.end(), instsCluster.getCluster()[i]->begin(),
				instsCluster.getCluster()[i]->end());
if (getInvalidMacros(buf, invalidInsts) == 0) {
		continue;
}
bool suc = true;;
if (invalidInsts.size() < 3) {
		for (size_t i = 0; i < invalidInsts.size(); i++) {
				if (!removeOverlap(invalidInsts[i])) {
						suc = false;
						break;
				}
		}
} else {
		suc = false;
}
if (suc) {
		continue;
}
buf.insert(buf.end(), invalidInsts.begin(), invalidInsts.end());
snapInsts(buf);

setPlacementStatus(*pMacros, oacLockedPlacementStatus);
setPlacementStatus(buf, oacPlacedPlacementStatus);
placeMacros(buf);
setPlacementStatus(*pMacros, oacPlacedPlacementStatus);
}
}
}

// all the Inst* in the src must be placed to grid
// all the Inst* has the same height
void fdplDetailPlace::placeMacros(vector<Inst*> &src)
{
		vector<Inst*> invalidInsts;;
		vector<Inst*> throwInsts;
		fdplSiteContainer loc;
		myRow *row;
		int siteIndex;
		unsigned long rowIndex;
		int rowStep;
		int instHeight;
		map<unsigned long, vector<fdplFreeSite>*> freeSites;
		map<unsigned long, vector<Inst*>*> segments;
		vector<Inst*> *pSeg;
		int maxSegLen;
		int numSites;

		if (src.size() == 0) {
				return;
		}
		instHeight = getHeight(src[0]);
		getSite(src[0], loc);
		if (getInvalidMacros(src, invalidInsts) == 0) {
				return;
		}
		row = loc.getNext(siteIndex);
		rowStep = loc.size();
		rowIndex = getIndex(row) % rowStep;
		numSites = row->getNumSites();
		while (rowIndex + rowStep <= rows.size()) {
				freeSites[rowIndex] = new vector<fdplFreeSite>;
				rowIndex += rowStep;
		}
		for (map<unsigned long, vector<fdplFreeSite>*>::iterator it = freeSites.begin();
						it != freeSites.end(); it++) {
				maxSegLen = numSites;
				getFreeSite(rows[it->first], rowStep, *(it->second),
								fdplcFixed, maxSegLen);
		}

		int instWidth;
		int siteWidth;
		int numInstSites;
		int j = rowStep;
		myRow *curRow;
		int curRowIndex;
		double cost;
		double minCost = 1.e100;
		myPoint origin;
		myPoint point;
		unsigned long optRowIndex = rows.size();
		unsigned long optFreeSiteIndex;
		int optSiteIndex;
		int curSiteIndex;
		vector<fdplFreeSite> *pFreeSite;

		for (vector<Inst*>::iterator it = invalidInsts.begin();
						it != invalidInsts.end(); it++) {
				minCost = 1.e100;
				getSite(*it, loc);
				row = loc.getNext(siteIndex);
				rowIndex = getIndex(row);
				curRowIndex = rowIndex;
				optRowIndex = rows.size();
				instWidth = getWidth(*it);
				j = rowStep;
				while (curRowIndex >= 0 &&
								curRowIndex + rowStep <= (int)rows.size()) {
						curRow = rows[curRowIndex];
						pFreeSite = freeSites[curRowIndex];
						if (!pFreeSite) {
								//printStatus(*it);
								cout << "curRowIndex = " << curRowIndex << endl;
								cout << freeSites.size() << " freeSites:" << endl;
								for (map<unsigned long, vector<fdplFreeSite>*>::iterator k =
												freeSites.begin(); k != freeSites.end(); k++) {
										cout << "(" << k->first << "," << k->second << "),";
								}
								cout << endl;
						}
						numInstSites = getNumSites(*it, curRow);
						siteWidth = getSiteWidth (curRow);
						for (size_t k = 0; k < pFreeSite->size(); k++) {
								cost = minCost + 1.;
								if (pFreeSite->at(k).width >= instWidth) {
										cost = 0.;
										if (siteIndex < pFreeSite->at(k).start) {
												curSiteIndex = pFreeSite->at(k).start;
												cost += offsetWeight *
														(curSiteIndex - siteIndex) * siteWidth;
										} else if (siteIndex >
														pFreeSite->at(k).end - numInstSites) {
												curSiteIndex = pFreeSite->at(k).end - numInstSites;
												cost += offsetWeight * siteWidth *
														(siteIndex - curSiteIndex);
										} else {
												curSiteIndex = siteIndex;
										}
										cost += offsetWeight * rowStep *
												ABS((int)rowIndex - curRowIndex);
								}
								if (cost < minCost) {
										minCost = cost;
										optRowIndex = curRowIndex;
										optFreeSiteIndex = k;
										optSiteIndex = curSiteIndex;
								}
						}
						if (minCost <=
										offsetWeight * rowStep * ABS((int)rowIndex - curRowIndex)) {
								//break;
						}

						curRowIndex += j;
						j = (j > 0 ? (-j - rowStep) : (-j + rowStep));
						if (curRowIndex < 0 || curRowIndex >= (int)rows.size()) {
								curRowIndex += j;
								j = (j > 0 ? (-j - rowStep) : (-j + rowStep));
						}
				}
				if (optRowIndex == rows.size()) {
						oaString name;
						(*it)->getName(ns, name);
						cout << "can't find the row for " << name << endl;
						removeInst(*it);
						throwInsts.append(*it);
						continue;
				}
				if (!segments[optRowIndex * numSites + optFreeSiteIndex]) {
						set<Inst*> temp;
						pSeg = new vector<Inst*>;
						segments[optRowIndex * numSites + optFreeSiteIndex] = pSeg;
						pFreeSite = freeSites[optRowIndex];
						for (int i = pFreeSite->at(optFreeSiteIndex).start;
										i < pFreeSite->at(optFreeSiteIndex).end; i++) {
								if (instOfRows[optRowIndex]->at(i).type & fdplcSiteInstType) {
										temp.insert(instOfRows[optRowIndex]->at(i).obj.inst);
								}
						}
						pSeg->insert(pSeg->end(), temp.begin(), temp.end());
				}
				segments[optRowIndex * numSites + optFreeSiteIndex]->append(*it);
				insertInstToRow(*it, rows[optRowIndex], optSiteIndex);
				freeSites[optRowIndex]->at(optFreeSiteIndex).width -= instWidth;
		}

		for (map<unsigned long, vector<Inst*>*>::iterator it = segments.begin();
						it != segments.end(); it++) {
				optFreeSiteIndex = it->first % numSites;
				rowIndex = it->first / numSites;
				pFreeSite = freeSites[rowIndex];
				segmentPlace(rowIndex, pFreeSite->at(optFreeSiteIndex).start,
								pFreeSite->at(optFreeSiteIndex).end, *(it->second));
				delete it->second;
		}


		for (map<unsigned long, vector<fdplFreeSite>*>::iterator it = freeSites.begin();
						it != freeSites.end(); it++) {
				delete it->second;
		}
}

bool fdplDetailPlace::removeOverlap(Inst *inst)
{
		bool res = false;
		vector<fdplSiteContent> *pRowInst;
		set<Inst*> overlaps;
		fdplSiteContainer loc;
		myRow *row;
		int siteIndex;
		unsigned long rowIndex;
		int numSites;
		int numInstSites;

		if (insert(inst) == fdplcValidPlace) {
				return true;
		}
		getSite(inst, loc);
		while (row = loc.getNext(siteIndex)) {
				rowIndex = getIndex(row);
				pRowInst = instOfRows[rowIndex];
				numSites = row->getNumSites();
				numInstSites = MIN(numSites,
								siteIndex + (int)getNumSites(inst, row));
				for (int i = siteIndex; i < numInstSites; i++) {
						if (pRowInst->at(i).type & fdplcSiteBlockageType) {
								return false;
						} else if (pRowInst->at(i).type & fdplcSiteInstType &&
										pRowInst->at(i).obj.inst != inst) {
								overlaps.insert(pRowInst->at(i).obj.inst);
						}
				}
		}
		vector<Inst*> overlapInsts;
		Rect box[5];
		myPoint origin[5];
		myPoint point[5];
		int xOverlap;
		int yOverlap;
		Inst *opInst;

		overlapInsts.insert(overlapInsts.end(), overlaps.begin(), overlaps.end());
		switch (overlapInsts.size()) {
				case 1:
						removeInst(inst);
						opInst = overlapInsts[0];
						if (opInst->getPlacementStatus() !=
										oacFixedPlacementStatus &&
										opInst->getPlacementStatus() !=
										oacLockedPlacementStatus) {
								removeInst(opInst);
						}
						origin[0] = getOrigin(inst);
						origin[1] = getOrigin(opInst);
						box[0] = getBBox(inst);
						box[1] = getBBox(opInst);
						if (box[0].left() < box[1].left() + (int)box[1].getWidth() / 2) {
								xOverlap = box[1].left() - box[0].right() ;
						} else {
								xOverlap = box[1].right() - box[0].left();
						}
						if (box[0].bottom() < box[1].bottom() +
										(int)box[1].getHeight() / 2) {
								yOverlap = box[1].bottom() - box[0].top();
						} else {
								yOverlap = box[1].top() - box[0].bottom();
						}
						xOverlap = roundSiteWidth(xOverlap);
						yOverlap = roundRowHeight(yOverlap);
						if (ABS(xOverlap) <= ABS(yOverlap)) {
								point[0] = origin[0];
								point[0].x() += xOverlap;
								setOrigin(inst, point[0]);
								if (rowArea.contains(getBBox(inst)) &&
												insert(inst) == fdplcValidPlace) {
										res = true;
										insert(overlapInsts[0]);
								} else {
										removeInst(inst);
										setOrigin(inst, origin[0]);
										if (overlapInsts[0]->getPlacementStatus() !=
														oacFixedPlacementStatus &&
														overlapInsts[0]->getPlacementStatus() !=
														oacLockedPlacementStatus) {
												point[1] = origin[1];
												point[1].x() -= xOverlap;
												setOrigin(overlapInsts[0], point[1]);
												if (rowArea.contains(getBBox(overlapInsts[0])) &&
																insert(overlapInsts[0]) == fdplcValidPlace) {
														res = true;
														insert(inst);
												} else {
														removeInst(overlapInsts[0]);
														setOrigin(overlapInsts[0], origin[1]);
												}
										}
								}
						} else {
								point[0] = origin[0];
								point[0].y() += yOverlap;
								setOrigin(inst, point[0]);
								if (rowArea.contains(getBBox(inst)) &&
												insert(inst) == fdplcValidPlace) {
										res = true;
										insert(overlapInsts[0]);
								} else {
										removeInst(inst);
										setOrigin(inst, origin[0]);
										if (overlapInsts[0]->getPlacementStatus() !=
														oacFixedPlacementStatus &&
														overlapInsts[0]->getPlacementStatus() !=
														oacLockedPlacementStatus) {
												point[1] = origin[1];
												point[1].y() -= yOverlap;
												setOrigin(overlapInsts[0], point[1]);
												if (rowArea.contains(getBBox(overlapInsts[0])) &&
																insert(overlapInsts[0]) == fdplcValidPlace) {
														res = true;
														insert(inst);
												} else {
														removeInst(overlapInsts[0]);
														setOrigin(overlapInsts[0], origin[1]);
												}
										}
								}
						}
						if (res) {
								break;
						}
						if (ABS(xOverlap) > ABS(yOverlap)) {
								point[0] = origin[0];
								point[0].x() += xOverlap;
								setOrigin(inst, point[0]);
								if (rowArea.contains(getBBox(inst)) &&
												insert(inst) == fdplcValidPlace) {
										res = true;
										insert(overlapInsts[0]);
								} else {
										removeInst(inst);
										setOrigin(inst, origin[0]);
										if (overlapInsts[0]->getPlacementStatus() !=
														oacFixedPlacementStatus &&
														overlapInsts[0]->getPlacementStatus() !=
														oacLockedPlacementStatus) {
												point[1] = origin[1];
												point[1].x() -= xOverlap;
												setOrigin(overlapInsts[0], point[1]);
												if (rowArea.contains(getBBox(overlapInsts[0])) &&
																insert(overlapInsts[0]) == fdplcValidPlace) {
														res = true;
														insert(inst);
												} else {
														removeInst(overlapInsts[0]);
														setOrigin(overlapInsts[0], origin[1]);
												}
										}
								}
						} else {
								point[0] = origin[0];
								point[0].y() += yOverlap;
								setOrigin(inst, point[0]);
								if (rowArea.contains(getBBox(inst)) &&
												insert(inst) == fdplcValidPlace) {
										res = true;
										insert(overlapInsts[0]);
								} else {
										removeInst(inst);
										setOrigin(inst, origin[0]);
										if (overlapInsts[0]->getPlacementStatus() !=
														oacFixedPlacementStatus &&
														overlapInsts[0]->getPlacementStatus() !=
														oacLockedPlacementStatus) {
												point[1] = origin[1];
												point[1].y() -= yOverlap;
												setOrigin(overlapInsts[0], point[1]);
												if (rowArea.contains(getBBox(overlapInsts[0])) &&
																insert(overlapInsts[0]) == fdplcValidPlace) {
														res = true;
														insert(inst);
												} else {
														removeInst(overlapInsts[0]);
														setOrigin(overlapInsts[0], origin[1]);
												}
										}
								}
						}
						removeInst(inst);
						if (overlapInsts[0]->getPlacementStatus() !=
										oacFixedPlacementStatus &&
										overlapInsts[0]->getPlacementStatus() !=
										oacLockedPlacementStatus) {
								insert(overlapInsts[0]);
						}
						insert(inst);
						break;
				case 2:
				case 3:
				case 4:
				default:
						break;
		}
		return res;
}
*/
//=========================================================
