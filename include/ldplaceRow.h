#ifndef ROWCLUSTER_H_
#define ROWCLUSTER_H_


/*
 class RowCluster;
 class mySubRow;
 class LdplaceRow;
 */

#include<list>
#include<vector>
#include<algorithm>
//#include "./ldpUtility.h"

class RowCluster {
	friend class Ldplace;
private:
	long index;
	long firstCellIndex, lastCellIndex; // first and last cell of a cluster
	long numCell; // number of cells in cluster
	long ec, wc, qc; // ec, wc, qc
	long clusterPos;


	long width;

private:
	long ecold, wcold, qcold;

public:
	list<Inst*> instOfCluster;
	vector<long> bounds;

	RowCluster(long num) {
		index = num;
		firstCellIndex = 0;
		lastCellIndex = 0;
		numCell = 0;
		ec = 0;
		wc = 0;
		qc = 0;
		ecold = 0;
		wcold = 0;
		qcold = 0;

		width = 0;

	}

	//add a new cell to cluster
	void addInst(Inst* inst, vector<long> boundList){
		instOfCluster.push_back(inst);

		for (long i=0; i<(long)boundList.size(); i++){
			bounds.push_back(boundList[i]);
		}
		numCell++;
		width += (long)inst->getWidth();
		clusterPos = inst->getCoordX();
		sort(bounds.begin(), bounds.end());

	}


	long getIndex() {
		return index;
	}

	void setIndex(long i){
		index = i;
	}

	long getWidth(){
		return width;
	}

	void setWidth(long w){
		width = w;
	}

	long getNumCell() {
		return numCell;
	}

	void setNumCell(long num) {
		numCell = num;
	}

	long getClusterPos() {
		return clusterPos;
	}

	void setClusterPos(long pos) {
		clusterPos = pos;
	}

};

/*
class RowCluster {
	friend class Ldplace;
private:
	long rowIndex;
	long firstCellIndex, lastCellIndex; // first and last cell of a cluster
	long numCell; // number of cells in cluster
	long ec, wc, qc; // ec, wc, qc
	long clusterPos;

private:
	long ecold, wcold, qcold;

public:
	RowCluster(long index) {
		rowIndex = index;
		firstCellIndex = 0;
		lastCellIndex = 0;
		numCell = 0;
		ec = 0;
		wc = 0;
		qc = 0;
		ecold = 0;
		wcold = 0;
		qcold = 0;
		clusterPos = 0;
	}

};
*/

class mySubRow {
	friend class Ldplace;
private:
	long rowIndex;
	long siteWidth;
	long yCoord;
	long left, right;
	long totalInstWidth;
	//long numCluster;


public:
	vector<RowCluster> clusters; 
    list<Inst *> instOfSubRow;
	//long *siteMap;

public:
	mySubRow(long index, long sWidth, long y, long l, long r) :
		rowIndex(index), siteWidth(sWidth), yCoord(y), left(l), right(r), totalInstWidth(0) {
		//long siteSize = (right - left) / siteWidth;
		//clusters.reserve(siteSize);
		//instOfSubRow.reserve(siteSize);
		//siteMap = new long[siteSize];
	}

	~mySubRow() {
		clusters.clear();
		instOfSubRow.clear();
		//delete [] siteMap;
	}

	long getRowIndex() {
		return rowIndex;
	}

  inline long getLeftBoundary() {
		return left;
	}

  inline long getRightBoundary() {
		return right;
	}
  inline void setLeftBoundary(long lt) {
    left = lt;
  }
  inline void setRightBoundary(long rt) {
    right = rt;
  }
  inline void setTotalInstWidth(long w){
	  totalInstWidth = w;
  }
  inline long getTotalInstWidth(){
	  return totalInstWidth;
  }
  

//not used
//	void increaseTotalInstWidth(Inst *inst) {
//		totalInstWidth += LdpUtility::getInstWidth(inst);
//	}

  inline long getWidth() {
		return right - left;
	}

	long getYCoord() {
		return yCoord;
	}

	long getNumCluster() {
		return clusters.size();
	}

  inline long getNumInst() {
		return instOfSubRow.size();
	}
  //with overlap return false
	bool clusterOverlap(RowCluster* a, RowCluster* b){
		if ((a->getIndex() < b->getIndex()) && ((a->getClusterPos()+a->getWidth()) > b->getClusterPos())){
			return false;
		}
		if ((a->getIndex() > b->getIndex()) && (a->getClusterPos() < (b->getClusterPos()+b->getWidth()))){
			return false;
		}
		return true;
	}


	//merge two clusters, update bounds list
	void mergeCluster(RowCluster *c1, RowCluster *c2){
		for (list<Inst*>::iterator k = c2->instOfCluster.begin(); k != c2->instOfCluster.end(); k++){
			Inst* temp = *k;
			c1->instOfCluster.push_back(temp);
		}
		long width = c1->getWidth();
		long num = c1->getNumCell();
		c1->setWidth(width+c2->getWidth());
		c1->setNumCell(num+c2->getNumCell());

		for (long i=0; i<(long)c2->bounds.size(); i++){
			c1->bounds.push_back(c2->bounds[i]);
		}
		sort(c1->bounds.begin(), c1->bounds.end());
	}


	//find optimal region center of this cluster
	void putInORC(RowCluster* rc)
	{
		long pos1 = rc->bounds[rc->bounds.size() / 2 - 1];
		long pos2 = rc->bounds[rc->bounds.size() / 2];
		long pos = (((pos1 + pos2) >> 1)+5)/10*10;
		//cout<<"ORC starts at "<<rc->bounds[rc->bounds.size() / 2 - 1]<<endl;
		//if (rc->getClusterPos() < pos || rc->getClusterPos() > rc->bounds[rc->bounds.size() / 2]){
		//	cout<<"not in ORC"<<endl;
		//}
		if (rc->getClusterPos()>=pos1 &&
				rc->getClusterPos()+rc->getWidth()<=pos2 &&
				rc->getClusterPos() >= left &&
				rc->getClusterPos()+rc->getWidth() <= right){
			//cout<<rc->getIndex()<<" already in ORC"<<endl;
			return;
		}


		if (pos + (rc->getWidth()>>1) > right){
			rc->setClusterPos(right - rc->getWidth());
			//cout<<"1put cluster "<<rc->getIndex()<<" rightB at "<<right<<endl;
			return;
			//cout<<"put cluster rightB at "<<right<<endl;
		}
		if (pos - (rc->getWidth()>>1) < left){
			rc->setClusterPos(left);
			//cout<<"2put cluster "<<rc->getIndex()<<" rightB at "<<left + rc->getWidth()<<endl;
			return;
		}
		else {
			rc->setClusterPos(((pos - (rc->getWidth()>>1))+5)/10*10);
			//cout<<"3put cluster "<<rc->getIndex()<<" rightB at "<<pos+(rc->getWidth()>>1)<<endl;
			return;

		}
		//cout<<"width: "<<rc->getWidth()<<endl;
		//cout<<"cluster "<<index<<" is placed at "<<clusterPos<<" "<<instOfCluster.front()->getCenterY()<<" width "<<width<<endl;
	}


};

class LdplaceRow {
	friend class Ldplace;
private:
	long rowIndex;
	long yCoord;
	long numSubRow;

public:
	vector<mySubRow*> subRows;

public:
	  LdplaceRow(long index, long y, long num) :
		rowIndex(index), yCoord(y), numSubRow(num) {
	//	numSubRow = 1;
	//	subRows = new mySubRow *[numSubRow];
	//	for (long i = 0; i < numSubRow; i++) {
	//		subRows[i] = NULL;
	//	}
	}

	~LdplaceRow() {
		for (long i = 0; i < numSubRow; i++) {
			delete subRows[i];
		}
	}

	long getRowIndex() {
		return rowIndex;
	}

	inline long getNumSubRow() {
		return numSubRow;
	}

	inline void setNumSubRow(long value) {
		numSubRow = value;
	}
//	inline void removeSubRow(vector<mySubRow*>::iterator subr){
//	    subRows.erase(subr);
//		delete *subr;
//	}
	
};

#endif /* ROWCLUSTER_H_ */
