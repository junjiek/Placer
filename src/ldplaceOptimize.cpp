
#include "ldplace.h"
#include "simPlPlace.h"
#include <iostream>
using namespace std;

void Ldplace::preOptimize() {

}

void Ldplace::checkABU(){
    double abuPenalty = 0.0;
    if (abu2 > block->getTargetUtil()){
    	abuPenalty += (abu2 / block->getTargetUtil() - 1) * 10.0 / 17;
    }
    if (abu5 > block->getTargetUtil()){
    	abuPenalty += (abu5 / block->getTargetUtil() - 1) * 4.0 / 17;
    }
    if (abu10 > block->getTargetUtil()){
    	abuPenalty += (abu10 / block->getTargetUtil() - 1) * 2.0 / 17;
    }
    if (abu20 > block->getTargetUtil()){
    	abuPenalty += (abu20 / block->getTargetUtil() - 1) * 1.0 / 17;
    }
    cout<<"[CHECK]before rebuildABU, abu penalty is "<<abuPenalty<<endl;
    buildABU();
    abuPenalty = 0.0;
    if (abu2 > block->getTargetUtil()){
    	abuPenalty += (abu2 / block->getTargetUtil() - 1) * 10.0 / 17;
    }
    if (abu5 > block->getTargetUtil()){
    	abuPenalty += (abu5 / block->getTargetUtil() - 1) * 4.0 / 17;
    }
    if (abu10 > block->getTargetUtil()){
    	abuPenalty += (abu10 / block->getTargetUtil() - 1) * 2.0 / 17;
    }
    if (abu20 > block->getTargetUtil()){
    	abuPenalty += (abu20 / block->getTargetUtil() - 1) * 1.0 / 17;
    }
    cout<<"[CHECK]after rebuildABU, abu penalty is "<<abuPenalty<<endl;
}

bool greaterV(const Triple& A, const Triple& B){
	return A.element > B.element;
}

bool cmpX(Inst A, Inst B){
	return A.getCoordX() > B.getCoordX();
}

bool greaterRowUtil(mySubRow* A, mySubRow* B){
	return ((double)A->getTotalInstWidth() / A->getWidth()) > ((double)B->getTotalInstWidth() / B->getWidth());
}

bool greaterPinDensity(Inst* A, Inst* B){
	return ((double)A->getNumInstTerms() / A->getArea()) > ((double)B->getNumInstTerms() / B->getArea());
}

bool greaterWidth(Inst* A, Inst* B){
	return (A->getWidth() > B->getWidth());
}

void Ldplace::initBins() {
    long blockX = region.left();
    long blockY = region.bottom();
    long blockW = region.width();
    long blockH = region.height();

    double totalCellArea = 0;//for debug
    double totalObstacleArea = 0;//for debug
    double totalArea = blockW * blockH;//for debug

    vector<Inst*> validNodes = block->getInsts();

    //build
    gridSizeX = 9 * validNodes[0]->getHeight();
    gridSizeY = 9 * validNodes[0]->getHeight();
    gridNumX = blockW / gridSizeX + 1;
    gridNumY = blockH / gridSizeY + 1;

    gridCoordX.resize(gridNumX + 1, 0);
    gridCoordY.resize(gridNumY + 1, 0);
    // X left grid and Y bottom grid
    gridCoordX[0] = blockX;
    gridCoordY[0] = blockY;
    // center grid
    for (long i = 1 ; i < gridNumX; i++) {
        gridCoordX[i] = gridCoordX[i - 1] + gridSizeX;
    }
    for (long i = 1; i < gridNumY; i++){
        gridCoordY[i] = gridCoordY[i - 1] + gridSizeY;
    }
    // X-Y direction, note the last grid
    gridCoordX[gridNumX] = blockX + blockW;
    gridCoordY[gridNumY] = blockY + blockH;

    //calculate CellArea, ObstacleArea and BinDensity
    gridDensity = new double*[gridNumX];
    gridAreaAvailable = new double*[gridNumX];
    gridCellArea = new double*[gridNumX];
    for (long i = 0 ; i < gridNumX; i++) {
        gridDensity[i] = new double[gridNumY];
        gridAreaAvailable[i] = new double[gridNumY];
        gridCellArea[i] = new double[gridNumY];
        memset(gridDensity[i], 0, sizeof(double) * gridNumY);
        memset(gridCellArea[i], 0, sizeof(double) * gridNumY);
        for (long j = 0; j < gridNumY; j++) {
            gridAreaAvailable[i][j] = (gridCoordX[i+1]-gridCoordX[i])*(gridCoordY[j+1]-gridCoordY[j]);
        }
    }
    //get all the bins density
    int numValidNodes = validNodes.size();
    for (long i = 0 ; i < numValidNodes; i++) {
        long xl = validNodes[i] -> getCoordX();
        long xh = xl + validNodes[i] -> getWidth();
        long yl = validNodes[i] -> getCoordY();
        long yh = yl + validNodes[i] -> getHeight();

        //for debug
        if (validNodes[i]->getStatus() == Moved){
        	totalCellArea += validNodes[i]->getArea();
        }
        else{
        	totalObstacleArea += validNodes[i]->getArea();
        }

        if (xh <= blockX || xl >= blockX + (long) blockW || yh <= blockY || yl
                >= blockY + (long) blockH) {
            if (validNodes[i]->getStatus() != Moved){
                continue;
            }
            else{
                cout<<"error!!! cell out of boundary!!!"<<endl;
                if (xh <= blockX) {
                    validNodes[i]->setCoordX(blockX);
                    xl = blockX;
                    xh = xl + validNodes[i]->getWidth();
                } else if (xl >= blockX + (long)blockW) {
                    validNodes[i]->setCoordX(blockX + blockW - validNodes[i]->getWidth());
                    xl = validNodes[i]->getCoordX();
                    xh = blockX + blockW;
                }
                if (yh <= blockY) {
                    validNodes[i]->setCoordY(blockY);
                    yl = blockY;
                    yh = yl + validNodes[i]->getHeight();
                } else if (yl >= blockY + (long)blockH) {
                    validNodes[i]->setCoordY(blockY + blockH - validNodes[i]->getHeight());
                    yl = validNodes[i]->getCoordY();
                    yh = blockY + blockH;
                }
            }
        }
        long xStart = (long) ((xl - blockX) / gridSizeX);
        long xEnd = (long) ((xh - blockX) / gridSizeX);
        long yStart = (long) ((yl- blockY) / gridSizeY);
        long yEnd = (long) ((yh- blockY) / gridSizeY);

        //assert(xStart >= 0);
        //assert(xEnd <= gridNumX);
        //assert(yStart >= 0);
        //assert(yEnd <= gridNumY);

        xStart = xStart < 0 ? 0 : xStart;
        yStart = yStart < 0 ? 0 : yStart;
        xEnd = xEnd < gridNumX ? xEnd : (gridNumX - 1);
        yEnd = yEnd < gridNumY ? yEnd : (gridNumY - 1);

        for (long j = xStart ; j <= xEnd ; ++j) {
            for (long k = yStart ; k <= yEnd ; ++k) {
                long left = xl > gridCoordX[j] ? xl : gridCoordX[j];
                long right = xh < gridCoordX[j+1] ? xh : gridCoordX[j+1];
                long down = yl > gridCoordY[k] ? yl : gridCoordY[k];
                long up = yh < gridCoordY[k+1] ? yh : gridCoordY[k+1];

                //assert(right >= left);
                //assert(up >= down);
                if (validNodes[i]->getStatus() == Moved){
                    gridCellArea[j][k] += ((right - left) * (up - down));
                    //assert(gridCellArea[j][k] >= 0);
                }
                else if (validNodes[i]->getStatus() == Fixed && validNodes[i]->isRect()){
                    gridAreaAvailable[j][k] -= ((right - left) * (up - down));
                    //assert(gridAreaAvailable[j][k] >= 0);
                }
            }
        }
    }
    for (long i = 0; i < gridNumX; ++i) {
        for (long j = 0; j < gridNumY; ++j) {
            //assert(gridCellArea[i][j] >= 0);
            //assert(gridAreaAvailable[i][j] >= 0);
            if (gridAreaAvailable[i][j] == 0){
                gridDensity[i][j] = -1;
            }
            else{
                gridDensity[i][j] = gridCellArea[i][j] / gridAreaAvailable[i][j];
            }
            //assert(gridDensity[i][j] >= 0 || gridDensity[i][j] == -1);
        }
    }
    buildABU();

    cout<<"[INFO]circuit utilization is "<<totalCellArea / (totalArea - totalObstacleArea)<<endl;
    cout<<(totalCellArea + totalObstacleArea) / totalArea<<endl;

    return;
}
void Ldplace::postOptimize() {
    double wl1 = myPlacement::getHPWL(block) * (1 + getABUpenalty());
	//double wl1 = myPlacement::getHPWL(block);
    double wl2 = wl1;

    cout<<"max displacement = "<<block->getMaxDisplacement()<<endl;
    cout<<"target util = "<<block->getTargetUtil()<<endl;


    double changeRate;
    double totalTime = 0;
    double totalSwapTime = 0;
    double totalReorderTime = 0;
    double totalRelaxationTime = 0;
    double thisTime;
    long i = 0;
    cout << "Original scaled HPWL : "<< wl1 <<endl<<endl;
    //fout<<wl1<<"  "<<getABUpenalty()<<"  "<<wl1 * (1 + getABUpenalty())<<endl;
    do {
        wl1 = wl2;
        time_t start = clock();

//(1) global and vertical Swap
        GVSwap();
        time_t swapdone = clock();
        cout<<"\t GVSwap time: "<<double(swapdone - start)/CLOCKS_PER_SEC<<endl;
        totalSwapTime += double(swapdone - start)/CLOCKS_PER_SEC;
//(2) localReOrdering
        localReOrdering(2);

        time_t reorderdone = clock();
        cout<<"\t re-order time: "<<double(reorderdone - swapdone)/CLOCKS_PER_SEC<<endl;
        totalReorderTime += double(reorderdone - swapdone)/CLOCKS_PER_SEC;

//(3) relaxation
        for (long j = 0; j < 5; ++j){
            cellBloating(1);
            relaxation();
            cellRestore();
            mendDisp();
            rebuildBins();
            relaxationV(1,1);
        }

        time_t bloatingdone = clock();
        cout<<"\t bloating time: "<<double(bloatingdone - reorderdone)/CLOCKS_PER_SEC<<endl;
        totalRelaxationTime += double(bloatingdone - reorderdone)/CLOCKS_PER_SEC;

        time_t end = clock();
       // wl2 = myPlacement::getHPWL(block);
        wl2 = myPlacement::getHPWL(block) * (1 + getABUpenalty());
        i++;
        changeRate = (double)(wl1-wl2)/wl1;
        thisTime = double(end - start)/CLOCKS_PER_SEC;
        totalTime += thisTime;
        cout << "iterator " << i << " scaled HPWL: " << wl2 <<endl;
        cout << "optimization rate: "<<changeRate<< endl;
        //cout << "ABU penalty: "<<getABUpenalty()<<endl;
        //fout<<wl2<<"  "<<getABUpenalty()<<"  "<<wl2 * (1+getABUpenalty())<<endl;
        cout << "iteration time: "<< thisTime <<" sec."<<endl<<endl;
    } while (changeRate >= 0.001 && i < 10);
    //fout.close();
    //TODO
    binSpaceRefine();
    //binRefineH();
    //mendDisp();
    //rebuildBins();

    assignToSites();

    cout<<"total swap time : "<<totalSwapTime<<" sec."<<endl;
    cout<<"total reordering time: "<<totalReorderTime<<" sec."<<endl;
    cout<<"total relaxation time: "<<totalRelaxationTime<<" sec."<<endl;
    cout<<"DP time: "<<totalTime<<" sec."<<endl;
}

//singleSegmentClustering
void Ldplace::singleSegmentClustering() {
    mySubRow* subRow;
    long num_old;
    long newCount;

    vector<RowCluster*> old_cluster;
    vector<RowCluster*> new_cluster;
    vector<RowCluster*> cluster;
    bool flag;
    //cout<<"clustering begin"<<endl;
    for (long s = 0; s < numRow; s++)
    {
        for (long t = 0; t < ldpRows[s]->numSubRow; t++)
        {
            old_cluster.clear();
            new_cluster.clear();
            cluster.clear();
            subRow = ldpRows[s]->subRows[t];
            num_old = 0;
            num_old = subRow->getNumInst();
            if (num_old != 0)
            {
                long index = 0;
                for(list<Inst*>::iterator k = subRow->instOfSubRow.begin(); k != subRow->instOfSubRow.end(); k++)
                {
                    Inst* temp = *k;

                    Rect netBox(0, 0, 0, 0);
                    if (temp->getNumInstTerms() == 0)
                    {
                        continue;
                    }
                    vector<long> x(2 * temp->getNumInstTerms());
                    vector<InstTerm>& instTermTemp = temp->getInstTerms();
                    long i = 0;
                    for(vector<InstTerm>::iterator iter = instTermTemp.begin();
                                                   iter != instTermTemp.end(); ++iter)
                    {
                        getBBox(*iter, netBox);
                        if(netBox.top() == rowAreaBottom)
                            continue;
                        x[i++] = netBox.left();
                        x[i++] = netBox.right();
                    }
                    if(i == 0)
                    {
                        x[i++] = temp->getCoordX();
                        x[i++] = temp->getCoordX()+temp->getWidth();
                    }

                    RowCluster* newRC = new RowCluster(index);
                    newRC->addInst(temp,x);
                    old_cluster.push_back(newRC);
                    cluster.push_back(newRC);
                    index++;
                }
                flag = true;
                do{
                    for (long i=0; i<num_old; i++)
                    {
                        subRow->putInORC(old_cluster[i]);
                    }
                    new_cluster.push_back(old_cluster[0]);
                    newCount = 1;
                    long j = 0;
                    while (j < num_old-1)
                    {
                        //check overlap
                        if (!subRow->clusterOverlap(new_cluster[newCount-1], old_cluster[j+1]))
                        {
                            //merge two clusters
                            subRow->mergeCluster(new_cluster[newCount-1],old_cluster[j+1]);
                            subRow->putInORC(new_cluster[newCount-1]);
                        }
                        else
                        {
                            newCount++;
                            new_cluster.push_back(old_cluster[j+1]);
                        }
                        j++;
                    }
                    num_old = newCount;
                    old_cluster.clear();
                    for (long i=0; i<num_old; i++)
                    {
                        old_cluster.push_back(new_cluster[i]);
                        old_cluster[i]->setIndex(i);
                    }
                    new_cluster.clear();
                    //do until no overlap in old_cluster
                    flag = false;
                    for (long i=0; i<num_old; i++)
                    {
                        for (long j=i+1; j<num_old; j++)
                        {
                            if (!subRow->clusterOverlap(old_cluster[i], old_cluster[j]))
                            {
                                flag = true;
                                break;
                            }
                        }
                        if (flag){
                            break;
                        }
                    }
                }while (flag);

                list<Inst*>::iterator k = subRow->instOfSubRow.begin();
                for (long i=0; i<num_old; i++)
                {
                    long offset = 0;
                    long newCoordX;
                    long LastCoordX,LastCoordY;//hjy20130624
                    for (long m = 0; m <old_cluster[i]->getNumCell(); m++)
                    {
                        Inst* temp1 = *k;
                        LastCoordX=temp1->getCoordX();//20130624
                        LastCoordY=temp1->getCoordY();//20130624
                        newCoordX = old_cluster[i]->getClusterPos()+offset;
                        offset += (long)temp1->getWidth();
                        k++;
                        if (abs(newCoordX - temp1->getOrigCoordX()) +
                        		abs(temp1->getCoordY() - temp1->getOrigCoordY()) < (long)block->getMaxDisplacement())
                        {
                            temp1->setCoordX(newCoordX);
                            updateDensity(temp1,LastCoordX,LastCoordY);//20130624
                        }
                    }
                }
                for (long i=0; i<subRow->getNumInst(); i++){
                    delete cluster[i];
                }
            }
        }
    }
}

inline void Ldplace::getSpaces(mySubRow* subRowNow, list<Inst*>::iterator k, long& origSpcPre, long& origSpcMid,
                        long& origSpcPost, long& origBoundPre, long& origBoundPost, bool type)
{
    list<Inst*>& instsNow = subRowNow->instOfSubRow;
    list<Inst*>::iterator preK, postK;
    if(k == instsNow.begin())
    {
    	//cout<<"condition 1"<<endl;
      if(!type)
      {
        origSpcPre = (*k)->getCoordX() - subRowNow->getLeftBoundary();
        origBoundPre = (*k)->getCoordX() + (*k)->getWidth();
        postK = ++k;
        --k;
        if(postK == instsNow.end())
        {
          origSpcMid = subRowNow->getRightBoundary() - (*k)->getCoordX() -(*k)->getWidth();
          origBoundPost = subRowNow->getRightBoundary();
          origSpcPost = 0;
        }
        else
        {
          origSpcMid = (*postK)->getCoordX() - (*k)->getCoordX() - (*k)->getWidth();
          origBoundPost = (*postK)->getCoordX();
          ++k;
          if(++postK == instsNow.end())
          {
            origSpcPost = subRowNow->getRightBoundary() - (*k)->getCoordX() -(*k)->getWidth();
          }
          else
          {
            origSpcPost = (*(postK))->getCoordX() - (*k)->getCoordX() - (*k)->getWidth();
          }
        }
      }

      else
      {
    	  //cout<<"condition1.2"<<endl;
        origSpcPre = 0;
        origBoundPre = subRowNow->getLeftBoundary();
        postK = ++k;

        if(postK == instsNow.end())
        {
        	//cout<<"condition 1.2.1"<<endl;
          origSpcMid = subRowNow->getRightBoundary() - subRowNow->getLeftBoundary();
          origBoundPost = subRowNow->getRightBoundary();
          origSpcPost = 0;
        }
        else
        {
        	//cout<<"condition 1.2.2"<<endl;
          origSpcMid = (*postK)->getCoordX() - subRowNow->getLeftBoundary();
          origBoundPost = (*postK)->getCoordX();
          postK++;
          if(postK == instsNow.end())
          {
            origSpcPost = subRowNow->getRightBoundary() - (*k)->getCoordX() -(*k)->getWidth();
          }
          else
          {
            origSpcPost = (*postK)->getCoordX() - (*k)->getCoordX() - (*k)->getWidth();
          }
        }
      }
    }

    else
    {
    	//cout<<"condition 2"<<endl;
      preK = --k;
      ++k;
      postK = ++k;
      --k;

      if(!type){
    	  //cout<<"condition 2.1"<<endl;
        origSpcPre = (*k)->getCoordX() - (*preK)->getCoordX() - (*preK)->getWidth();
        origBoundPre = (*k)->getCoordX() + (*k)->getWidth();
        if(postK == instsNow.end())
        {
        	//cout<<"condition 2.1.1"<<endl;
          origSpcMid = subRowNow->getRightBoundary() - (*k)->getCoordX() -(*k)->getWidth();
          origBoundPost = subRowNow->getRightBoundary();
          origSpcPost = 0;
        }
        else
        {
        	//cout<<"condition 2.1.2"<<endl;
          origSpcMid = (*postK)->getCoordX() - (*k)->getCoordX() - (*k)->getWidth();
          origBoundPost = (*postK)->getCoordX();
          ++k;
          if(++postK == instsNow.end())
          {
        	  //cout<<"condition 2.1.2.1"<<endl;
            origSpcPost = subRowNow->getRightBoundary() - (*k)->getCoordX() -(*k)->getWidth();
          }
          else
          {
        	  //cout<<"condition 2.1.2.2"<<endl;
            origSpcPost = (*postK)->getCoordX() - (*k)->getCoordX() - (*k)->getWidth();
          }
        }
      }
      else
      {
    	  //cout<<"condition 2.2"<<endl;
    	origBoundPre = (*preK)->getCoordX() + (*preK)->getWidth();
        if(preK == instsNow.begin())
        {
        	//cout<<"condition 2.2.1"<<endl;
          origSpcPre = (*preK)->getCoordX() - subRowNow->getLeftBoundary();
        }
        else
        {
        	//cout<<"condition 2.2.2"<<endl;
          --preK;
          --k;
          origSpcPre = (*k)->getCoordX() - (*preK)->getCoordX() - (*preK)->getWidth();
          ++k;
          ++preK;
        }
        if(postK == instsNow.end())
        {
        	//cout<<"condition 2.2.3"<<endl;
          origSpcPost = 0;
          origSpcMid = subRowNow->getRightBoundary() - (*preK)->getCoordX() - (*preK)->getWidth();
          origBoundPost = subRowNow->getRightBoundary();
        }
        else
        {
        	//cout<<"condition 2.2.4"<<endl;
          origSpcMid = (*postK)->getCoordX() - (*preK)->getCoordX() - (*preK)->getWidth();
          origBoundPost = (*postK)->getCoordX();
          ++k;
          ++postK;
          if(postK == instsNow.end())
          {
        	  //cout<<"condition 2.2.4.1"<<endl;
            origSpcPost = subRowNow->getRightBoundary() - (*k)->getCoordX() -(*k)->getWidth();
          }
          else
          {
        	  //cout<<"condition 2.2.4.2"<<endl;
            origSpcPost = (*postK)->getCoordX() - (*k)->getCoordX() - (*k)->getWidth();
          }
        }
      }
      //cout<<"return"<<endl;
    }
    //assert(origSpcMid == origBoundPost - origBoundPre);
}

inline void Ldplace::getPenalty(unsigned long instWidth, long& mid, long& p1, long& p2) {
    //if allow p1<0,p2<0, we take space as a benefit, it's more greedy

    if((long)instWidth > mid) {
        p1 = -1;
    }
    else {
        p1 = 0;
    }
    p2 = 0;
   /*
    p1 = (instWidth - mid) * SWAP_WEIGHT1;

    if(p1 < 0) {
        p1 = 0;
        p2 = 0;
    }
    else{
      p2 = (instWidth - pre - mid - post) * SWAP_WEIGHT2;
      if(p2 < 0) p2 = 0;
    }
    */
}

void Ldplace::swapInRow(long* origInfor, long& rowIndex, mySubRow* & subOrigNow,
                        list<Inst*>::iterator& k, bool& done, bool vertical) {
	//cout<<"swap in row "<<vertical<<" "<<rowIndex<<endl;
	Inst* instOrigin = *k;
    Inst* instObj;
    Inst* instObjEnd = NULL;
    mySubRow* subNow;
    long benefit, penalty1, penalty2, penaltyD;
    long boxLeft,boxRight,instObjL,instObjR;
    long origSpcPre, origSpcMid, origSpcPost, origSpcFree;
    long objSpcPre, objSpcMid, objSpcPost;
    long origX, origY, origWidth, objX, objY, objWidth;
    long origXnew,objXnew;
    long origHPWLold, objHPWLold, HPWLchange, rangeL, verticalCount;
    bool spaceFlag, subRowFlag;
    long temp1;
    long origBoundPre, origBoundPost, objBoundPre, objBoundPost;
    //double penaltyD1,penaltyD2;
    /* origInfor contains: origX,      origY,      origWidth,   objXnew,
     *                     origSpcPre, origSpcMid, origSpcPost, origSpcFree
     *                     origHPWLold,boxLeft,    boxRight,
     */
    origX = origInfor[0];
    origY = origInfor[1];
    origWidth = origInfor[2];
    origSpcPre = origInfor[4];
    origSpcMid = origInfor[5];
    origSpcPost = origInfor[6];
    origSpcFree = origInfor[7];
    origHPWLold = origInfor[8];
    boxLeft = origInfor[9];
    boxRight = origInfor[10];
    origBoundPre = origInfor[11];
    origBoundPost = origInfor[12];
    if(vertical)
    {
        //rangeL = origX+ RANGE;
        //if(boxLeft > origX ||
        //    rangeL > ldpRows[rowIndex]->subRows[ldpRows[rowIndex]->subRows.size() - 1]->getRightBoundary())
        rangeL = origX;
        verticalCount = 0;
        boxLeft = rangeL;
    }
    //for every subRow
    for(long subIndex = 0; subIndex < ldpRows[rowIndex]->numSubRow; ++subIndex)
    {
        if(vertical)
        {
            if(verticalCount > RANGE_NUM)
            {
                break;
            }
        }
        subNow = ldpRows[rowIndex]->subRows[subIndex];

        // if the subRow havn't reach the optBox, or the subRow is not large enough
        // or the area intersect is not large enough to hold the instOrigin
        if(subNow->getRightBoundary() <= boxLeft || subNow->getWidth() < origWidth||
            subNow->getRightBoundary() - boxLeft < origWidth)
        {
            continue;
        }

        if(subNow->getTotalInstWidth() + origWidth > subNow->getWidth())
        {
            subRowFlag = false;
        }
        else{
        	subRowFlag = true;
        }

        // if the optBox within fixed nodes, following break will work
        if(!vertical)
        {
            if(subNow->getLeftBoundary() >= boxRight)
            {
                break;
            }
        }

        //if the area intersect with the optBox is empty or the instOfrow is empty, just push_back.
        if(!(subNow->instOfSubRow.empty()))
        {
            instObjEnd = *(--subNow->instOfSubRow.end());
        }
        if(subRowFlag && (subNow->instOfSubRow.empty() ||
                instObjEnd->getCoordX() + (long)instObjEnd->getWidth() <= boxLeft))
        {
            if(boxLeft < subNow->getLeftBoundary())
            {
            	temp1 = subNow->getLeftBoundary();
            }
            else
            {
            	temp1 = boxLeft;
            }
            if (abs(temp1 - instOrigin->getOrigCoordX()) +
            		abs(subNow->getYCoord() - instOrigin->getOrigCoordY()) >= (long)block->getMaxDisplacement())
            {
                continue;
                //cout<<"000exceed max displacement"<<endl;
            }
            instOrigin->setOrigin(temp1, subNow->getYCoord());
            //assert((temp1 + instOrigin->getWidth()) <= region.right());
            updateDensity(instOrigin, origX, origY);

            //if(origHPWLold - getScaledHPWL(instOrigin) >= BENEFIT_THRESHOLD)
            //if(origHPWLold - getScaledHPWL(instOrigin) > BENEFIT_THRESHOLD)//hjy20130625
            if((origHPWLold - getScaledHPWL(instOrigin) > BENEFIT_THRESHOLD)
            	&& getPenaltyD(instOrigin, origX, origY) > 0)
            {
                subNow->instOfSubRow.push_back(instOrigin);
                subNow->totalInstWidth += origWidth;
                k = subOrigNow->instOfSubRow.erase(k);
                subOrigNow->totalInstWidth -= origWidth;
                done = true;
                break;
            }
            else
            {
                long tempX = instOrigin->getCoordX();
                long tempY = instOrigin->getCoordY();
                instOrigin->setOrigin(origX, origY);
                updateDensity(instOrigin, tempX, tempY);
                continue;
            }
        }
        else
        {
        	//search row from left to right
        	if (origX <= subNow->getLeftBoundary()){
        		for(list<Inst*>::iterator objPter = subNow->instOfSubRow.begin();
								 objPter != subNow->instOfSubRow.end(); ++objPter)
				{
					if(vertical)
					{
						if(verticalCount < RANGE_NUM)
						{
							++verticalCount;
						}
						else
						{
							break;
						}
					}
					//zhouq20130807
					//if cell objPter is not in optimal region. continue
					if ((*objPter)->getCoordX() >= boxRight){
						break;
					}
					if ((*objPter)->getCoordX() + (long)((*objPter)->getWidth()) <= boxLeft){
						continue;
					}

					spaceFlag = subRowFlag;

					instObj = *objPter;

					objWidth = (long)instObj->getWidth();
					if(subNow->totalInstWidth -objWidth + origWidth > subNow->getWidth()
							|| objWidth - origWidth > origSpcFree || instObj == instOrigin)
					{
						continue;
					}

					instObjL = instObj->getCoordX();
					instObjR = instObjL + objWidth;
					if(instObjR <= boxLeft)
					{
						continue;
					}
					objHPWLold = getScaledHPWL(instObj);
					objX = instObjL;
					objY = instObj->getCoordY();
			  // didn't involve space predicting overlap to ignore some consequent nodes
					if(objWidth < origWidth)
					{
						getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, true);
						spaceFlag = false;
						getPenalty(origWidth, objSpcMid, penalty1, penalty2);
						//zhouq2013719
						//objXnew = origX;

						//objXnew = origBoundPre;
						//origXnew = objBoundPre;

						objXnew = (origBoundPre + origBoundPost) / 2 - instObj->getWidth() / 2;
						origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;

						/*if(objPter == subNow->instOfSubRow.begin())
						{
							origXnew = subNow->getLeftBoundary();
						}
						else
						{
							--objPter;
							origXnew =  (*objPter)->getCoordX() + (*objPter)->getWidth();
							++objPter;
						}*/
					}
					else
					{
						//zhouq2013722
						getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, true);
						getPenalty(objWidth, origSpcMid, penalty1, penalty2);
						//zhouq2013719
						objXnew = (origBoundPre + origBoundPost) / 2 - instObj->getWidth() / 2;
						origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;
						//objXnew = origBoundPre;
						//origXnew = objBoundPre;


						//origXnew = objX;
						//objXnew = origInfor[3];
					}

					/*
					 *  just care order, single-segment clustering will do the rest
					 *  but it impacts the HPWL, so there is still space to improve: put it at the very center
					 *  3 places to refine!
					 */

					//cout<<"objNew: "<<objXnew<<endl;
					//cout<<"origNow:"<<origXnew<<endl;
					if(abs(origXnew - instOrigin->getOrigCoordX()) +
							abs(objY - instOrigin->getOrigCoordY()) >= (long)block->getMaxDisplacement())
					{
						//cout<<"111exceed max displacement"<<endl;
						//zhouq20130813
						//continue;
						break;
					}
					if(abs(objXnew - instObj->getOrigCoordX()) +
							abs(origY - instObj->getOrigCoordY()) >= (long)block->getMaxDisplacement())
					{
						//cout<<"exceed max displacement"<<endl;
						continue;
					}
					instOrigin->setOrigin(origXnew,objY);
					instObj->setOrigin(objXnew,origY);
					//assert(origXnew + instOrigin->getWidth() <= region.right());
					//assert(objXnew + instObj->getWidth() <= region.right());

					if (instObj->getWidth() > instOrigin->getWidth()){
						penaltyD = getPenaltyD(instObj, objX, objY);
					}
					else{
						penaltyD = getPenaltyD(instOrigin, origX, origY);
					}

					updateDensity(instOrigin, origX, origY);
					updateDensity(instObj, objX, objY);
					HPWLchange = objHPWLold + origHPWLold - getScaledHPWL(instOrigin) - getScaledHPWL(instObj);
					if(penalty1 == -1)
					{
						benefit = -1;
					}
					else
					{
						benefit = HPWLchange - penalty1 - penalty2;
					}
					//if(benefit < BENEFIT_THRESHOLD)
					if(benefit <= BENEFIT_THRESHOLD || penaltyD < 0)//hjy20130625
					{
						long tempX1 = instOrigin->getCoordX();
						long tempY1 = instOrigin->getCoordY();
						long tempX2 = instObj->getCoordX();
						long tempY2 = instObj->getCoordY();

						instOrigin->setOrigin(origX,origY);
						instObj->setOrigin(objX,objY);

						updateDensity(instOrigin, tempX1, tempY1);
						updateDensity(instObj, tempX2, tempY2);

						//  swap with space

						if(spaceFlag)
						{
							getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, false);
							getPenalty(origWidth, objSpcMid, penalty1, penalty2);
							//zhouq2013719

							//origXnew = objBoundPre;

							origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;

							//origXnew = origXnew + (*objPter)->getWidth();

							if(abs(origXnew - instOrigin->getOrigCoordX()) +
									abs(objY - instOrigin->getOrigCoordY()) >= (long)block->getMaxDisplacement())
							{
								break;
							}

							instOrigin->setOrigin(origXnew, objY);
							//assert(origXnew + instOrigin->getWidth() <= region.right());
							updateDensity(instOrigin, origX, origY);

							penaltyD = getPenaltyD(instOrigin, origX, origY);

							if(penalty1 == -1){
								benefit = -1;
							}
							else {
								benefit = origHPWLold - getHPWL(instOrigin)  - penalty1 - penalty2;
							}
							//if(benefit < BENEFIT_THRESHOLD)
							if(benefit <= BENEFIT_THRESHOLD || penaltyD < 0)//hjy20130625
							{
								long tempX = instOrigin->getCoordX();
								long tempY = instOrigin->getCoordY();
								instOrigin->setOrigin(origX,origY);
								updateDensity(instOrigin, tempX, tempY);
								continue;
							}
							else
							{
								objPter = subNow->instOfSubRow.insert(++objPter, instOrigin);
								subNow->totalInstWidth += origWidth;
								subOrigNow->totalInstWidth -= origWidth;
								k = subOrigNow->instOfSubRow.erase(k);
								--k;
								done = true;
								break;
							}
						}
						else{
							//zhouq20130813
							continue;
							//break;
						}
					}
					else
					{
						objPter = subNow->instOfSubRow.erase(objPter);
						objPter = subNow->instOfSubRow.insert(objPter, instOrigin);
						subNow->totalInstWidth -= objWidth;
						subNow->totalInstWidth += origWidth;

						k = subOrigNow->instOfSubRow.erase(k);
						k = subOrigNow->instOfSubRow.insert(k,instObj);
						subOrigNow->totalInstWidth -= origWidth;
						subOrigNow->totalInstWidth += objWidth;
						done = true;
						break;
					}
					if(objPter == subNow->instOfSubRow.end())
					{
						break;
					}
				}
			}
        	//search row from right to left
            else if (origX >= subNow->getRightBoundary()){
            	list<Inst*>::iterator objPter = subNow->instOfSubRow.end();
            	while (objPter != subNow->instOfSubRow.begin())
				{
            		objPter--;
					if(vertical)
					{
						if(verticalCount < RANGE_NUM)
						{
							++verticalCount;
						}
						else
						{
							break;
						}
					}
					//zhouq20130807
					//if cell objPter is not in optimal region. continue
					if ((*objPter)->getCoordX() >= boxRight){
						continue;
					}
					if ((*objPter)->getCoordX() + (long)(*objPter)->getWidth() <= boxLeft){
						break;
					}

					spaceFlag = subRowFlag;

					instObj = *objPter;

					objWidth = (long)instObj->getWidth();
					if(subNow->totalInstWidth -objWidth + origWidth > subNow->getWidth()
							|| objWidth - origWidth > origSpcFree || instObj == instOrigin)
					{
						continue;
					}

					instObjL = instObj->getCoordX();
					instObjR = instObjL + objWidth;
					if(instObjR <= boxLeft)
					{
						continue;
					}
					objHPWLold = getScaledHPWL(instObj);
					objX = instObjL;
					objY = instObj->getCoordY();
			  // didn't involve space predicting overlap to ignore some consequent nodes
					if(objWidth < origWidth)
					{
						getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, true);
						spaceFlag = false;
						getPenalty(origWidth, objSpcMid, penalty1, penalty2);
						//zhouq2013719
						//objXnew = origX;

						//objXnew = origBoundPre;
						//origXnew = objBoundPre;

						objXnew = (origBoundPre + origBoundPost) / 2 - instObj->getWidth() / 2;
						origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;

						/*if(objPter == subNow->instOfSubRow.begin())
						{
							origXnew = subNow->getLeftBoundary();
						}
						else
						{
							--objPter;
							origXnew =  (*objPter)->getCoordX() + (*objPter)->getWidth();
							++objPter;
						}*/
					}
					else
					{
						//zhouq2013722
						getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, true);
						getPenalty(objWidth, origSpcMid, penalty1, penalty2);
						//zhouq2013719
						objXnew = (origBoundPre + origBoundPost) / 2 - instObj->getWidth() / 2;
						origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;
						//objXnew = origBoundPre;
						//origXnew = objBoundPre;


						//origXnew = objX;
						//objXnew = origInfor[3];
					}

					/*
					 *  just care order, single-segment clustering will do the rest
					 *  but it impacts the HPWL, so there is still space to improve: put it at the very center
					 *  3 places to refine!
					 */

					if(abs(origXnew - instOrigin->getOrigCoordX()) +
							abs(objY - instOrigin->getOrigCoordY()) >= (long)block->getMaxDisplacement())
					{
						break;
					}
					if(abs(objXnew - instObj->getOrigCoordX()) +
							abs(origY - instObj->getOrigCoordY()) >= (long)block->getMaxDisplacement())
					{
						continue;
					}
					instOrigin->setOrigin(origXnew,objY);
					instObj->setOrigin(objXnew,origY);
					//assert(origXnew + instOrigin->getWidth() <= region.right());
					//assert(objXnew + instObj->getWidth() <= region.right());

					if (instObj->getWidth() > instOrigin->getWidth()){
						penaltyD = getPenaltyD(instObj, objX, objY);
					}
					else{
						penaltyD = getPenaltyD(instOrigin, origX, origY);
					}

					updateDensity(instOrigin, origX, origY);
					updateDensity(instObj, objX, objY);
					HPWLchange = objHPWLold + origHPWLold - getScaledHPWL(instOrigin) - getScaledHPWL(instObj);
					if(penalty1 == -1)
					{
						benefit = -1;
					}
					else
					{
						benefit = HPWLchange - penalty1 - penalty2;
					}
					//if(benefit < BENEFIT_THRESHOLD)
					if(benefit <= BENEFIT_THRESHOLD || penaltyD < 0)//hjy20130625
					{
						long tempX1 = instOrigin->getCoordX();
						long tempY1 = instOrigin->getCoordY();
						long tempX2 = instObj->getCoordX();
						long tempY2 = instObj->getCoordY();

						instOrigin->setOrigin(origX,origY);
						instObj->setOrigin(objX,objY);

						updateDensity(instOrigin, tempX1, tempY1);
						updateDensity(instObj, tempX2, tempY2);

						//  swap with space

						if(spaceFlag)
						{
							getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, false);
							getPenalty(origWidth, objSpcMid, penalty1, penalty2);
							//zhouq2013719

							//origXnew = objBoundPre;

							origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;

							//origXnew = origXnew + (*objPter)->getWidth();

							if(abs(origXnew - instOrigin->getOrigCoordX()) +
									abs(objY - instOrigin->getOrigCoordY()) >= (long)block->getMaxDisplacement())
							{
								break;
							}

							instOrigin->setOrigin(origXnew, objY);
							//assert(origXnew + instOrigin->getWidth() <= region.right());
							updateDensity(instOrigin, origX, origY);

							penaltyD = getPenaltyD(instOrigin, origX, origY);

							if(penalty1 == -1){
								benefit = -1;
							}
							else {
								benefit = origHPWLold - getHPWL(instOrigin)  - penalty1 - penalty2;
							}
							//if(benefit < BENEFIT_THRESHOLD)
							if(benefit <= BENEFIT_THRESHOLD || penaltyD < 0)//hjy20130625
							{
								long tempX = instOrigin->getCoordX();
								long tempY = instOrigin->getCoordY();
								instOrigin->setOrigin(origX,origY);
								updateDensity(instOrigin, tempX, tempY);
								continue;
							}
							else
							{
								objPter = subNow->instOfSubRow.insert(++objPter, instOrigin);
								subNow->totalInstWidth += origWidth;
								subOrigNow->totalInstWidth -= origWidth;
								k = subOrigNow->instOfSubRow.erase(k);
								//cout<<"space success"<<endl;
								--k;
								done = true;
								break;
							}
						}
						else{
							//zhouq20130813
							continue;
							//break;
						}
					}
					else
					{
						objPter = subNow->instOfSubRow.erase(objPter);
						objPter = subNow->instOfSubRow.insert(objPter, instOrigin);
						subNow->totalInstWidth -= objWidth;
						subNow->totalInstWidth += origWidth;

						k = subOrigNow->instOfSubRow.erase(k);
						k = subOrigNow->instOfSubRow.insert(k,instObj);
						subOrigNow->totalInstWidth -= origWidth;
						subOrigNow->totalInstWidth += objWidth;
						done = true;
						break;
					}
					if(objPter == subNow->instOfSubRow.end())
					{
						break;
					}
				}
            }
            else{
				for(list<Inst*>::iterator objPter = subNow->instOfSubRow.begin();
								 objPter != subNow->instOfSubRow.end(); ++objPter)
				{
					if(vertical)
					{
						if(verticalCount < RANGE_NUM)
						{
							++verticalCount;
						}
						else
						{
							break;
						}
					}
					//zhouq20130807
					//if cell objPter is not in optimal region. continue
					if ((*objPter)->getCoordX() >= boxRight){
						break;
					}
					if ((*objPter)->getCoordX() + (long)(*objPter)->getWidth() <= boxLeft){
						continue;
					}

					spaceFlag = subRowFlag;

					instObj = *objPter;

					objWidth = (long)instObj->getWidth();
					if(subNow->totalInstWidth -objWidth + origWidth > subNow->getWidth()
							|| objWidth - origWidth > origSpcFree || instObj == instOrigin)
					{
						continue;
					}

					instObjL = instObj->getCoordX();
					instObjR = instObjL + objWidth;
					if(instObjR <= boxLeft)
					{
						continue;
					}
					objHPWLold = getScaledHPWL(instObj);
					objX = instObjL;
					objY = instObj->getCoordY();
			  // didn't involve space predicting overlap to ignore some consequent nodes
					if(objWidth < origWidth)
					{
						getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, true);
						spaceFlag = false;
						getPenalty(origWidth, objSpcMid, penalty1, penalty2);
						//zhouq2013719
						//objXnew = origX;

						//objXnew = origBoundPre;
						//origXnew = objBoundPre;

						objXnew = (origBoundPre + origBoundPost) / 2 - instObj->getWidth() / 2;
						origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;

						/*if(objPter == subNow->instOfSubRow.begin())
						{
							origXnew = subNow->getLeftBoundary();
						}
						else
						{
							--objPter;
							origXnew =  (*objPter)->getCoordX() + (*objPter)->getWidth();
							++objPter;
						}*/
					}
					else
					{
						//zhouq2013722
						getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, true);
						getPenalty(objWidth, origSpcMid, penalty1, penalty2);
						//zhouq2013719
						objXnew = (origBoundPre + origBoundPost) / 2 - instObj->getWidth() / 2;
						origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;
						//objXnew = origBoundPre;
						//origXnew = objBoundPre;


						//origXnew = objX;
						//objXnew = origInfor[3];
					}

					/*
					 *  just care order, single-segment clustering will do the rest
					 *  but it impacts the HPWL, so there is still space to improve: put it at the very center
					 *  3 places to refine!
					 */

					if(abs(origXnew - instOrigin->getOrigCoordX()) +
							abs(objY - instOrigin->getOrigCoordY()) >= (long)block->getMaxDisplacement())
					{
						continue;
					}
					if(abs(objXnew - instObj->getOrigCoordX()) +
							abs(origY - instObj->getOrigCoordY()) >= (long)block->getMaxDisplacement())
					{
						continue;
					}
					instOrigin->setOrigin(origXnew,objY);
					instObj->setOrigin(objXnew,origY);
					//assert(origXnew + instOrigin->getWidth() <= region.right());
					//assert(objXnew + instObj->getWidth() <= region.right());

					if (instObj->getWidth() > instOrigin->getWidth()){
						penaltyD = getPenaltyD(instObj, objX, objY);
					}
					else{
						penaltyD = getPenaltyD(instOrigin, origX, origY);
					}

					updateDensity(instOrigin, origX, origY);
					updateDensity(instObj, objX, objY);
					HPWLchange = objHPWLold + origHPWLold - getScaledHPWL(instOrigin) - getScaledHPWL(instObj);
					if(penalty1 == -1)
					{
						benefit = -1;
					}
					else
					{
						benefit = HPWLchange - penalty1 - penalty2;
					}
					//if(benefit < BENEFIT_THRESHOLD)
					if(benefit <= BENEFIT_THRESHOLD || penaltyD < 0)//hjy20130625
					{
						long tempX1 = instOrigin->getCoordX();
						long tempY1 = instOrigin->getCoordY();
						long tempX2 = instObj->getCoordX();
						long tempY2 = instObj->getCoordY();

						instOrigin->setOrigin(origX,origY);
						instObj->setOrigin(objX,objY);

						updateDensity(instOrigin, tempX1, tempY1);
						updateDensity(instObj, tempX2, tempY2);

						//  swap with space

						if(spaceFlag)
						{
							getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, false);
							getPenalty(origWidth, objSpcMid, penalty1, penalty2);
							//zhouq2013719

							//origXnew = objBoundPre;

							origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;

							//origXnew = origXnew + (*objPter)->getWidth();

							if(abs(origXnew - instOrigin->getOrigCoordX()) +
									abs(objY - instOrigin->getOrigCoordY()) >= (long)block->getMaxDisplacement())
							{
								continue;
							}

							instOrigin->setOrigin(origXnew, objY);
							updateDensity(instOrigin, origX, origY);

							penaltyD = getPenaltyD(instOrigin, origX, origY);

							if(penalty1 == -1){
								benefit = -1;
							}
							else {
								benefit = origHPWLold - getHPWL(instOrigin)  - penalty1 - penalty2;
							}
							//if(benefit < BENEFIT_THRESHOLD)
							if(benefit <= BENEFIT_THRESHOLD || penaltyD < 0)//hjy20130625
							{
								long tempX = instOrigin->getCoordX();
								long tempY = instOrigin->getCoordY();
								instOrigin->setOrigin(origX,origY);
								updateDensity(instOrigin, tempX, tempY);
								continue;
							}
							else
							{
								objPter = subNow->instOfSubRow.insert(++objPter, instOrigin);
								subNow->totalInstWidth += origWidth;
								subOrigNow->totalInstWidth -= origWidth;
								k = subOrigNow->instOfSubRow.erase(k);
								//cout<<"space success"<<endl;
								--k;
								done = true;
								break;
							}
						}
						else{
							//zhouq20130813
							continue;
							//break;
						}
					}
					else
					{
						objPter = subNow->instOfSubRow.erase(objPter);
						objPter = subNow->instOfSubRow.insert(objPter, instOrigin);
						subNow->totalInstWidth -= objWidth;
						subNow->totalInstWidth += origWidth;

						k = subOrigNow->instOfSubRow.erase(k);
						k = subOrigNow->instOfSubRow.insert(k,instObj);
						subOrigNow->totalInstWidth -= origWidth;
						subOrigNow->totalInstWidth += objWidth;
						done = true;
						break;
					}
					if(objPter == subNow->instOfSubRow.end())
					{
						break;
					}
				}
			}
        }

        if(done)
        {
        	break;
        }
    }
}

void Ldplace::swapInRow_Density(long* origInfor, long& rowIndex, mySubRow* & subOrigNow,
                        list<Inst*>::iterator& k, bool& done) {
	//cout<<"swap in row "<<vertical<<" "<<rowIndex<<endl;
	Inst* instOrigin = *k;
    Inst* instObj;
    Inst* instObjEnd = NULL;
    mySubRow* subNow;
    long penalty1, penalty2, penaltyD;
    long boxLeft,boxRight;
    long origSpcPre, origSpcMid, origSpcPost, origSpcFree;
    long objSpcPre, objSpcMid, objSpcPost;
    long origX, origY, origWidth;
    long origXnew;
    long temp1;
    long origBoundPre, origBoundPost, objBoundPre, objBoundPost;
    //double penaltyD1,penaltyD2;
    /* origInfor contains: origX,      origY,      origWidth,   objXnew,
     *                     origSpcPre, origSpcMid, origSpcPost, origSpcFree
     *                     origHPWLold,boxLeft,    boxRight,
     */
    origX = origInfor[0];
    origY = origInfor[1];
    origWidth = origInfor[2];
    origSpcPre = origInfor[4];
    origSpcMid = origInfor[5];
    origSpcPost = origInfor[6];
    origSpcFree = origInfor[7];
    boxLeft = origInfor[9];
    boxRight = origInfor[10];
    origBoundPre = origInfor[11];
    origBoundPost = origInfor[12];
    //for every subRow
    for(long subIndex = 0; subIndex < ldpRows[rowIndex]->numSubRow; ++subIndex)
    {
        subNow = ldpRows[rowIndex]->subRows[subIndex];

        // if the optBox within fixed nodes, following break will work
        if(subNow->getLeftBoundary() >= boxRight)
        {
        	break;
        }

        // if the subRow havn't reach the optBox, or the subRow is not large enough
        // or the area intersect is not large enough to hold the instOrigin
        if(subNow->getRightBoundary() <= boxLeft || subNow->getWidth() < origWidth||
            subNow->getRightBoundary() - boxLeft < origWidth)
        {
            continue;
        }
        if(subNow->getTotalInstWidth() + origWidth > subNow->getWidth())
        {
            continue;
        }
        if((double)(subNow->getTotalInstWidth() + origWidth) / subNow->getWidth() > block->getTargetUtil()){
        	continue;
        }

        //if the area intersect with the optBox is empty or the instOfrow is empty, just push_back.
        if(!(subNow->instOfSubRow.empty()))
        {
            instObjEnd = *(--subNow->instOfSubRow.end());
        }
        if(subNow->instOfSubRow.empty() ||
                instObjEnd->getCoordX() + (long)instObjEnd->getWidth() <= boxLeft)
        {
            if(boxLeft < subNow->getLeftBoundary())
            {
            	temp1 = subNow->getLeftBoundary();
            }
            else
            {
            	temp1 = boxLeft;
            }
            if (abs(temp1 - instOrigin->getOrigCoordX()) +
            		abs(subNow->getYCoord() - instOrigin->getOrigCoordY()) >= (long)block->getMaxDisplacement())
            {
                continue;
            }
            instOrigin->setOrigin(temp1, subNow->getYCoord());
            //assert((temp1 + instOrigin->getWidth()) <= region.right());
            updateDensity(instOrigin, origX, origY);

            //if(origHPWLold - getScaledHPWL(instOrigin) >= BENEFIT_THRESHOLD)
            //if(origHPWLold - getScaledHPWL(instOrigin) > BENEFIT_THRESHOLD)//hjy20130625
            if(getPenaltyD(instOrigin, origX, origY) > 0)
            {
                subNow->instOfSubRow.push_back(instOrigin);
                subNow->totalInstWidth += origWidth;
                k = subOrigNow->instOfSubRow.erase(k);
                subOrigNow->totalInstWidth -= origWidth;
                done = true;
                --k;
                break;
            }
            else
            {
                long tempX = instOrigin->getCoordX();
                long tempY = instOrigin->getCoordY();
                instOrigin->setOrigin(origX, origY);
                updateDensity(instOrigin, tempX, tempY);
                continue;
            }
        }
        else
        {
        	//search row from left to right
        	if (origX <= subNow->getLeftBoundary()){
        		for(list<Inst*>::iterator objPter = subNow->instOfSubRow.begin();
								 objPter != subNow->instOfSubRow.end(); ++objPter)
				{
					//zhouq20130807
					//if cell objPter is not in optimal region. continue
					if ((*objPter)->getCoordX() >= boxRight){
						break;
					}
					if ((*objPter)->getCoordX() + (long)((*objPter)->getWidth()) <= boxLeft){
						continue;
					}

					instObj = *objPter;

					getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, false);
					getPenalty(origWidth, objSpcMid, penalty1, penalty2);
					//zhouq2013719
					origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;

					if(abs(origXnew - instOrigin->getOrigCoordX()) +
							abs(subNow->getYCoord() - instOrigin->getOrigCoordY()) >= (long)block->getMaxDisplacement())
					{
						break;
					}

					instOrigin->setOrigin(origXnew, subNow->getYCoord());
					// //assert(origXnew + instOrigin->getWidth() <= region.right());
					updateDensity(instOrigin, origX, origY);

					penaltyD = getPenaltyD(instOrigin, origX, origY);

					//if(benefit < BENEFIT_THRESHOLD)
					if(penaltyD < 0 || penalty1 < 0)
					{
						long tempX = instOrigin->getCoordX();
						long tempY = instOrigin->getCoordY();
						instOrigin->setOrigin(origX,origY);
						updateDensity(instOrigin, tempX, tempY);
						continue;
					}
					else
					{
						objPter = subNow->instOfSubRow.insert(++objPter, instOrigin);
						subNow->totalInstWidth += origWidth;
						subOrigNow->totalInstWidth -= origWidth;
						k = subOrigNow->instOfSubRow.erase(k);
						//cout<<"space success"<<endl;
						--k;
						done = true;
						break;
					}
				}
			}
        	//search row from right to left
            else if (origX >= subNow->getRightBoundary()){
            	list<Inst*>::iterator objPter = subNow->instOfSubRow.end();
            	while (objPter != subNow->instOfSubRow.begin())
				{
            		objPter--;

					//zhouq20130807
					//if cell objPter is not in optimal region. continue
					if ((*objPter)->getCoordX() >= boxRight){
						continue;
					}
					if ((*objPter)->getCoordX() + (long)(*objPter)->getWidth() <= boxLeft){
						break;
					}

					instObj = *objPter;

					getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, false);
					getPenalty(origWidth, objSpcMid, penalty1, penalty2);

					origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;

					if(abs(origXnew - instOrigin->getOrigCoordX()) +
							abs(subNow->getYCoord() - instOrigin->getOrigCoordY()) >= (long)block->getMaxDisplacement())
					{
						break;
					}

					instOrigin->setOrigin(origXnew, subNow->getYCoord());
					// //assert(origXnew + instOrigin->getWidth() <= region.right());
					updateDensity(instOrigin, origX, origY);

					penaltyD = getPenaltyD(instOrigin, origX, origY);

					//if(benefit < BENEFIT_THRESHOLD)
					if(penaltyD < 0 || penalty1 < 0)
					{
						long tempX = instOrigin->getCoordX();
						long tempY = instOrigin->getCoordY();
						instOrigin->setOrigin(origX,origY);
						updateDensity(instOrigin, tempX, tempY);
						continue;
					}
					else
					{
						objPter = subNow->instOfSubRow.insert(++objPter, instOrigin);
						subNow->totalInstWidth += origWidth;
						subOrigNow->totalInstWidth -= origWidth;
						k = subOrigNow->instOfSubRow.erase(k);
						//cout<<"space success"<<endl;
						--k;
						done = true;
						break;
					}
				}
            }
            else{
				for(list<Inst*>::iterator objPter = subNow->instOfSubRow.begin();
								 objPter != subNow->instOfSubRow.end(); ++objPter){

					//zhouq20130807
					//if cell objPter is not in optimal region. continue
					if ((*objPter)->getCoordX() >= boxRight){
						break;
					}
					if ((*objPter)->getCoordX() + (long)(*objPter)->getWidth() <= boxLeft){
						continue;
					}

					instObj = *objPter;

					getSpaces(subNow, objPter, objSpcPre, objSpcMid, objSpcPost, objBoundPre, objBoundPost, false);
					getPenalty(origWidth, objSpcMid, penalty1, penalty2);
					//zhouq2013719

					//origXnew = objBoundPre;

					origXnew = (objBoundPre + objBoundPost) / 2 - instOrigin->getWidth() / 2;

					//origXnew = origXnew + (*objPter)->getWidth();

					if(abs(origXnew - instOrigin->getOrigCoordX()) +
							abs(subNow->getYCoord() - instOrigin->getOrigCoordY()) >= (long)block->getMaxDisplacement())
					{
						//cout<<"111exceed max displacement"<<endl;
						//zhouq20130813
						continue;
					}

					instOrigin->setOrigin(origXnew, subNow->getYCoord());
					// //assert(origXnew + instOrigin->getWidth() <= region.right());
					updateDensity(instOrigin, origX, origY);

					penaltyD = getPenaltyD(instOrigin, origX, origY);

					//if(benefit < BENEFIT_THRESHOLD)
					if(penaltyD < 0 || penalty1 < 0)
					{
						long tempX = instOrigin->getCoordX();
						long tempY = instOrigin->getCoordY();
						instOrigin->setOrigin(origX,origY);
						updateDensity(instOrigin, tempX, tempY);
						continue;
					}
					else
					{
						objPter = subNow->instOfSubRow.insert(++objPter, instOrigin);
						subNow->totalInstWidth += origWidth;
						subOrigNow->totalInstWidth -= origWidth;
						k = subOrigNow->instOfSubRow.erase(k);
						//cout<<"space success"<<endl;
						--k;
						done = true;
						break;
					}
				}
			}
        }
        if(done)
        {
        	break;
        }
    }
}

//density-aware gvswap
void Ldplace::GVSwap() {
    Inst* instOrigin;
    mySubRow* subOrigNow;
    long origInfor[13];
    /* origInfor contains: origX,      origY,      origWidth,   objXnew,
     *                     origSpcPre, origSpcMid, origSpcPost, origSpcFree
     *                     origHPWLold,boxLeft,    boxRight,
     */
    long rowBottom, rowTop;
    long rowIndexV;
    bool done=false;
    Rect optBox;

    //zhouq2013722
    //for (long i = 0; i < numRow; i++)
    for (long i = 0; i < numRow; i++)
	{
    	for(long j = 0; j< ldpRows[i]->numSubRow; j++)
    	{
    		//cout<<"i = "<<i<<endl;
    		//cout<<"\t j = "<<j<<" "<<ldpRows[i]->subRows[j]->getNumInst()<<endl;
    		subOrigNow = ldpRows[i]->subRows[j];
    		list<Inst*>& instsNow = subOrigNow->instOfSubRow;
    		origInfor[7] = subOrigNow->getWidth()-subOrigNow->totalInstWidth;
    		for(list<Inst*>::iterator k = instsNow.begin(); k != instsNow.end(); ++k)
    		{
    			if(done)
    			{
    				origInfor[7] = subOrigNow->getWidth()-subOrigNow->totalInstWidth;
    			}
    			done = false;
    			instOrigin = *k;

    			origInfor[0] = instOrigin->getCoordX();
    			origInfor[1] = instOrigin->getCoordY();
    			origInfor[2] = (long)instOrigin->getWidth();
    			//get the local space information of instOrigion
    			getSpaces(subOrigNow, k, origInfor[4], origInfor[5], origInfor[6], origInfor[11], origInfor[12], true);
    			if(k == instsNow.begin()) {
    				origInfor[3] = subOrigNow->getLeftBoundary();
    			}else{
    				--k;
    				origInfor[3] = (*k)->getCoordX() + (*k)->getWidth();
    				++k;
    			}
    			//origInfor[8] = getHPWL(instOrigin);
    			origInfor[8] = getScaledHPWL(instOrigin);
    			getOptimalBox(instOrigin, optBox);
    			if(optBox.top() > rowAreaTop ||optBox.top() < rowAreaBottom)optBox.setTop(rowAreaTop);
    			if(optBox.bottom() < rowAreaBottom ||optBox.bottom() > rowAreaTop)optBox.setBottom(rowAreaBottom);
    			if(optBox.left() < rowAreaLeft || optBox.left() > rowAreaRight)optBox.setLeft(rowAreaLeft);
    			if(optBox.right() < rowAreaLeft || optBox.right() > rowAreaRight)optBox.setRight(rowAreaRight);
    			//convert the left, right of the optBox to site grid
    			origInfor[9] = (long)(optBox.left() / siteWidth) * siteWidth;
    			origInfor[10] = (long)(optBox.right() / siteWidth) * siteWidth + siteWidth;

    			if(optBox.right() % siteWidth == 0)
    			{
    				origInfor[10] -= siteWidth;
    			}
    			//if the cell is contained in the optimalbox, ignore it
    			if (LdpUtility::containInBox(instOrigin, optBox))
    			{
    				done = true;
    				continue;
    			}
    			rowBottom = getRowIndexIn(optBox.bottom(), true);
    			rowTop = getRowIndexIn(optBox.top(), false);

    			//assert(rowTop - rowBottom >= -1);
    			//assert(rowBottom >= 0);

    			if (rowBottom >= numRow){
    				continue;
    			}
    			//assert(rowBottom < numRow);

    			if(rowBottom > rowTop)
    			{
    				rowTop = rowBottom;
    			}
    			//global swap
    			long rowTem = 0;
    			long sig = -1;

    			//zhouq20130813
    			//cell k is below the optimal region, search from bottom to top
    			if (((*k)->getCoordY() - region.bottom()) / rowHeight <= rowBottom){
    				for(long rowIndex = rowBottom; rowIndex <= rowTop; rowIndex ++)
    				{
    					if (abs(ldpRows[rowIndex]->yCoord - (*k)->getOrigCoordY()) > block->getMaxDisplacement() * 0.8){
    						break;
    					}
    				 	swapInRow(origInfor, rowIndex, subOrigNow, k, done, false);//global
    				 	if(done)//swap successfully
    				 	{
    				 		break;
    				 	}
    				}
    			}
    			//cell k is above the optimal region, search from top to bottom
    			else if (((*k)->getCoordY() - region.bottom()) / rowHeight >= rowTop){
    				for(long rowIndex = rowTop; rowIndex >= rowBottom; rowIndex--)
    				{
    					if (abs(ldpRows[rowIndex]->yCoord - (*k)->getOrigCoordY()) > block->getMaxDisplacement() * 0.8){
    						break;
    					}
    				 	swapInRow(origInfor, rowIndex, subOrigNow, k, done, false);//global
    				 	if(done)//swap successfully
    				 	{
    				 		break;
    				 	}
    				}
    			}
    			//cell k is between top and bottom, search from center row
    			else{
					for(long rowIndex = ((rowBottom + rowTop) >> 1); rowIndex <= rowTop
							&& rowIndex >= rowBottom; rowIndex += rowTem*sig )
					{
						sig = -sig;
						++rowTem;
    					if (abs(ldpRows[rowIndex]->yCoord - (*k)->getOrigCoordY()) > block->getMaxDisplacement() * 0.8){
    						continue;
    					}
						swapInRow(origInfor, rowIndex, subOrigNow, k, done, false);//global

						if(done)//swap successfully
						{
							break;
						}
					}
    			}

    			if(done)
    			{
    				continue;
    			}
    			// vertical swap
#if 1
    			if(origInfor[1] < rowBottom || origInfor[1] >= rowTop)
    			{
    				if(origInfor[1] < rowBottom)
    				{
    					rowIndexV = i + 1;

    				}
    				else
    				{
    					rowIndexV = i - 1;
    					if(i == 0)
    					{
    						rowIndexV++;
    					}
    				}
    				swapInRow(origInfor, rowIndexV, subOrigNow, k, done, true);//vertical swap
    			}
#endif
    		}
    	}
    }
    //cout<< "numSwap = "<< numSwap <<endl;
    //cout << "Swap Done! "<< endl;
}

void Ldplace::VerticalSwap() {

    Inst* instOrigin;
    mySubRow* subOrigNow;
    long origInfor[13];
    /* origInfor contains: origX,      origY,      origWidth,   objXnew,
     *                     origSpcPre, origSpcMid, origSpcPost, origSpcFree
     *                     origHPWLold,boxLeft,    boxRight,
     */
    long rowBottom, rowTop;
    long rowIndexV;
    bool done=false;
    long numSwap = 0;
    Rect optBox;
    for (long i = 0; i < numRow; i++)
    {
      for(long j = 0; j< ldpRows[i]->numSubRow; j++)
      {
        subOrigNow = ldpRows[i]->subRows[j];
        list<Inst*>& instsNow = subOrigNow->instOfSubRow;
        origInfor[7] = subOrigNow->getWidth()-subOrigNow->totalInstWidth;
        for(list<Inst*>::iterator k = instsNow.begin(); k != instsNow.end(); ++k)
        {
          if(done)
            origInfor[7] = subOrigNow->getWidth()-subOrigNow->totalInstWidth;
          done = false;
          instOrigin = *k;
          origInfor[0] = instOrigin->getCoordX();
          origInfor[1] = instOrigin->getCoordY();
          origInfor[2] = (long)instOrigin->getWidth();
          //get the local space information of instOrigion
          getSpaces(subOrigNow, k, origInfor[4], origInfor[5], origInfor[6], origInfor[11], origInfor[12], true);

          if(k == instsNow.begin()) {
            origInfor[3] = subOrigNow->getLeftBoundary();
          }else{
            --k;
            origInfor[3] = (*k)->getCoordX() + (*k)->getWidth();
            ++k;
          }
          origInfor[8] = getHPWL(instOrigin);
          getOptimalBox(instOrigin, optBox);

          //convert the left, right of the optBox to site grid
          origInfor[9] = (long)(optBox.left() / siteWidth) * siteWidth;
          origInfor[10] = (long)(optBox.right() / siteWidth) * siteWidth + siteWidth;
          if(optBox.right() % siteWidth == 0)origInfor[10] -= siteWidth;
          //if the cell is contained in the optimalbox, ignore it
          if (LdpUtility::containInBox(instOrigin, optBox)) {
            continue;
          }
          rowBottom = getRowIndexIn(optBox.bottom(), true);
          rowTop = getRowIndexIn(optBox.top(), false);
          if(origInfor[1] < rowBottom || origInfor[1] >= rowTop) {
            if(origInfor[1] < rowBottom) {
              rowIndexV = i + 1;

            }
            else{
              rowIndexV = i - 1;
              if(i == 0)rowIndexV++;
            }
            swapInRow(origInfor, rowIndexV, subOrigNow, k, done, true);
          }
          if(done == true)numSwap++;
        }
      }
    }
//    cout << "numSwap :" << numSwap << endl;
//    cout << "Vertical Swap Done!" << endl;

}
bool Ldplace::test() {
    Inst* instOrigin;
        mySubRow* subOrigNow;
        long origInfor[13];
        /* origInfor contains: origX,      origY,      origWidth,   objXnew,
         *                     origSpcPre, origSpcMid, origSpcPost, origSpcFree
         *                     origHPWLold,boxLeft,    boxRight,
         */
        bool done=false;
        Rect optBox;
        long numInOpt = 0;
        long overlapNum = 0;
        list<Inst*>::iterator tem;
        for (long i = 0; i < numRow; i++) {
          for(long j = 0; j< ldpRows[i]->numSubRow; j++) {
            subOrigNow = ldpRows[i]->subRows[j];
            list<Inst*>& instsNow = subOrigNow->instOfSubRow;
            origInfor[7] = subOrigNow->getWidth()-subOrigNow->totalInstWidth;
            for(list<Inst*>::iterator k = instsNow.begin(); k != instsNow.end(); ++k) {
              if(done)origInfor[7] = subOrigNow->getWidth()-subOrigNow->totalInstWidth;
              done = false;
              tem = ++k;
              --k;
              instOrigin = *k;
              if(tem != instsNow.end()) {
                  if((*tem)->getCoordX() < (instOrigin->getCoordX() + (long)instOrigin->getWidth())) {
                          overlapNum++;
                  }


              }
              getOptimalBox(instOrigin, optBox);
              if (LdpUtility::containInBox(instOrigin, optBox)) {
                done = true;
                ++numInOpt;
              }

            }
          }
        }
        cout<<"numInOpt = "<< numInOpt <<endl;
        cout<<"overlap num = "<<overlapNum<<endl;
        return false;

}
void Ldplace::localReOrdering(long con)
{
  mySubRow * subRow;
  Inst *pre, *post, *temp;
 // double preHPWLOld, midHPWLOld, postHPWLOld;
 // double preHPWLNew, midHPWLNew, postHPWLNew;
  double preScaledHPWLOld,midScaledHPWLOld,postScaledHPWLOld;
  double preScaledHPWLNew,midScaledHPWLNew,postScaledHPWLNew;
  //double minHPWL;
  double minScaledHPWL;
  long yCoord;
  long left, right;
  list<Inst*>::iterator preEnd;
  myPoint originPreOld, originPostOld;
  myPoint newOrigin;
  //if con is even, change the position of adjacent two cells
  if (con % 2 == 0)
  {
	for (long i = 0; i < numRow; i++)
	{
	  //if(i%100==0)cout<<"LocalReOrder: "<<i<<endl;
	  for (long j = 0; j < ldpRows[i]->getNumSubRow(); j++)
	  {
		subRow = ldpRows[i]->subRows[j];
		if (subRow->instOfSubRow.size() <= 1)
		{
		  continue;
		}
		//preEnd = --();

		for (list<Inst*>::iterator k = subRow->instOfSubRow.end() ;k != subRow->instOfSubRow.begin(); --k)
		{
		  if(k == subRow->instOfSubRow.end())
		  {
			--k;
			right = subRow->getRightBoundary();
		  }
		  else
		  {
			//right = (*(++k))->getCoordX();
			//--k;
			right=(((*k)->getCoordX()+(*k)->getWidth())+((*(++k))->getCoordX()))/2;//hjy20130730ok
			--k;//20130730ok
		  }
		  //cout<<"rwo,subrow,inst: "<<i<<" "<<j<<" "<< (*k)->getCoordX() <<endl;
		  pre = *k;
		  post = *(--k);
		  if( k == subRow->instOfSubRow.begin())
		  {
			left = subRow->getLeftBoundary();
		  }
		  else
		  {
			--k;
			//left = (*k)->getCoordX() + (*k)->getWidth();
			left=((*k)->getCoordX()+(*k)->getWidth()+post->getCoordX())/2;//hjy20130731ok
			++k;
		  }
		  ++k;//k point to pre
		  originPreOld.setCoordXY(pre->getCoordX(), pre->getCoordY());
		  originPostOld.setCoordXY(post->getCoordX(), post->getCoordY());

	//	  preHPWLOld = getHPWL(pre);
	//	  postHPWLOld = getHPWL(post);

		  preScaledHPWLOld=getScaledHPWL(pre);
		  postScaledHPWLOld=getScaledHPWL(post);

		  pre->setOrigin(left,originPreOld.coordY());
		  post->setOrigin(right - post->getWidth(), originPreOld.coordY());

		  updateDensity(pre,originPreOld.coordX(),originPreOld.coordY());//20130624
		  updateDensity(post,originPostOld.coordX(),originPostOld.coordY());//20130624

		//  preHPWLNew = getHPWL(pre);
		//  postHPWLNew = getHPWL(post);

		  preScaledHPWLNew=getScaledHPWL(pre);
		  postScaledHPWLNew=getScaledHPWL(post);
		  //if ( preHPWLOld - preHPWLNew +  postHPWLOld - postHPWLNew > 0)
		  if(preScaledHPWLOld - preScaledHPWLNew +  postScaledHPWLOld - postScaledHPWLNew > 0)
		  {
			if((abs(pre->getCoordX()-pre->getOrigCoordX())+abs(pre->getCoordY()-pre->getOrigCoordY())<block->getMaxDisplacement())
				&&(abs(post->getCoordX()-post->getOrigCoordX())+abs(post->getCoordY()-post->getOrigCoordY())<block->getMaxDisplacement()))
			{
			  temp = post;
			  k = subRow->instOfSubRow.erase(--k);
			  k = subRow->instOfSubRow.insert(++k, temp);

			}
			else
			{
			  pre->setOrigin(originPreOld);
			  post->setOrigin(originPostOld);
			  updateDensity(pre,left,originPreOld.coordY());//20130624
			  updateDensity(post,right - post->getWidth(),originPostOld.coordY());//20130624
			  continue;
			}
		  }
		  else
		  {
			pre->setOrigin(originPreOld);
			post->setOrigin(originPostOld);
			updateDensity(pre,left,originPreOld.coordY());//20130624
			updateDensity(post,right - post->getWidth(),originPostOld.coordY());//20130624

		  }
		}

	  }

	}
  }
  //===============================hjy20130606==================================
  else
  {

	Inst *mid;
	myPoint originMidOld;
	Inst *tempPre,*tempMid,*tempPost;
	long preLastCoordX = 0;
	long midLastCoordX = 0;
	long postLastCoordX = 0;
	int PairCount=0;
	int PairSwap=0;
	int swap=0;
	for (long i = 0; i < numRow; i++)
	{
	  //if(row%100==0)cout<<"LocalReOrder: "<<i<<endl;
	  for (long j = 0; j < ldpRows[i]->getNumSubRow(); j++)
	  {
		subRow = ldpRows[i]->subRows[j];
		if (subRow->instOfSubRow.size() <= 1)
		{
		  continue;
		}
		else if(subRow->instOfSubRow.size()<=2)
		{
			PairCount++;
			for(list<Inst*>::iterator k=subRow->instOfSubRow.end();k!=subRow->instOfSubRow.begin();--k)
			{

				  if(k == subRow->instOfSubRow.end())
				  {
					--k;
					right = subRow->getRightBoundary();
				  }
				  else
				  {
					//right = (*(++k))->getCoordX();
					//--k;
					right=(((*k)->getCoordX()+(*k)->getWidth())+((*(++k))->getCoordX()))/2;//hjy20130730ok
					--k;//20130730ok
				  }
				  //cout<<"rwo,subrow,inst: "<<i<<" "<<j<<" "<< (*k)->getCoordX() <<endl;
				  pre = *k;
				  post = *(--k);
				  if( k == subRow->instOfSubRow.begin())
				  {
					left = subRow->getLeftBoundary();
				  }
				  else
				  {
					--k;
					//left = (*k)->getCoordX() + (*k)->getWidth();
					left=((*k)->getCoordX()+(*k)->getWidth()+post->getCoordX())/2;//hjy20130731ok
					++k;
				  }
				  ++k;
				  originPreOld.setCoordXY(pre->getCoordX(), pre->getCoordY());
				  originPostOld.setCoordXY(post->getCoordX(), post->getCoordY());

			//	  preHPWLOld = getHPWL(pre);
			//	  postHPWLOld = getHPWL(post);

				  preScaledHPWLOld=getScaledHPWL(pre);
				  postScaledHPWLOld=getScaledHPWL(post);

				  pre->setOrigin(left,originPreOld.coordY());
				  post->setOrigin(right - post->getWidth(), originPreOld.coordY());

				  updateDensity(pre,originPreOld.coordX(),originPreOld.coordY());//20130624
				  updateDensity(post,originPostOld.coordX(),originPostOld.coordY());//20130624

			//	  preHPWLNew = getHPWL(pre);
			//	  postHPWLNew = getHPWL(post);

				  preScaledHPWLNew=getScaledHPWL(pre);
				  postScaledHPWLNew=getScaledHPWL(post);
				  //if ( preHPWLOld - preHPWLNew +  postHPWLOld - postHPWLNew > 0)
				  if(preScaledHPWLOld - preScaledHPWLNew +  postScaledHPWLOld - postScaledHPWLNew > 0)
				  {
					if((abs(pre->getCoordX()-pre->getOrigCoordX())+abs(pre->getCoordY()-pre->getOrigCoordY())<block->getMaxDisplacement())
						&&(abs(post->getCoordX()-post->getOrigCoordX())+abs(post->getCoordY()-post->getOrigCoordY())<block->getMaxDisplacement()))
					{
					  temp = post;
					  k = subRow->instOfSubRow.erase(--k);
					  k = subRow->instOfSubRow.insert(++k, temp);
					  PairSwap++;
					}
					else
					{
					  pre->setOrigin(originPreOld);
					  post->setOrigin(originPostOld);
					  updateDensity(pre,left,originPreOld.coordY());//20130624
					  updateDensity(post,right - post->getWidth(),originPostOld.coordY());//20130624
					  continue;
					}
				  }
				  else
				  {
					pre->setOrigin(originPreOld);
					post->setOrigin(originPostOld);
					updateDensity(pre,left,originPreOld.coordY());//20130624
					updateDensity(post,right - post->getWidth(),originPostOld.coordY());//20130624

				  }

			}
			continue;
		}

		for (list<Inst*>::iterator k = subRow->instOfSubRow.end() ;k != subRow->instOfSubRow.begin(); --k)
		{

		  if(k == subRow->instOfSubRow.end())
		  {
			--k;//k point to pre
			pre = *k;
			--k;//k point to mid
			right = subRow->getRightBoundary();
		  }
		  else//k point to LastPost
		  {
			++k;//k point to LastMid
            ++k;//k point to LastPre //hjy20130809
			right=((*k)->getCoordX()+(*(--k))->getCoordX()+(*k)->getWidth())/2;//hjy20130809  // k point to LastMid
			//right=(*k)->getCoordX();//0815
			//--k;//0815
            pre = *k;//set LastMid to curPre
			--k;//k point to curMid


		  }
		  //cout<<"rwo,subrow,inst: "<<i<<" "<<j<<" "<< (*k)->getCoordX() <<endl;
		  mid = *k;
		  post = *(--k);
		  if( k == subRow->instOfSubRow.begin())//k point to post
		  {
			left = subRow->getLeftBoundary();
		  }
		  else
		  {

			--k;//k point to before post
			//left = (*k)->getCoordX() + (*k)->getWidth();
			left = ((*k)->getCoordX()+(*k)->getWidth()+post->getCoordX())/2;//hjy20130731failed
			++k;//k point to post


		  }
		  ++k;//k point to mid
		  yCoord = pre->getCoordY();
		  originPreOld.setCoordXY(pre->getCoordX(), pre->getCoordY());
		  originMidOld.setCoordXY(mid->getCoordX(), mid->getCoordY());
		  originPostOld.setCoordXY(post->getCoordX(), post->getCoordY());

		//  preHPWLOld = getHPWL(pre);
		//  midHPWLOld = getHPWL(mid);
		//  postHPWLOld = getHPWL(post);


		  preScaledHPWLOld=getScaledHPWL(pre);//20130624
		  midScaledHPWLOld=getScaledHPWL(mid);//20130624
		  postScaledHPWLOld=getScaledHPWL(post);//20130624

		//  double totalHPWLOld=preHPWLOld+midHPWLOld+postHPWLOld;
		  double totalScaledHPWLOld=preScaledHPWLOld+midScaledHPWLOld+postScaledHPWLOld;//20130624

		  Inst *Group=new Inst[3];

		  // minHPWL=totalHPWLOld;
		  minScaledHPWL=totalScaledHPWLOld;

		  int pos=5;
		  //int pos=6;
		  for(int m=0;m<5;m++)
		  //for(int m=1;m<5;m++)
		  {
			double curTotalScaledHPWL;
			pre->setOrigin(originPreOld);
			mid->setOrigin(originMidOld);
			post->setOrigin(originPostOld);
			Group[0]=*pre;
			Group[1]=*mid;
			Group[2]=*post;
			if(m!=0)
			{
			  updateDensity(pre,preLastCoordX,originPreOld.coordY());//20130624
			  updateDensity(mid,midLastCoordX,originMidOld.coordY());//20130624
			  updateDensity(post,postLastCoordX,originPostOld.coordY());//20130624
			}

			Group=getPerm1(m,Group,left,right);

			pre->setCoordX(Group[0].getCoordX());
			mid->setCoordX(Group[1].getCoordX());
			post->setCoordX(Group[2].getCoordX());

			//preHPWLNew=getHPWL(pre);
			//midHPWLNew=getHPWL(mid);
			//postHPWLNew=getHPWL(post);

			preScaledHPWLNew=getScaledHPWL(pre);
			midScaledHPWLNew=getScaledHPWL(mid);
			postScaledHPWLNew=getScaledHPWL(post);

			updateDensity(pre,originPreOld.coordX(),originPreOld.coordY());
			updateDensity(mid,originMidOld.coordX(),originMidOld.coordY());
			updateDensity(post,originPostOld.coordX(),originPostOld.coordY());

			preLastCoordX=pre->getCoordX();
			midLastCoordX=mid->getCoordX();
			postLastCoordX=post->getCoordX();


    		//curTotalHPWL=preHPWLNew+midHPWLNew+postHPWLNew;
			curTotalScaledHPWL=preScaledHPWLNew+midScaledHPWLNew+postScaledHPWLNew;

			if(curTotalScaledHPWL<minScaledHPWL)
			{
			  minScaledHPWL=curTotalScaledHPWL;
			  pos=m;
			}

			/*if(curTotalHPWL<minHPWL)
			{
			  minHPWL=curTotalHPWL;
			  pos=m;
			}*/


		  }

		  pre->setOrigin(originPreOld);
		  mid->setOrigin(originMidOld);
		  post->setOrigin(originPostOld);
		  updateDensity(pre,preLastCoordX,originPreOld.coordY());//20130624
		  updateDensity(mid,midLastCoordX,originMidOld.coordY());//20130624
		  updateDensity(post,postLastCoordX,originPostOld.coordY());//20130624

		  if(pos==5)
		  {

			delete []Group;
			Group=NULL;
			continue;
		  }
		  else
		  {
			//  cout<<"The current pos is : "<<pos<<endl;

			Group[0]=*pre;
			Group[1]=*mid;
			Group[2]=*post;

			Group=getPerm1(pos,Group,left,right);

			pre->setCoordX(Group[0].getCoordX());
			mid->setCoordX(Group[1].getCoordX());
			post->setCoordX(Group[2].getCoordX());

			updateDensity(pre,originPreOld.coordX(),originPreOld.coordY());
			updateDensity(mid,originMidOld.coordX(),originMidOld.coordY());
			updateDensity(post,originPostOld.coordX(),originPostOld.coordY());

			preLastCoordX=pre->getCoordX();
			midLastCoordX=mid->getCoordX();
			postLastCoordX=post->getCoordX();

			if((abs(pre->getCoordX()-pre->getOrigCoordX())+abs(pre->getCoordY()+pre->getOrigCoordY())<block->getMaxDisplacement())
				&&(abs(mid->getCoordX()-mid->getOrigCoordX())+abs(mid->getCoordY()+mid->getOrigCoordY())<block->getMaxDisplacement())
				&&(abs(post->getCoordX()-post->getOrigCoordX())+abs(post->getCoordY()+post->getOrigCoordY())<block->getMaxDisplacement()))
			{

			  vector<Inst> tempOrder;
			  for(int t=0;t<3;t++)
			  {
				  tempOrder.push_back(Group[t]);
			  }
			  sort(tempOrder.begin(),tempOrder.end(),cmpX);
			  int flag_num=0;
			  for(vector<Inst>::iterator ite=tempOrder.begin();ite!=tempOrder.end();ite++)
			  {
					switch(flag_num)
					{
					case 0:
						if((*ite).getIndex()==pre->getIndex())
						{
							tempPre=pre;
							break;
						}
						if((*ite).getIndex()==mid->getIndex())
						{
							tempPre=mid;
							break;
						}
						if((*ite).getIndex()==post->getIndex())
						{
							tempPre=post;
							break;
						}
						break;
					case 1:
						if((*ite).getIndex()==pre->getIndex())
						{
							tempMid=pre;
							break;
						}
						if((*ite).getIndex()==mid->getIndex())
						{
							tempMid=mid;
							break;
						}
						if((*ite).getIndex()==post->getIndex())
						{
							tempMid=post;
							break;
						}
						break;
					case 2:
						if((*ite).getIndex()==pre->getIndex())
						{
							tempPost=pre;
							break;
						}
						if((*ite).getIndex()==mid->getIndex())
						{
							tempPost=mid;
							break;
						}
						if((*ite).getIndex()==post->getIndex())
						{
							tempPost=post;
							break;
						}
						break;

					}
					++flag_num;
				}

			  k=subRow->instOfSubRow.erase(--k);
			  k=subRow->instOfSubRow.erase(k);
			  k=subRow->instOfSubRow.erase(k);
			  k=subRow->instOfSubRow.insert(k,tempPre);
			  k=subRow->instOfSubRow.insert(k,tempMid);
			  k=subRow->instOfSubRow.insert(k,tempPost);
			  ++k;
			  swap++;
			}
			else
			{

			  pre->setOrigin(originPreOld);
			  mid->setOrigin(originMidOld);
			  post->setOrigin(originPostOld);

			  updateDensity(pre,preLastCoordX,originPreOld.coordY());//20130624
			  updateDensity(mid,midLastCoordX,originMidOld.coordY());//20130624
			  updateDensity(post,postLastCoordX,originPostOld.coordY());//20130624

			}

			delete []Group;
			Group=NULL;

		  }



		}

	  }

	}
	cout<<"The count of two pair cells in subRow is :"<<PairCount<<endl;
	cout<<"The succeed pair swap amount is: "<<PairSwap<<endl;
	cout<<"The local(3) reorder amount is :"<<swap<<endl;

  }
  //============================================================================
  //  cout << "Local ReOrdering Done!" << endl;
}
//used
void Ldplace::getOptimalBox(Inst *inst, Rect& optBox) {
    Rect netBox(0, 0, 0, 0);
    if (inst->getNumInstTerms() == 0) {
        return;
    }
    //myPoint instOrigin;
    //instOrigin.setCoordXY(inst->getCoordX(), inst->getCoordY());

    vector<long> x(2 * inst->getNumInstTerms());
    vector<long> y(2 * inst->getNumInstTerms());
    vector<InstTerm>& instTermTemp = inst->getInstTerms();
    long i = 0;
    //cout<<"net bbox, numInst:"<<inst->getNumInstTerms()<<endl;
    for(vector<InstTerm>::iterator iter = instTermTemp.begin();
                                   iter != instTermTemp.end(); ++iter) {
      getBBox(*iter, netBox);
      if(netBox.top() == rowAreaBottom)continue;
      //if(inst->getCoordX()==7980 && inst->getCoordY()==206280)
      //cout<<"netBox "<< i <<" "<<netBox.left()<<" "<<netBox.right()<<" "<<netBox.top()<<" "<<netBox.bottom()<<endl;
      x[i] = netBox.left();
      y[i++] = netBox.bottom();
      x[i] = netBox.right();
      y[i++] = netBox.top();

    }
    if(i == 0) {
        LdpUtility::getBBox(inst, optBox);
        //cout<<optBox.left()<<" "<<optBox.right()<<" "<<optBox.top()<<" "<<optBox.bottom()<<endl;
        return;
    }
    sort(x.begin(), x.end());
    sort(y.begin(), y.end());
    //cout<<"after sort, x y:"<<endl;
    //if(inst->getCoordX()==7980 && inst->getCoordY()==206280)
      //for(long i = 0; i< x.size();i++)cout<<x[i]<<" "<<y[i]<<endl;
    optBox.setLeft( x[x.size() / 2 - 1] );
    optBox.setRight( x[x.size() / 2] );
    optBox.setBottom( y[y.size() / 2 - 1] );
    optBox.setTop( y[y.size() / 2] );
    //cout<<optBox.left()<<" "<<optBox.right()<<" "<<optBox.top()<<" "<<optBox.bottom()<<endl;
    x.clear();
    y.clear();
    return ;
}

//used
void Ldplace::getBBox(InstTerm& instTerm,  Rect& bbox) {

// getNet(InstTerm & ) from myPlacement class
    myNet* net = getNet(instTerm);
    //Rect bbox;
    //if (include) {
    //  getBBox(net, bbox);
    //} else {
        myPoint pnt;
        long yt = rowAreaBottom, yb = rowAreaTop, xl = rowAreaRight, xr = rowAreaLeft;
        vector<InstTerm>& instTerms = net->getTerms();
        //vector<Inst*>& insts = block->getInsts();  // for test
        for(vector<InstTerm>::iterator iter = instTerms.begin();
                                       iter != instTerms.end(); ++iter) {
          if( (*iter).getIndexInst() != instTerm.getIndexInst()) {
            getOrigin(*iter, pnt);
            if(pnt.coordX() > xr)xr = pnt.coordX();
            if(pnt.coordX() < xl)xl = pnt.coordX();
            if(pnt.coordY() > yt)yt = pnt.coordY();
            if(pnt.coordY() < yb)yb = pnt.coordY();
          }
        }
        bbox.setCoord(xl, yb, xr, yt);
      return;

}

inline long Ldplace::getHPWL(Inst *inst) {
        long hpwl = 0;
        unsigned long netIndex;

        vector<InstTerm> & termsNow = inst->getInstTerms();
        for(vector<InstTerm>::iterator i = termsNow.begin(); i != termsNow.end(); ++i) {
            netIndex = (*i).getIndexNet();
            vector<myNet*>& netsNow = block->getNets();
            hpwl += (long)(myPlacement::getHPWL(netsNow[netIndex]));
        }
        return hpwl;
}

long Ldplace::getScaledHPWL(Inst* inst){
	//cal HPWL
    long hpwl = getHPWL(inst);
    //cout<<"rate = "<<rate<<endl;
    return (long)(hpwl * (1.0 + getABUpenalty()));
}

inline void Ldplace::getOrigin(InstTerm& instTerm, myPoint& pnt) {
    vector<Inst*>& insts = block->getInsts();
    pnt.setCoordX( insts[instTerm.getIndexInst()]->getCenterX() + instTerm.getOffsetX() );
    pnt.setCoordY( insts[instTerm.getIndexInst()]->getCenterY() + instTerm.getOffsetY() );
}

inline void Ldplace::getBBox( myNet* & net, Rect& bbox) {
    myPoint pnt;
    long yt = 0, yb = rowAreaTop, xl = rowAreaBottom, xr = 0;
    vector<InstTerm>& instTerms = net->getTerms();
    for(vector<InstTerm>::iterator iter = instTerms.begin();
        iter != instTerms.end(); ++iter) {
      getOrigin(*iter, pnt);
      if(pnt.coordX() > xr)xr = pnt.coordX();
      if(pnt.coordX() < xl)xl = pnt.coordX();
      if(pnt.coordY() > yt)yt = pnt.coordY();
      if(pnt.coordY() < yb)yb = pnt.coordY();
    }
    bbox.setCoord(xl, yb, xr, yt);
}

//used
inline long Ldplace::getRowIndexIn(long y,bool bottom) {
        long index = (long) ((y - rowAreaBottom) / rowHeight);
        //guarantee the bottom row is totally within the region
        if (bottom) {
          //if((y - rowAreaBottom) % rowHeight != 0)
          //++index;
        }else{
          if((y - rowAreaBottom) % rowHeight == 0)
            --index;
        }
        return index;
}

double Ldplace::getDensity(long x, long y){
	x = max((long)0, x);
	x = min(x, region.right());
    return gridDensity[(x - region.left()) / gridSizeX][(y - region.bottom()) / gridSizeY];
}

//add by ZhouQ
long Ldplace::getPenaltyD(Inst* inst, long oldX, long oldY)
{
    double oldDensity = max(getDensity(oldX, oldY),
    						getDensity(oldX + inst->getWidth(), oldY));
    double newDensity = max(getDensity(inst->getCoordX(), inst->getCoordY()),
    						getDensity(inst->getCoordX() + inst->getWidth(), inst->getCoordY()));

    if (newDensity < block->getTargetUtil()){
        return 1;
    }

    if (oldDensity > newDensity){
        return 1;
    }
    return -1;
}

//add by ZhouQ
void Ldplace::updateDensity(Inst* inst, long oldX, long oldY){
    //assert(inst->getStatus() == Moved);
    double oldDensity;
    long blockX = region.left();
    long blockY = region.bottom();
    long blockW = region.width();

    //update density in new grid
    long xl = inst -> getCoordX();
    long xh = xl + inst -> getWidth();
    long y = inst -> getCoordY();
    if (xh <= blockX || xl >= blockX + (long) blockW) {
        //cout<<"111"<<endl;
        if (xh <= blockX) {
            //cout<<"112"<<endl;
            inst->setCoordX(blockX);
            xl = blockX;
            xh = xl + inst->getWidth();
        } else if (xl >= blockX + (long)blockW) {
            //cout<<"113"<<endl;
            inst->setCoordX(blockX + blockW - inst->getWidth());
            xl = inst->getCoordX();
            xh = blockX + blockW;
        }
    }
    long xStart = (long) ((xl - blockX) / gridSizeX);
    long xEnd = (long) ((xh - blockX) / gridSizeX);
    long yGrid = (long) ((y - blockY) / gridSizeY);;
    //assert(xStart >= 0);
    //assert(xEnd <= gridNumX);
    //assert(yGrid >= 0);
    //assert(yGrid <= gridNumY);

    xStart = xStart < 0 ? 0 : xStart;
    yGrid = yGrid < 0 ? 0 : yGrid;
    xEnd = xEnd < gridNumX ? xEnd : (gridNumX - 1);
    yGrid = yGrid < gridNumY ? yGrid : (gridNumY - 1);

    for (long j = xStart ; j <= xEnd ; ++j) {
        long left = xl > gridCoordX[j] ? xl : gridCoordX[j];
        long right = xh < gridCoordX[j+1] ? xh : gridCoordX[j+1];
        //assert(right >= left);
        oldDensity = gridDensity[j][yGrid];

        gridCellArea[j][yGrid] += ((right - left) * inst->getHeight());
        if (gridAreaAvailable[j][yGrid] == 0){
            gridDensity[j][yGrid] = -1;
        }
        else{
            gridDensity[j][yGrid] = gridCellArea[j][yGrid] / gridAreaAvailable[j][yGrid];
            updateABU(gridDensity[j][yGrid], oldDensity);
        }
    }


    //update density in old grid
    xl = oldX;
    xh = xl + inst -> getWidth();
    y = oldY;

    if (xh <= blockX || xl >= blockX + (long) blockW) {
        if (xh <= blockX) {
            inst->setCoordX(blockX);
            xl = blockX;
            xh = xl + inst->getWidth();
        } else if (xl >= blockX + (long)blockW) {
            inst->setCoordX(blockX + blockW - inst->getWidth());
            xl = inst->getCoordX();
            xh = blockX + blockW;
        }
    }

    xStart = (long) ((xl - blockX) / gridSizeX);
    xEnd = (long) ((xh - blockX) / gridSizeX);
    yGrid = (long) ((y - blockY) / gridSizeY);

    xStart = xStart < 0 ? 0 : xStart;
    yGrid = yGrid < 0 ? 0 : yGrid;
    xEnd = xEnd < gridNumX ? xEnd : (gridNumX - 1);
    yGrid = yGrid < gridNumY ? yGrid : (gridNumY - 1);

    for (long j = xStart ; j <= xEnd ; ++j) {
        long left = xl > gridCoordX[j] ? xl : gridCoordX[j];
        long right = xh < gridCoordX[j+1] ? xh : gridCoordX[j+1];
        //assert(right >= left);

        oldDensity = gridDensity[j][yGrid];
        gridCellArea[j][yGrid] -= ((right - left) * inst->getHeight());
        if (gridAreaAvailable[j][yGrid] == 0){
            gridDensity[j][yGrid] = -1;
        }
        else{
            gridDensity[j][yGrid] = gridCellArea[j][yGrid] / gridAreaAvailable[j][yGrid];
            updateABU(gridDensity[j][yGrid], oldDensity);
        }
    }
}

//add by ZhouQ
void Ldplace::updateABU(double newDensity, double oldDensity){
	double totalDensity;
    //cout<<newDensity<<endl;
  //  cout<<oldDensity<<endl;
    double tempNew = newDensity;
    double tempOld = oldDensity;
    int num;
    if (tempNew > abu2Last || tempOld > abu2Last){
        num = (int)(gridNumX * gridNumY * 0.02 -1);
        totalDensity = abu2 * num;
        totalDensity = totalDensity - tempOld + tempNew;
        abu2 = totalDensity / num;

    }
    else if (tempNew > abu5Last || tempOld > abu5Last){
        num = (int)(gridNumX * gridNumY * 0.05 -1);
        totalDensity = abu5 * num;
        totalDensity = totalDensity - tempOld + tempNew;
        abu5 = totalDensity / num;

    }

    else if (tempNew > abu10Last || tempOld > abu10Last){
        num = (int)(gridNumX * gridNumY * 0.10 -1);
        totalDensity = abu10 * num;
        totalDensity = totalDensity - tempOld + tempNew;
        abu10 = totalDensity / num;

    }
    else if (tempNew > abu20Last || tempOld > abu20Last){
        num = (int)(gridNumX * gridNumY * 0.20 -1);
        totalDensity = abu20 * num;
        totalDensity = totalDensity - tempOld + tempNew;
        abu20 = totalDensity / num;
    }
}


//add by ZhouQ
double Ldplace::getDensityPenalty(Inst* inst, long newX, long newY){
    long oldX = inst->getCoordX();
    long oldY = inst->getCoordY();

    double oldHPWL = getScaledHPWL(inst);

    //put inst in new location
    inst->setCoordX(newX);
    inst->setCoordY(newY);
    updateDensity(inst, oldX, oldY);

    double newHPWL = getScaledHPWL(inst);

    //put inst in original location
    inst->setCoordX(oldX);
    inst->setCoordY(oldY);
    updateDensity(inst, newX, newY);

    return (newHPWL - oldHPWL);
}

//add by ZhouQ
Inst* Ldplace::getPerm1(int idx,Inst* &Group,const long left,const long right)
{
	switch(idx)
			    {
			    case 0:
			    	Group[0].setCoordX(left);
			    	Group[2].setCoordX(right-Group[2].getWidth());
			    	Group[1].setCoordX((Group[0].getCoordX()+Group[0].getWidth()+Group[2].getCoordX()-Group[1].getWidth())/2);
			    	break;
			    case 1:
			    	Group[1].setCoordX(left);
			    	Group[2].setCoordX(right-Group[2].getWidth());
			    	Group[0].setCoordX((Group[1].getCoordX()+Group[1].getWidth()+Group[2].getCoordX()-Group[0].getWidth())/2);
			    	break;
			    case 2:
			    	Group[0].setCoordX(left);
			    	Group[1].setCoordX(right-Group[1].getWidth());
			    	Group[2].setCoordX((Group[0].getCoordX()+Group[0].getWidth()+Group[1].getCoordX()-Group[2].getWidth())/2);
			    	//Group[2].setCoordX(((inleft+Group[0].getWidth()+right-Group[1].getWidth())>>1)-Group[2].getWidth());//0815
			    	break;
			    case 3:
			    	Group[1].setCoordX(left);
			    	Group[0].setCoordX(right-Group[0].getWidth());
			    	Group[2].setCoordX((Group[1].getCoordX()+Group[1].getWidth()+Group[0].getCoordX()-Group[2].getWidth())/2);
			    	//Group[2].setCoordX(inright-Group[2].getWidth());//0815
			    	break;
			    case 4:
			    	Group[2].setCoordX(left);
			    	Group[1].setCoordX(right-Group[1].getWidth());
			    	Group[0].setCoordX((Group[2].getCoordX()+Group[2].getWidth()+Group[1].getCoordX()-Group[0].getWidth())/2);
			    	//Group[0].setCoordX(inleft);//0815
			    	break;
			    }
	return Group;
}




void Ldplace::buildABU(){
//cal abu2,5,10,20
    binDensity.clear();
    double currentCellArea = 0;
    double currentAvailableArea = 0;
    double currentDensity = 0;
    for (int i = 0; i < gridNumX; ++i){
        for (int j = 0; j < gridNumY; ++j){
            if (gridDensity[i][j] >= 0){
                Triple tri(i, j, gridDensity[i][j]);
                binDensity.push_back(tri);
            }
        }
    }

    sort(binDensity.begin(), binDensity.end(), greaterV);
    //int binNum = binDensity.size();
    int binNum = gridNumX * gridNumY;
    //cout<<"bin num X = "<<gridNumX<<" ,"<<"bin num Y = "<<gridNumY<<endl;
    //cout<<"bin density size is "<<binNum<<" / "<<gridNumX * gridNumY<<endl;
    abu2 = -1;
    abu5 = -1;
    abu10 = -1;
    abu20 = -1;

    for (int i = 0; i < binNum; ++i){
        currentCellArea += gridCellArea[binDensity[i].row][binDensity[i].column];
        currentAvailableArea += gridAreaAvailable[binDensity[i].row][binDensity[i].column];
        currentDensity += binDensity[i].element;

        if (i + 1  >= binNum * 0.20 && abu20 == -1){
            //abu20 = currentCellArea / currentAvailableArea;
            abu20 = currentDensity / (i+1);
            abu20Last = binDensity[i].element;
            //cout<<"abu20 = "<<abu20<<endl;
            //cout<<i<<endl;
            break;
        }
        if (i + 1 >= binNum * 0.10 && abu10 == -1){
            //abu10 = currentCellArea / currentAvailableArea;
            abu10 = currentDensity / (i+1);
            abu10Last = binDensity[i].element;
            //cout<<"abu10 = "<<abu10<<endl;
            //cout<<i<<endl;
            continue;
        }
        if (i + 1 >= binNum * 0.05 && abu5 == -1){
            //abu5 = currentCellArea / currentAvailableArea;
            abu5 = currentDensity / (i+1);
            abu5Last = binDensity[i].element;
            //cout<<"abu5 = "<<abu5<<endl;
            //cout<<i<<endl;
            continue;
        }
        if (i + 1  >= binNum * 0.02 && abu2 == -1){
            //abu2 = currentCellArea / currentAvailableArea;
            abu2 = currentDensity / (i+1);
            abu2Last = binDensity[i].element;
            //cout<<"abu2 = "<<abu2<<endl;
            //cout<<i<<endl;
            continue;
        }
    }
}

double Ldplace::getABUpenalty(){
    double rate = 0;
    if (abu2 > block->getTargetUtil()){
        rate += (abu2 / block->getTargetUtil() - 1) * 10.0 / 17;
    }
    if (abu5 > block->getTargetUtil()){
        rate += (abu5 / block->getTargetUtil() - 1) * 4.0 / 17;
    }
    if (abu10 > block->getTargetUtil()){
        rate += (abu10 / block->getTargetUtil() - 1) * 2.0 / 17;
    }
    if (abu20 > block->getTargetUtil()){
        rate += (abu20 / block->getTargetUtil() - 1) * 1.0 / 17;
    }
    return rate;
}


Inst* Ldplace::getPerm(int idx,Inst* &Group,const int size,Inst* &pBuf,const long left,const long right)
{
        int i,j,k;
        pBuf[0]=Group[0];
        for(i=2;i<=size;i++)
        {
                k=idx%i;
                for(j=i-1;j>k;j--)
                {
                        pBuf[j]=pBuf[j-1];
                }
                pBuf[k]=Group[i-1];
                idx/=i;
        }
        Group[0]=pBuf[0];
        Group[1]=pBuf[1];
        Group[2]=pBuf[2];
        int MidX=(left+right+pBuf[2].getWidth()-pBuf[1].getWidth()-pBuf[0].getWidth())>>1;
        //int MidX=(2*left+2*right+pBuf[2].getWidth()-2*pBuf[1].getWidth()-pBuf[0].getWidth())>>2;
        Group[0].setCoordX(right-pBuf[0].getWidth());
        Group[1].setCoordX(MidX);
        Group[2].setCoordX(left);
        //cout<<"In getPerm: ***************************"<<endl;
        //cout<<"pBuf[0].Name "<<pBuf[0].getName()<<endl;
        //cout<<"pBuf[1].Name "<<pBuf[1].getName()<<endl;
        //cout<<"pBuf[2].Name "<<pBuf[2].getName()<<endl;
        //cout<<"pBuf Now CurTotalHPWL: "<<curTotalHPWL<<endl;
        //return curTotalHPWL;
        return Group;
}

inline int Ldplace::getGridX(long x){
	return (int)((x - region.left()) / gridSizeX);
}
inline int Ldplace::getGridY(long y){
	return (int)((y - region.bottom()) / gridSizeY);
}

void Ldplace::cellBloating(double alpha){
	mySubRow* subRowNow;
	Inst* inst;
	double bloatRate;
	double segUt;
	double binUt,binUt1,binUt2;
	long totalWidthT = 0;
	for (long i = 0; i < numRow; ++i){
		for (long j = 0; j < ldpRows[i]->getNumSubRow(); ++j){
			subRowNow = ldpRows[i]->subRows[j];
			segUt = double(subRowNow->totalInstWidth) / subRowNow->getWidth();
			totalWidthT = 0;
			//assert(segUt <= 1);
			for (list<Inst*>::iterator k = subRowNow->instOfSubRow.begin(); k != subRowNow->instOfSubRow.end(); ++k){
				inst = *k;
				//assert(inst->getStatus() == Moved);
				binUt1 = getDensity(inst->getCoordX(), inst->getCoordY());
				binUt2 = getDensity(inst->getCoordX() + inst->getWidth(), inst->getCoordY());
				binUt = max(binUt1, binUt2);
				//assert(binUt <= 1);
				//assert(binUt >= 0);

				if (binUt < block->getTargetUtil()){
					//do nothing
				}
				else{
					bloatRate = alpha * (1 + (1 - max(binUt, segUt)));
					if (bloatRate < 1){
						bloatRate = 1;
					}
					//cout<<binUt<<" "<<segUt<<" "<<bloatRate<<endl;
					inst->setWidth((long)(inst->getWidth() * bloatRate));
				}
				totalWidthT += inst->getWidth();
				//assert(totalWidthT <= subRowNow->getWidth());
			}
			subRowNow->setTotalInstWidth(totalWidthT);
		}
	}
}


void Ldplace::cellRestore(){
	vector<Inst*> insts = block->getInsts();
	mySubRow* subRowNow;
	long totalWidthT = 0;
	for (long i = 0; i < (long)insts.size(); ++i){
		if (insts[i]->getStatus() == Moved){
			insts[i]->setWidth(insts[i]->getOrigWidth());
		}
	}

	for (long i = 0; i < numRow; ++i){
		for (long j = 0; j < ldpRows[i]->getNumSubRow(); ++j){
			subRowNow = ldpRows[i]->subRows[j];
			totalWidthT = 0;
			for (list<Inst*>::iterator k = subRowNow->instOfSubRow.begin(); k != subRowNow->instOfSubRow.end(); ++k){
				totalWidthT += (*k)->getWidth();
			}
			subRowNow->setTotalInstWidth(totalWidthT);
		}
	}
}

void Ldplace::rebuildBins(){
    long blockX = region.left();
    long blockY = region.bottom();
    long blockW = region.width();
    long blockH = region.height();

    vector<Inst*> validNodes = block->getInsts();
    for (long i = 0 ; i < gridNumX; i++) {
        memset(gridDensity[i], 0, sizeof(double) * gridNumY);
        memset(gridCellArea[i], 0, sizeof(double) * gridNumY);
        for (long j = 0; j < gridNumY; j++) {
            gridAreaAvailable[i][j] = (gridCoordX[i+1]-gridCoordX[i])*(gridCoordY[j+1]-gridCoordY[j]);
        }
    }

    //get all the bins density

    int numValidNodes = validNodes.size();

    for (long i = 0 ; i < numValidNodes; i++) {
        long xl = validNodes[i] -> getCoordX();
        long xh = xl + validNodes[i] -> getWidth();
        long yl = validNodes[i] -> getCoordY();
        long yh = yl + validNodes[i] -> getHeight();

        if (xh <= blockX || xl >= blockX + (long) blockW || yh <= blockY || yl
                >= blockY + (long) blockH) {
            if (validNodes[i]->getStatus() != Moved){
                continue;
            }
            else{
                cout<<"error!!! cell out of boundary!!!"<<endl;
                if (xh <= blockX) {
                	cout<<"xh "<<validNodes[i]->getName()<<" "<<blockX<<" "<<validNodes[i]->getCoordX()<<" "<<validNodes[i]->getWidth()<<endl;
                    validNodes[i]->setCoordX(blockX);
                    xl = blockX;
                    xh = xl + validNodes[i]->getWidth();
                } else if (xl >= blockX + (long)blockW) {
                	cout<<"xl "<<validNodes[i]->getName()<<" "<<blockX + blockW<<" "<<validNodes[i]->getCoordX()<<" "<<validNodes[i]->getWidth()<<endl;
                    validNodes[i]->setCoordX(blockX + blockW - validNodes[i]->getWidth());
                    xl = validNodes[i]->getCoordX();
                    xh = blockX + blockW;
                }
                if (yh <= blockY) {
                	cout<<"yh "<<validNodes[i]->getName()<<" "<<blockY<<" "<<validNodes[i]->getCoordY()<<" "<<validNodes[i]->getHeight()<<endl;
                    validNodes[i]->setCoordY(blockY);
                    yl = blockY;
                    yh = yl + validNodes[i]->getHeight();
                } else if (yl >= blockY + (long)blockH) {
                	cout<<"yl "<<validNodes[i]->getName()<<" "<<blockY + blockH<<" "<<validNodes[i]->getCoordY()<<" "<<validNodes[i]->getHeight()<<endl;
                    validNodes[i]->setCoordY(blockY + blockH - validNodes[i]->getHeight());
                    yl = validNodes[i]->getCoordY();
                    yh = blockY + blockH;
                }
            }
        }

        long xStart = (long) ((xl - blockX) / gridSizeX);
        long xEnd = (long) ((xh - blockX) / gridSizeX);
        long yStart = (long) ((yl- blockY) / gridSizeY);
        long yEnd = (long) ((yh- blockY) / gridSizeY);

        //assert(xStart >= 0);
        //assert(xEnd <= gridNumX);
        //assert(yStart >= 0);
        //assert(yEnd <= gridNumY);

        xStart = xStart < 0 ? 0 : xStart;
        yStart = yStart < 0 ? 0 : yStart;
        xEnd = xEnd < gridNumX ? xEnd : (gridNumX - 1);
        yEnd = yEnd < gridNumY ? yEnd : (gridNumY - 1);


        for (long j = xStart ; j <= xEnd ; ++j) {
            for (long k = yStart ; k <= yEnd ; ++k) {
                long left = xl > gridCoordX[j] ? xl : gridCoordX[j];
                long right = xh < gridCoordX[j+1] ? xh : gridCoordX[j+1];
                long down = yl > gridCoordY[k] ? yl : gridCoordY[k];
                long up = yh < gridCoordY[k+1] ? yh : gridCoordY[k+1];

                //assert(right >= left);
                //assert(up >= down);
                if (validNodes[i]->getStatus() == Moved){
                    gridCellArea[j][k] += ((right - left) * (up - down));
                }
                else if (validNodes[i]->getStatus() == Fixed && validNodes[i]->isRect()){
                    gridAreaAvailable[j][k] -= ((right - left) * (up - down));
                    //assert(gridAreaAvailable[j][k] >= 0);
                }
            }
        }
    }
    for (long i = 0; i < gridNumX; ++i) {
        for (long j = 0; j < gridNumY; ++j) {
            //assert(gridCellArea[i][j] >= 0);
            //assert(gridAreaAvailable[i][j] >= 0);
            if (gridAreaAvailable[i][j] == 0){
                gridDensity[i][j] = -1;
            }
            else{
                gridDensity[i][j] = gridCellArea[i][j] / gridAreaAvailable[i][j];
            }
            //assert(gridDensity[i][j] >= 0 || gridDensity[i][j] == -1);
        }
    }

    //cout<<gridCoordX[283]<<" "<<gridCoordX[284]<<" "<<gridCoordY[152]<<" "<<gridCoordY[153]<<endl;
    buildABU();
    return;
}

void Ldplace::relaxation(){
	mySubRow* subRowNow;
	long restSpace = 0;
	long restCells = 0;
	bool breakFlag = false;
	for (long i = 0; i < numRow; ++i){
		//cout<<"i = "<<i<<endl;
		for (long j = 0; j < ldpRows[i]->getNumSubRow(); ++j){
			//cout<<"\t j = "<<j<<" "<<ldpRows[i]->getNumSubRow();
			subRowNow = ldpRows[i]->subRows[j];
			restCells = subRowNow->getTotalInstWidth();
			//cout<<" "<<subRowNow->getTotalInstWidth()<<" "<<subRowNow->getWidth()<<endl;
			//cout<<subRowNow->getLeftBoundary()<<" "<<subRowNow->getYCoord()<<endl;
			breakFlag = false;
			if (restCells == 0){
				continue;
			}
			list<Inst*>::iterator k = subRowNow->instOfSubRow.begin();
			list<Inst*>::iterator preK = k;
			restSpace = subRowNow->getRightBoundary() - (*k)->getCoordX() - (*k)->getWidth();
			restCells -= (*k)->getWidth();
			if (restSpace < restCells){
				(*k)->setCoordX(subRowNow->getRightBoundary() - restCells - (*k)->getWidth());
				restSpace = subRowNow->getRightBoundary() - (*k)->getCoordX() - (*k)->getWidth();
				//assert(restSpace == restCells);
				breakFlag = true;
			}
			k++;
			while (k != subRowNow->instOfSubRow.end() && !breakFlag){
				restCells -= (*k)->getWidth();
				if ((*k)->getCoordX() < (*preK)->getCoordX() + (long)(*preK)->getWidth()){
					//assert(restSpace >= restCells);
					(*k)->setCoordX((*preK)->getCoordX() + (*preK)->getWidth());
					restSpace = subRowNow->getRightBoundary() - (*k)->getCoordX() - (*k)->getWidth();
					//assert(restSpace >= restCells);
				}
				else{
					restSpace = subRowNow->getRightBoundary() - (*k)->getCoordX() - (*k)->getWidth();
					if (restSpace  < restCells){
						//assert(subRowNow->getRightBoundary() - restCells - (*k)->getWidth() >= (*preK)->getCoordX() + (*preK)->getWidth());
						(*k)->setCoordX(subRowNow->getRightBoundary() - restCells - (*k)->getWidth());
						restSpace = subRowNow->getRightBoundary() - (*k)->getCoordX() - (*k)->getWidth();
						//assert(restSpace == restCells);
						breakFlag = true;
					}
				}

				preK = k;
				k++;
				if (breakFlag){
					break;
				}
			}
			if (breakFlag){
				while (k != subRowNow->instOfSubRow.end()){
					//assert(restSpace == restCells);
					(*k)->setCoordX((*preK)->getCoordX() + (*preK)->getWidth());
					//for debug
					restSpace = subRowNow->getRightBoundary() - (*k)->getCoordX() - (*k)->getWidth();
					restCells -= (*k)->getWidth();

					preK = k;
					k++;
				}
			}
		}
	}
}

void Ldplace::mendDisp(){
	mySubRow* subRowNow;
	long exceedDisp;
	long leftCellWidth;
	list<Inst*>::iterator k, preK, postK;
	for (long i = 0; i < numRow; ++i){
		for (long j = 0; j < ldpRows[i]->getNumSubRow(); ++j){
			subRowNow = ldpRows[i]->subRows[j];
			if (subRowNow->getNumInst() == 0){
				continue;
			}
			leftCellWidth = 0;

			for (k = subRowNow->instOfSubRow.begin(); k != subRowNow->instOfSubRow.end(); ++k){
				exceedDisp = abs((*k)->getCoordX() - (*k)->getOrigCoordX()) +
						abs((*k)->getCoordY() - (*k)->getOrigCoordY()) - block->getMaxDisplacement();
				leftCellWidth += (*k)->getWidth();
				if (exceedDisp <= 0){
					continue;
				}
				else{
					if ((*k)->getCoordX() > (*k)->getOrigCoordX()){
						(*k)->setCoordX((*k)->getCoordX() - exceedDisp);
						(*k)->setStatus(RightFixed);
					}
					else{
						(*k)->setCoordX((*k)->getCoordX() + exceedDisp);
						(*k)->setStatus(LeftFixed);
					}
				}
			}
			//for debug
			for (k = subRowNow->instOfSubRow.begin(); k != subRowNow->instOfSubRow.end(); ++k){
				//assert(abs((*k)->getCoordX() - (*k)->getOrigCoordX()) +
							//abs((*k)->getCoordY() - (*k)->getOrigCoordY()) <= block->getMaxDisplacement());
			}



			k = subRowNow->instOfSubRow.begin();
			preK = k;
			k++;
			while (k != subRowNow->instOfSubRow.end()){
				if ((*preK)->getCoordX() + (long)(*preK)->getWidth() > (*k)->getCoordX()){
					if ((*preK)->getStatus() == LeftFixed){
						//assert((*k)->getStatus() != RightFixed);
						(*k)->setCoordX((*preK)->getCoordX() + (*preK)->getWidth());
						//assert(abs((*k)->getCoordX() - (*k)->getOrigCoordX()) +
								//abs((*k)->getCoordY() - (*k)->getOrigCoordY()) <= block->getMaxDisplacement());
						(*k)->setStatus(LeftFixed);
					}
					else if ((*k)->getStatus() == RightFixed){
						//assert((*preK)->getStatus() != LeftFixed);
						(*preK)->setCoordX((*k)->getCoordX() - (*preK)->getWidth());
						//assert(abs((*preK)->getCoordX() - (*preK)->getOrigCoordX()) +
								//abs((*preK)->getCoordY() - (*preK)->getOrigCoordY()) <= block->getMaxDisplacement());
						(*preK)->setStatus(RightFixed);
						if (preK != subRowNow->instOfSubRow.begin()){
							preK--;
							k--;
							continue;
						}
						else{
							//assert((*preK)->getCoordX() >= subRowNow->getLeftBoundary());
						}
					}
				}
				preK++;
				k++;
			}

			for (k = subRowNow->instOfSubRow.begin(); k != subRowNow->instOfSubRow.end(); ++k){
				(*k)->setStatus(Moved);
				postK = ++k;
				--k;
				if (postK == subRowNow->instOfSubRow.end()){
					break;
				}
				//assert(abs((*k)->getCoordX() - (*k)->getOrigCoordX()) +
							//abs((*k)->getCoordY() - (*k)->getOrigCoordY()) <= block->getMaxDisplacement());
				//assert((*k)->getCoordX() + (*k)->getWidth() <= (*postK)->getCoordX());
			}
		}
	}
}

void Ldplace::checkDisp(){
	mySubRow* subRowNow;
	for (long i = 0; i < numRow; ++i){
		for (long j = 0; j < ldpRows[i]->getNumSubRow(); ++j){
			subRowNow = ldpRows[i]->subRows[j];
			if (subRowNow->getNumInst() == 0){
				continue;
			}
			for (list<Inst*>::iterator k = subRowNow->instOfSubRow.begin(); k != subRowNow->instOfSubRow.end(); ++k){
				//assert(abs((*k)->getCoordX() - (*k)->getOrigCoordX()) +
							//abs((*k)->getCoordY() - (*k)->getOrigCoordY()) <= block->getMaxDisplacement());
			}
		}
	}
}
//for debug
void Ldplace::checkBinDensity(){
	ofstream fout;
	fout.open("BinDensity");
	for (long i = 0; i < gridNumX; ++i){
		for (long j = 0; j < gridNumY; ++j){
			fout<<i<<" "<<j<<" "<<gridDensity[i][j]<<endl;
		}
	}
	fout.close();
	cout<<"output bin density done"<<endl;
}

//for debug
void Ldplace::checkRowDensity(){
	ofstream fout;
	fout.open("RowDensity");
	mySubRow* subRowNow;
	double totalWidth;
	double totalCell;

	for (long i = 0; i < numRow; ++i){
		totalWidth = 0;
		totalCell = 0;
		for (long j = 0; j < ldpRows[i]->getNumSubRow(); ++j){
			subRowNow = ldpRows[i]->subRows[j];
			if (subRowNow->getNumInst() == 0){
				continue;
			}
			totalWidth += subRowNow->getWidth();
			totalCell += subRowNow->getTotalInstWidth();
		}
		fout<<totalCell / totalWidth<<endl;
	}
	fout.close();
	cout<<"output row density done"<<endl;
}

//for debug
void Ldplace::checkOverlap(){
	mySubRow* subRowNow;
	list<Inst*>::iterator pre;
	for (long i = 0; i < numRow; ++i){
		for (long j = 0; j < ldpRows[i]->getNumSubRow(); ++j){
			subRowNow = ldpRows[i]->subRows[j];
			if (subRowNow->getNumInst() == 0){
				continue;
			}
			for (list<Inst*>::iterator k = subRowNow->instOfSubRow.begin(); k != subRowNow->instOfSubRow.end(); ++k){
				if (k == subRowNow->instOfSubRow.begin()){
					//assert((*k)->getCoordX() >= subRowNow->getLeftBoundary());
					continue;
				}
				pre = --k;
				++k;
				//assert((*k)->getCoordX() >= ((*pre)->getCoordX() + (*pre)->getWidth()));
				//assert((*k)->getCoordX() + (*k)->getWidth() <= subRowNow->getRightBoundary());
			}
		}
	}
}


void Ldplace::binRefineH(){
	mySubRow* subRowNow;
	long leftBound, rightBound;
	list<Inst*>::iterator pre, post;
	Rect optBox;
	long optCenterX;
	for (long i = 0; i < numRow; ++i){
		for (long j = 0; j < ldpRows[i]->getNumSubRow(); ++j){
			subRowNow = ldpRows[i]->subRows[j];
			if (subRowNow->getNumInst() == 0){
				continue;
			}
			for (list<Inst*>::iterator k = subRowNow->instOfSubRow.begin(); k != subRowNow->instOfSubRow.end(); ++k){
				//if cell k is not completely contained in a bin
				if (getGridX((*k)->getCoordX()) != getGridX((*k)->getCoordX() + (*k)->getWidth())){
					continue;
				}
				else{
					//step 1. find left and right boundary of space
					if (k == subRowNow->instOfSubRow.begin()){
						leftBound = max(gridCoordX[getGridX((*k)->getCoordX())], subRowNow->getLeftBoundary());
					}
					else{
						pre = --k;
						++k;
						if (getGridX((*k)->getCoordX()) == getGridX((*pre)->getCoordX() + (*pre)->getWidth())){
							leftBound = (*pre)->getCoordX() + (*pre)->getWidth();
							//assert(gridCoordX[getGridX((*k)->getCoordX())] <= (*pre)->getCoordX() + (*pre)->getWidth());
						}
						else{
							leftBound = max(gridCoordX[getGridX((*k)->getCoordX())], subRowNow->getLeftBoundary());
							//assert(gridCoordX[getGridX((*k)->getCoordX())] >= (*pre)->getCoordX() + (*pre)->getWidth());
						}
					}
					post = ++k;
					--k;
					if (post == subRowNow->instOfSubRow.end()){
						rightBound = min(subRowNow->getRightBoundary(), gridCoordX[getGridX((*k)->getCoordX()) + 1]);
					}
					else{
						if (getGridX((*k)->getCoordX() + (*k)->getWidth()) == getGridX((*post)->getCoordX())){
							rightBound = (*post)->getCoordX();
						}
						else{
							rightBound = min(subRowNow->getRightBoundary(), gridCoordX[getGridX((*k)->getCoordX()) + 1]);
						}
					}
					//assert(rightBound >= leftBound);
					//step 2. move cell k towards its optimal region center
					getOptimalBox((*k), optBox);
					optCenterX = (optBox.left() + optBox.right()) / 2;

					if (optCenterX <= leftBound){
						(*k)->setCoordX(leftBound);
						(*k)->setStatus(LeftFixed);
					}
					else if (optCenterX >= rightBound){
						(*k)->setCoordX(rightBound - (*k)->getWidth());
						(*k)->setStatus(RightFixed);
					}
					else{
						//do nothing
					}
					//cout<<"done"<<endl;
					if (k != subRowNow->instOfSubRow.begin()){
						pre = --k;
						++k;
						//assert((*k)->getCoordX() >= (*pre)->getCoordX() + (*pre)->getWidth());
					}
				}
			}

			for (list<Inst*>::iterator k = subRowNow->instOfSubRow.begin(); k != subRowNow->instOfSubRow.end(); ++k){
				(*k)->setStatus(Moved);
			}
		}
	}
	//checkOverlap();
}

void Ldplace::relaxationV(long rangeX, long rangeY){
	long gridX;
	long gridY;
	long rowIndexNow, rowIndexTop, rowIndexBottom;

    Inst* instOrigin;
    mySubRow* subOrigNow;
    long origInfor[13];
    /* origInfor contains: origX,      origY,      origWidth,   objXnew,
     *                     origSpcPre, origSpcMid, origSpcPost, origSpcFree
     *                     origHPWLold,boxLeft,    boxRight,
     */
    bool done=false;

    for (long i = 0; i < numRow; i++)
	{
    	//cout<<"i = "<<i<<endl;
    	for(long j = 0; j< ldpRows[i]->numSubRow; j++)
    	{
    		//cout<<"\tj = "<<j<<endl;
    		subOrigNow = ldpRows[i]->subRows[j];
    		if ((double)subOrigNow->getTotalInstWidth() / subOrigNow->getWidth() <= block->getTargetUtil()){
    			continue;
    		}
    		if (subOrigNow->getNumInst() == 0){
    			continue;
    		}
    		list<Inst*>& instsNow = subOrigNow->instOfSubRow;
    		list<Inst*> sortedInsts = instsNow;
    		//cout<<instsNow.front()->getName()<<" "<<subOrigNow->instOfSubRow.front()->getName()<<endl;
    		sortedInsts.sort(greaterWidth);
    		//cout<<instsNow.front()->getName()<<" "<<subOrigNow->instOfSubRow.front()->getName()<<endl;
    		//long x;
    		//cin>>x;
    		//cout<<"sort done"<<endl;
    		origInfor[7] = subOrigNow->getWidth()-subOrigNow->totalInstWidth;
    		for(list<Inst*>::iterator t = sortedInsts.begin(); t != sortedInsts.end(); ++t)
    		{
    			list<Inst*>::iterator k = find(instsNow.begin(), instsNow.end(), (*t));
    			//assert(k != instsNow.end());
    			if (getDensity((*k)->getCoordX(), (*k)->getCoordY()) > getDensity((*k)->getCoordX() + (*k)->getWidth(), (*k)->getCoordY())){
    				gridX = getGridX((*k)->getCoordX());
    				gridY = getGridY((*k)->getCoordY());
    				rowIndexNow = getRowIndexIn((*k)->getCoordY(), true);
    			}
    			else{
    				gridX = getGridX((*k)->getCoordX() + (*k)->getWidth());
    				gridY = getGridY((*k)->getCoordY());
    				rowIndexNow = getRowIndexIn((*k)->getCoordY(), true);
    			}

    			if(done)
    			{
    				origInfor[7] = subOrigNow->getWidth()-subOrigNow->totalInstWidth;
    			}
    			done = false;

    			if (gridDensity[gridX][gridY] <= block->getTargetUtil()){
    				continue;
    			}
    			instOrigin = *k;
    			origInfor[0] = instOrigin->getCoordX();
    			origInfor[1] = instOrigin->getCoordY();
    			origInfor[2] = (long)instOrigin->getWidth();
    			//get the local space information of instOrigion
    			getSpaces(subOrigNow, k, origInfor[4], origInfor[5], origInfor[6], origInfor[11], origInfor[12], true);
    			if(k == instsNow.begin()) {
    				origInfor[3] = subOrigNow->getLeftBoundary();
    			}else{
    				--k;
    				origInfor[3] = (*k)->getCoordX() + (*k)->getWidth();
    				++k;
    			}
    			//origInfor[8] = getHPWL(instOrigin);
    			origInfor[8] = getScaledHPWL(instOrigin);

    			origInfor[9] = gridCoordX[max((int)(getGridX((*k)->getCoordX()) - rangeX), 0)];
    			origInfor[10] = gridCoordX[min((int)(getGridX((*k)->getCoordX() + (*k)->getWidth()) + 1 + rangeX), gridNumX)];
    			//assert(origInfor[10] > origInfor[9]);

    			//cout<<"density "<<gridDensity[gridX][gridY]<<endl;
    			if (gridDensity[gridX][gridY] > block->getTargetUtil()){
    				//rowIndexMax = min(rowIndexNow + rowIndexRange, numRow - 1);
    				//rowIndexMin = max((int)(rowIndexNow - rowIndexRange), 0);
    				rowIndexTop = (getGridY(instOrigin->getCoordY()) + 1) * 9;
    				rowIndexBottom = getGridY(instOrigin->getCoordY()) * 9;
    				//assert(rowIndexTop > rowIndexBottom);
    				//cout<<"row "<<rowIndexMin<<" "<<rowIndexMax<<endl;
    				//long z = 0;
    				for (long r = 1; r <= rangeY * 9; ++r){
    					//cout<<"\t\tz = "<<z<<" "<<(*k)->getName()<<endl;
    					//++z;
    					rowIndexBottom--;
    					if (rowIndexBottom >= 0){
        					swapInRow_Density(origInfor, rowIndexBottom, subOrigNow, k, done);
    					}
        				if (done){
        					break;
        				}
        				if (gridDensity[gridX][gridY] <= block->getTargetUtil()){
        					break;
        				}

        				rowIndexTop++;
        				if (rowIndexTop < numRow){
        					swapInRow_Density(origInfor, rowIndexTop, subOrigNow, k, done);
        				}
        				if (done){
        					break;
        				}
        				if (gridDensity[gridX][gridY] <= block->getTargetUtil()){
        					break;
        				}
    				}
    			}
    		}
    	}
	}
}


void Ldplace::binRefineV(){
	long gridX;
	long gridY;
	long rowIndexNow, rowIndexMax, rowIndexMin;
	long rowIndexRange = 9;

    Inst* instOrigin;
    mySubRow* subOrigNow;
    long origInfor[13];
    /* origInfor contains: origX,      origY,      origWidth,   objXnew,
     *                     origSpcPre, origSpcMid, origSpcPost, origSpcFree
     *                     origHPWLold,boxLeft,    boxRight,
     */
    bool done=false;
    Rect optBox;

    for (long i = 0; i < numRow; i++)
	{
    	//cout<<"i = "<<i<<endl;
    	for(long j = 0; j< ldpRows[i]->numSubRow; j++)
    	{
    		//cout<<"\t j = "<<j<<endl;
    		subOrigNow = ldpRows[i]->subRows[j];
    		if ((double)subOrigNow->getTotalInstWidth() / subOrigNow->getWidth() <= block->getTargetUtil()){
    			continue;
    		}
    		list<Inst*>& instsNow = subOrigNow->instOfSubRow;
    		origInfor[7] = subOrigNow->getWidth()-subOrigNow->totalInstWidth;
    		for(list<Inst*>::iterator k = instsNow.begin(); k != instsNow.end(); ++k)
    		{
    			if(done)
    			{
    				origInfor[7] = subOrigNow->getWidth()-subOrigNow->totalInstWidth;
    			}
    			done = false;
    			instOrigin = *k;
    			origInfor[0] = instOrigin->getCoordX();
    			origInfor[1] = instOrigin->getCoordY();
    			origInfor[2] = (long)instOrigin->getWidth();
    			//get the local space information of instOrigion
    			getSpaces(subOrigNow, k, origInfor[4], origInfor[5], origInfor[6], origInfor[11], origInfor[12], true);
    			if(k == instsNow.begin()) {
    				origInfor[3] = subOrigNow->getLeftBoundary();
    			}else{
    				--k;
    				origInfor[3] = (*k)->getCoordX() + (*k)->getWidth();
    				++k;
    			}
    			//origInfor[8] = getHPWL(instOrigin);
    			origInfor[8] = getScaledHPWL(instOrigin);
    			getOptimalBox(instOrigin, optBox);
    			if(optBox.top() > rowAreaTop ||optBox.top() < rowAreaBottom)optBox.setTop(rowAreaTop);
    			if(optBox.bottom() < rowAreaBottom ||optBox.bottom() > rowAreaTop)optBox.setBottom(rowAreaBottom);
    			if(optBox.left() < rowAreaLeft || optBox.left() > rowAreaRight)optBox.setLeft(rowAreaLeft);
    			if(optBox.right() < rowAreaLeft || optBox.right() > rowAreaRight)optBox.setRight(rowAreaRight);
    			//convert the left, right of the optBox to site grid
    			origInfor[9] = (long)(optBox.left() / siteWidth) * siteWidth;
    			origInfor[10] = (long)(optBox.right() / siteWidth) * siteWidth + siteWidth;

    			if(optBox.right() % siteWidth == 0)
    			{
    				origInfor[10] -= siteWidth;
    			}
    			//if the cell is contained in the optimalbox, ignore it
    			if (LdpUtility::containInBox(instOrigin, optBox))
    			{
    				done = true;
    				continue;
    			}
    			if (getDensity((*k)->getCoordX(), (*k)->getCoordY()) > getDensity((*k)->getCoordX() + (*k)->getWidth(), (*k)->getCoordY())){
    				gridX = getGridX((*k)->getCoordX());
    				gridY = getGridY((*k)->getCoordY());
    				rowIndexNow = getRowIndexIn((*k)->getCoordY(), true);
    			}
    			else{
    				gridX = getGridX((*k)->getCoordX() + (*k)->getWidth());
    				gridY = getGridY((*k)->getCoordY());
    				rowIndexNow = getRowIndexIn((*k)->getCoordY(), true);
    			}
    			//cout<<"density "<<gridDensity[gridX][gridY]<<endl;
    			if (gridDensity[gridX][gridY] > block->getTargetUtil()){
    				rowIndexMax = min(rowIndexNow + rowIndexRange, numRow - 1);
    				rowIndexMin = max((int)(rowIndexNow - rowIndexRange), 0);
    				//assert(rowIndexMax > rowIndexMin);
    				//cout<<"row "<<rowIndexMin<<" "<<rowIndexMax<<endl;
    				for (long rowIndexV = rowIndexMin; rowIndexV <= rowIndexMax; ++rowIndexV){
    					//cout<<(*k)->getName()<<" "<<(*k)->getWidth()<<endl;
        				swapInRow(origInfor, rowIndexV, subOrigNow, k, done, false);//vertical swap
        				if (done){
        					break;
        				}
    				}
    				if (done){
        				//cout<<"swap done"<<endl;
    				}
    			}
    		}
    	}
	}
}

void Ldplace::binSpaceRefine(){
	mySubRow* subRowNow;
	long freeSpace;
	long spaceStep;
	long binIndex;
	list<Inst*>::iterator p;
	for (long i = 0; i < numRow; ++i){
		for (long j = 0; j < ldpRows[i]->getNumSubRow(); ++j){
			//cout<<"i = "<<i<<", j = "<<j<<endl;
			subRowNow = ldpRows[i]->subRows[j];
			if (subRowNow->getNumInst() == 0){
				continue;
			}
			binIndex = getGridX(subRowNow->getLeftBoundary());
			for (list<Inst*>::iterator k = subRowNow->instOfSubRow.begin(); k != subRowNow->instOfSubRow.end(); ++k){
				if (getGridX((*k)->getCoordX()) != getGridX((*k)->getCoordX() + (*k)->getWidth())){
					continue;
				}
			}
		}
	}
}


void Ldplace::assignToSites(){
	mySubRow* subRowNow;
	long siteSpacing = plcTopBlock->getRows()[0]->getSiteSpace();
	long left,right;
	//cout<<"!!site: "<<siteSpacing<<endl;
	for (long i = 0; i < numRow; ++i){
		for (long j = 0; j < ldpRows[i]->getNumSubRow(); ++j){
			//cout<<"i = "<<i<<", j = "<<j<<endl;
			subRowNow = ldpRows[i]->subRows[j];
			if (subRowNow->getNumInst() == 0){
				continue;
			}
			left = subRowNow->getLeftBoundary();
			right = subRowNow->getRightBoundary();
			if (left % siteSpacing != 0){
				//cout<<"[WARNING]: subrow left = "<<left<<" , site spacing = "<<siteSpacing<<endl;
				left = (left / siteSpacing + 1) * siteSpacing;
			}
			if (right % siteSpacing != 0){
				//cout<<"[WARNING]: subrow right = "<<right<<" , site spacing = "<<siteSpacing<<endl;
				right = (right / siteSpacing) * siteSpacing;
			}
			for (list<Inst*>::iterator k = subRowNow->instOfSubRow.begin(); k != subRowNow->instOfSubRow.end(); ++k){
				if (((long)((*k)->getCoordX()) - left) % siteSpacing != 0){
					long rate = ((*k)->getCoordX() - left) / siteSpacing;
					//cout<<"\t"<<rate<<endl;
					(*k)->setCoordX(left + rate * siteSpacing);
					//cout<<subRowNow->getYCoord()<<" "<<(*k)->getCoordY()<<endl;
				}
			}
		}
	}
}
