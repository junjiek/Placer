//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <cluster.cpp>
//
// class Cluster implementation
//
// Author: Lu Yongqiang
// History: 2009/4/21 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#include "cluster.h"
// *****************************************************************************
// class ClusterView methods
// *****************************************************************************
#include "cluster.h"
const char* ClusterView::CV_INST_ID_NAME = "APP_CV_INST_ID";
const char* ClusterView::CV_TYPE_GROUP = "CV_GROUP";
const char* ClusterView::CV_TYPE_CLUSTER = "CV_CLUSTER";

// *****************************************************************************
// Clustering algorithm, can be customized by deriving from ClusterView
// By default, only unfixed items can be clustered in function createClusters; 
//    i.e., you only need to process movable insts/clusters in this function.
//    otherwise, if you also would like to cluster fixed insts/clusters
//    please create cvTopClusters and cvTopClsuterView by your self in
//    overloaded function createClusters() with movable/fixed items together.
// *****************************************************************************

void 
ClusterView::createClusters()
{

}


// Clustering main entry, don't need to chagne this function by default.
void
ClusterView::doClustering()
{
    if (cvClusterType == CLUSTER_INST) {
	// block CV don't need to cluster
	return;
    }
    if (cvTopClusters.getNumElements()) {
	// has clustered once
	clear();
    }

    // clustering algorithm, only need to add your clusters on movable items
    // to cvTopClusters; other work, like indexing of view, networks are 
    // done by defualt procedures that follows.
    // NOTE: if you would like to cluster fixed items too, please make sure 
    // to create cvTopClusters and cvTopClusterView and the index ranges 
    // like those in resetTopClusterIndex()) by yourself.
    createClusters();
    if (cvTopClusterView.isEmpty()) {
	// clusterView will be done here
	resetTopClusterIndex();
    }
    createClusterNets();
}

// create the cluster view based on oaInst
ClusterView::ClusterView(oaBlock *topBlock) 
    : cvMaxClusterNum(0),
      cvClusterType(CLUSTER_INST),
      cvTopBlock(topBlock)
{
    // create CV based on oaBlock, this is the base CV, without clustering,
    // just one cluster standing for one instance (without actual cluster 
    // storage, only a mapping)

    // 1 create pads array and arrange the sequence
    resetPadIndex(topBlock);

    // 2. create CluterPtr array for each inst by di-ptrs
    oaIter<oaInst> sit(topBlock->getInsts());
    while (oaInst *inst = sit.getNext()) {
	addTopCluster(inst);
    }
    // create the inst id (oa AppDef) for clusterPtr mapping.
    cvInstIdDef = oaIntAppDef<oaInst>::get(CV_INST_ID_NAME);
    if (!cvInstIdDef) {
	cerr<<"Error: instance ID Def "<<CV_INST_ID_NAME<<" not found"<<endl;
	return;
    }
    // arrange the cluster sequence by floated-fixed in cvTopClusterView.
    resetTopClusterIndex();
     
    // 3. create topNets index because no need to do cluster on base CV of block
    createClusterNets();

}

// create a further cluster view based on current cluster view
ClusterView::ClusterView(ClusterView *cv) 
    : cvPads(cv->cvPads), 
      cvPadRanges(cv->cvPadRanges),
      cvMaxClusterNum(0),
      cvClusterType(CLUSTER_CLUSTER),
      cvLowerLevelCV(cv)
{
    cvPortInstParents = new HashPtr<oaInst*, ClusterPtr*>;
}


ClusterView::~ClusterView()
{
    if (cvClusterType == CLUSTER_INST) {
	if (cvInstIdDef) {
	    // delete the app def for inst-cluster mapper
	    cvInstIdDef->remove(cvTopBlock->getDesign());
	}
	return;
    }
    else if (cvClusterType == CLUSTER_CLUSTER) {
	if (cvPortInstParents) {
	    delete cvPortInstParents;
	}
	for (unsigned int i = 0; i < cvTopClusters.getNumElements(); i++) {
	    ClusterPtr *clPtr = &cvTopClusters[i];
	    if (clPtr->get1()) {
		// a cluster, need to destroy			
		delete clPtr->get1();
	    }
	}
	if (cvLowerLevelCV) {
	    // delete lower level CV
	    delete cvLowerLevelCV;
	}
    }
}

// bottome-up setting up cluster CV information including pads, netlist
/* commented on 26/08/09 13:40:43 
void
ClusterView::bottomupSetupCVsByClusters()
{
    if (cvClusterType == CLUSTER_INST) {
        // create the inst id (oa AppDef) for clusterPtr mapping.
        cvInstIdDef = oaIntAppDef<oaInst>::get(CV_INST_ID_NAME);
        if (!cvInstIdDef) {
            cerr<<"Error: instance ID Def "<<CV_INST_ID_NAME<<" not found"<<endl;
            return;
        }
        assert(cvTopClusters.getNumElements() > 0);
        assert(cvTopClusters[0].get2());
        cvTopBlock = cvTopClusterss[0].get2()->getBlock();
        //resetPadIndex(cvTopBlock);
    }
    else {
        //cvPads = cvLowerLevelCV->cvPads;
        //cvPadRanges = cvLowerLevelCV->cvPadRanges;
        cvPortInstParents = new HashPtr<oaInst*, ClusterPtr*>;
    }

    // setup cluster index
    if (cvTopClusterView.isEmpty()) {
	// clusterView will be done here
	resetTopClusterIndex();
    }
    
    // setup cluster nets
    createClusterNets();

    if (cvClusterType != CLUSTER_INST) {
        cvLowerLevelCV->bottomupSetupCVsByClusters();
    }
}
 */

// flatten a CV hierarchy to have only two-level CV left, top and root
void
ClusterView::flatten()
{
    CV *root = getRoot();

    // just flatten all Clusters so that the top Clusters have only
    // oaInst inside (in ClusterPtr).
    for (oaUInt4 i = 0; i < cvTopClusters.getNumElements(); i++) {
	ClusterPtr *clPtr = &cvTopClusters[i];
	if (clPtr->get1()) {
	    // need to explore further
	    Cluster *flat = clPtr->get1()->flatten(NULL, true);
	    delete clPtr->get1();
	    *clPtr = flat;
	}
    }


    // get the CVs to delete
    CV *cv = cvLowerLevelCV;
    List<CV*> todel;
    while (cv != root) {
	todel.add(cv);
	CV *next = cv->cvLowerLevelCV;
	// prevent from hierarchy deletion by the top CV
	cv->cvLowerLevelCV = NULL;
	cv = next;
    }
    ListIter<CV*> lit(todel);
    while (lit.getNext(cv)) {
	delete cv;
    }

    // leave two levels
    cvLowerLevelCV = root;
}


// convert the oaCluster's subs to a CV, clptrs returns the ClusterPtrs 
// corresponding to those oaClusters. 
ClusterView*
ClusterView::convertOAClusterToCV(ClusterView *cv, 
	ClusterPtr *oaclPtr, oaUInt4 &begin, oaUInt4 &end)
{
    if (!cv) {
	cv = new ClusterView();
    }
    oaCluster *topCluster = NULL;
    if (oaclPtr->get1()) {
	topCluster = (oaCluster*) oaclPtr->get1();
    }
    else if (oaclPtr->get2()) {
	cv->addTopCluster(oaclPtr->get2());
	return cv;
    }

    begin = cv->cvTopClusters.getNumElements();
    end = begin;

    // process oaCluster's subs
    oaIter<oaCluster> cit(topCluster->getClusters());
    while (oaCluster *oacls = cit.getNext()) {
	cv->addTopCluster((Cluster*) oacls);
	end++;
    }

    // process sub oaInst
    oaIter<oaInst> iit(topCluster->getInsts());
    while (oaInst *inst = iit.getNext()) {
	cv->addTopCluster(inst);
	end++;
    }
    return cv;
}

void
ClusterView::convertOAClusterHierToClusterHier(
	HashPtr<oaCluster*, Cluster*> &pairs)
{
    // check if having clusters
    bool hasSub = false;
    for (oaUInt4 i = 0; i < cvTopClusters.getNumElements(); i++) {
	if (cvTopClusters[i].get1()) {
	    hasSub = true;
	    break;
	}
    }
    if (!hasSub) {
	cvClusterType = CLUSTER_INST;
	return;
    }
    // pre-alloced auto array
    Array<UInt4> index(cvTopClusters.getNumElements() + 1, true);
    index.add(0);
    // convert the subs of current oaCluster to a sub CV
    oaUInt4 begin = 0, end = 0;
    for (oaUInt4 i = 0; i < cvTopClusters.getNumElements(); i++) {
	cvLowerLevelCV = convertOAClusterToCV(cvLowerLevelCV,
		&cvTopClusters[i], begin, end);
        index.add(end);
    }

    for (oaUInt4 i = 0; i < cvTopClusters.getNumElements(); i++) {
	if (!cvTopClusters[i].get1()) {
	    // only oaCluster indicates sub hierarchy, which needs to go further
	    continue;
	}
        // create Cluster to trace the ClusterPtrs
	Cluster *cluster = new Cluster();
	for (oaUInt4 j = index[i]; j < index[i + 1]; j++) {
	    cluster->add(&(cvLowerLevelCV->cvTopClusters[j]));
	}
	pairs.add((oaCluster*) cvTopClusters[i].get1(), cluster);
	// convert current level's oaCluster ClusterPtr to real ClusterPtr
	cvTopClusters[i] = cluster;
    }

    // Now, the current level is OK, cvTopClusters have real ClusterPtr,
    // but the lowerLevels need to convert similarly
    cvLowerLevelCV->convertOAClusterHierToClusterHier(pairs);
}

// create a Cluster view from an oaCluster hierarchy
void
ClusterView::create(oaCluster *topCluster, HashPtr<oaCluster*, Cluster*> &pairs)
{
    if (cvLowerLevelCV) {
	cerr<<"Error: cannot overwrite an existing CV hierarchy "<<endl;
	return;
    }

    if (cvTopClusters.getNumElements()) {
	// a used CV, first clear
	clear();
    }

    // get top oaClusters
    oaIter<oaCluster> cit(topCluster->getClusters());
    while (oaCluster *oacls = cit.getNext()) {
	// temp storage
	addTopCluster((Cluster*) oacls);
    }

    // process top oaInst
    oaIter<oaInst> iit(topCluster->getInsts());
    while (oaInst *inst = iit.getNext()) {
	// temp storage
	addTopCluster(inst);
    }

    // now the CV has ClusterPtrs with DiPtr<oaCluster, oaInst> actually, 
    // need to convert them to <Cluster, oaInst> ptrs
    convertOAClusterHierToClusterHier(pairs);

    // setup the index and netlist among clusters
    //bottomupSetupCVsByClusters();
}

// convert the oaGroup's subs to a CV, clptrs returns the ClusterPtrs 
// corresponding to those oaGroups. 
ClusterView*
ClusterView::convertOAGroupToCV(ClusterView *cv, 
	ClusterPtr *oaclPtr, oaUInt4 &begin, oaUInt4 &end)
{
    if (!cv) {
	cv = new ClusterView();
    }
    oaGroup *topGroup = NULL;
    if (oaclPtr->get1()) {
	topGroup = (oaGroup*) oaclPtr->get1();
    }
    else if (oaclPtr->get2()) {
	cv->addTopCluster(oaclPtr->get2());
	return cv;
    }

    begin = cv->cvTopClusters.getNumElements();
    end = begin;

    // process oaGroup's subs
    oaIter<oaGroupMember> iter(topGroup->getMembers());

    while (oaGroupMember *memb = iter.getNext()) {
	if (memb->getObject()->getType() == oacGroupType) {
	    cv->addTopCluster((Cluster*) memb->getObject());
	    end++;
	}
	else {
	    // temp storage
	    cv->addTopCluster((oaInst*) memb->getObject());
	    end++;
	}
    }

    return cv;
}

void
ClusterView::convertOAGroupHierToClusterHier(
	HashPtr<oaGroup*, Cluster*> &pairs)
{
    // check if having clusters
    bool hasSub = false;
    for (oaUInt4 i = 0; i < cvTopClusters.getNumElements(); i++) {
	if (cvTopClusters[i].get1()) {
	    hasSub = true;
	    break;
	}
    }
    if (!hasSub) {
	cvClusterType = CLUSTER_INST;
	return;
    }
    // pre-alloced auto array
    Array<UInt4> index(cvTopClusters.getNumElements() + 1, true);
    index.add(0);
    // convert the subs of current oaCluster to a sub CV
    oaUInt4 begin = 0, end = 0;
    for (oaUInt4 i = 0; i < cvTopClusters.getNumElements(); i++) {
	cvLowerLevelCV = convertOAGroupToCV(cvLowerLevelCV,
		&cvTopClusters[i], begin, end);
        index.add(end);
    }

    for (oaUInt4 i = 0; i < cvTopClusters.getNumElements(); i++) {
	if (!cvTopClusters[i].get1()) {
	    // only oaCluster indicates sub hierarchy, which needs to go further
	    continue;
	}
        // create Cluster to trace the ClusterPtrs
	Cluster *cluster = new Cluster();
	for (oaUInt4 j = index[i]; j < index[i + 1]; j++) {
	    cluster->add(&(cvLowerLevelCV->cvTopClusters[j]));
	}
	pairs.add((oaGroup*) cvTopClusters[i].get1(), cluster);
	// convert current level's oaCluster ClusterPtr to real ClusterPtr
	cvTopClusters[i] = cluster;
    }


    // Now, the current level is OK, cvTopClusters have real ClusterPtr,
    // but the lowerLevels need to convert similarly
    cvLowerLevelCV->convertOAGroupHierToClusterHier(pairs);
}
// create a Cluster view from an oaGroup hierarchy
void
ClusterView::create(oaGroup *topGroup,  HashPtr<oaGroup*, Cluster*> &pairs)
{
    if (cvLowerLevelCV) {
	cerr<<"Error: cannot overwrite an existing CV hierarchy "<<endl;
	return;
    }

    if (cvTopClusters.getNumElements()) {
	// a used CV, first clear
	clear();
    }

    // get top oaGroups
    oaIter<oaGroupMember> iter(topGroup->getMembers());

    while (oaGroupMember *memb = iter.getNext()) {
	if (memb->getObject()->getType() == oacGroupType) {
	    addTopCluster((Cluster*) memb->getObject());
	}
	else {
	    // temp storage
	    addTopCluster((oaInst*) memb->getObject());
	}
    }


    // now the CV has ClusterPtrs with DiPtr<oaGroup, oaInst> actually, 
    // need to convert them to <Cluster, oaInst> ptrs
    convertOAGroupHierToClusterHier(pairs);
}

void
ClusterView::resetPadIndex(oaBlock *topBlock)
{
    cvPads.reset();
    cvPadRanges.reset();

    oaIter<oaTerm> padIter(topBlock->getTerms());
    Array<oaTerm*> pis;
    Array<oaTerm*> pios;
    Array<oaTerm*> pos;
    while (oaTerm *term = padIter.getNext()) {
	switch (term->getTermType()) {
	    case oacInputTermType:
		pis.add(term);
		break;
	    case oacInputOutputTermType:
		pios.add(term);
		break;
	    case oacOutputTermType:
		pos.add(term);
		break;
	    default:
		break;
	}
    }
    cvPads.append(pis);
    cvPads.append(pios);
    cvPads.append(pos);
    OneDUIntRange tmp;
    // pi's index
    tmp.begin = 0;
    tmp.end = pis.getNumElements();
    cvPadRanges.append(tmp);
    // pio's index
    tmp.begin = cvPadRanges[PI].end;
    tmp.end = pios.getNumElements() + tmp.begin;
    cvPadRanges.append(tmp);
    // po's index
    tmp.begin = cvPadRanges[PIO].end;
    tmp.end =  pos.getNumElements() + tmp.begin;
    cvPadRanges.append(tmp);
}

bool
ClusterView::compareClusters(ClusterPtr* const &p1, ClusterPtr* const &p2)
{
    oaPlacementStatus stat1 = getPlacementStatus(p1);
    oaPlacementStatus stat2 = getPlacementStatus(p2);

    if (stat1 == oacNonePlacementStatus && stat2 == oacFixedPlacementStatus) {
	// movable is sorted as "smaller"
	return true;
    }
    if (stat2 == oacNonePlacementStatus && stat1 == oacFixedPlacementStatus) {
	// movable is sorted as "smaller"
	return false;
    }
    // then judge ele number
    return (getNumElements(p1) > getNumElements(p2));
}

void
ClusterView::resetTopClusterIndex()
{
    // reset the sequence by placement status

    if (cvClusterType != CLUSTER_INST) {
        // by default, the fixed items will be added to cvTopClsuters here by
        // creating new cluster objects
        oaUInt4 begin = 0, end = 0;
        cvLowerLevelCV->getIndexRange(INST_FIXED, begin, end);
        for (oaUInt4 i = begin; i < end; i++) {
            // create clusters for lower level's fix items
            // by default, one fix cluster v.s. one fix cluster
            // If user would like to cluster fix clusters/insts, please
            // revise this section
            Cluster *fixCl = new Cluster();
            fixCl->add(cvLowerLevelCV->cvTopClusterView[i]);
            fixCl->setType(CV::INST_FIXED);
            addTopCluster(fixCl);
        }
    }


    if (cvTopClusters.getNumElements()) {
	cvTopClusterView.clear();
	for (oaUInt4 i = 0; i < cvTopClusters.getNumElements(); i++) {
	    cvTopClusterView.add(&cvTopClusters[i]);
	}
    }

   
    cvTopClusterView.sort(compareClusters);
    
    oaUInt4 fix = cvTopClusterView.getNumElements();
    // get the ranges
    for (oaUInt4 i = 0; i < cvTopClusterView.getNumElements(); i++) {
	if (oacFixedPlacementStatus ==
		getPlacementStatus(cvTopClusterView[i])) {
	    // a fixed found
	    fix = i;
	    break;
	}
    }

    OneDUIntRange tmp;
    tmp.begin = 0;
    tmp.end = fix;
    cvClusterRanges.append(tmp);
    tmp.begin = fix;
    tmp.end = cvTopClusterView.getNumElements();
    cvClusterRanges.append(tmp);

    // reset the ids
    if (cvClusterType == CLUSTER_INST) {
	// the base inst-based CV, all ClusterPtr are insts
	// using the inst ID AppDef
	for (oaUInt4 i = 0; i < cvTopClusterView.getNumElements(); i++) {
	    cvInstIdDef->set(cvTopClusterView[i]->get2(), i);
	}
    }
    // reset the ID just by ClusterPtr for clusters
    for (oaUInt4 i = 0; i < cvTopClusterView.getNumElements(); i++) {
	cvTopClusterView[i]->id = i;
    }
}


ClusterPtr* 
ClusterView::getTermCluster(oaInstTerm *term)
{
    oaInst *inst = term->getInst();
    if (!inst) {
	return NULL;
    }
    if (cvClusterType == CLUSTER_INST) {
	// only inst, block based CV, get the inst ID
	unsigned int id = (unsigned int) cvInstIdDef->get(inst);
	if (id < cvTopClusterView.getNumElements()) {
	    return cvTopClusterView[id];
	}
    }
    else {
	ClusterPtr *ptr = NULL;
	cvPortInstParents->find(inst, ptr);
	return ptr;
    }
    return NULL;
}

bool
ClusterView::setClusterID(ClusterPtr *clPtr, unsigned int idIn)
{
    if (idIn >= cvTopClusterView.getNumElements()) {
	return false;
    }
    clPtr->id = idIn;
    if (cvClusterType == CLUSTER_INST) {
	// inst cluster view
	cvInstIdDef->set(clPtr->get2(), (int) idIn);
	cvTopClusterView[idIn] = clPtr;
    }
    else {
	cvTopClusterView[idIn] = clPtr;
    }
    return true;
}

unsigned int 
ClusterView::getNumElements(const ClusterPtr *clPtr)
{
    if (clPtr->get1()) {
	return clPtr->get1()->getNumElements();
    }
    else if (clPtr->get2()) {
	return 1;
    }
    return 0;
}

oaPlacementStatus 
ClusterView::getPlacementStatus(const ClusterPtr *clPtr)
{
    oaPlacementStatus stat(oacNonePlacementStatus);

    if (clPtr->get1()) {
        if (clPtr->get1()->getType() == CV::INST_FIXED) {
            return oacFixedPlacementStatus;
        }
    }
    else if (clPtr->get2()) {
        oaInst *inst = clPtr->get2();
        stat = inst->getPlacementStatus();

        switch (stat) {
            case oacNonePlacementStatus:
            case oacUnplacedPlacementStatus:
            case oacPlacedPlacementStatus:
                // all those are regarded as movable
                stat = oacNonePlacementStatus;
                break;
            case oacFixedPlacementStatus:
            case oacLockedPlacementStatus:
                stat = oacFixedPlacementStatus;
                break;
        }
    }

    return stat;
}

void
ClusterView::addClusterTerm(oaInstTerm *term, ClusterPtr *clPtr)
{
    if (clPtr->get1()) {
	// a cluster
	clPtr->get1()->addClusterTerm(term);
	cvPortInstParents->add(term->getInst(), clPtr);
    }
    else if (clPtr->get2() && cvClusterType == CLUSTER_CLUSTER) {
	// a inst itself, but the base block CV don't need this map, it
	// uses inst-id based mapping
	cvPortInstParents->add(term->getInst(), clPtr);
    }
}

Collection<oaInstTerm> 
ClusterView::getClusterTerms(ClusterPtr *clPtr)
{
    if (clPtr->get1()) {
	// cluster ptr
	return clPtr->get1()->getClusterTerms();
    }
    else if (clPtr->get2()){
	// inst ptr
	oaCollection<oaInstTerm, oaInst> *tCol = 
	    new oaCollection<oaInstTerm, oaInst>(clPtr->get2()->getInstTerms());
	return Collection<oaInstTerm>(tCol);
    }
    return Collection<oaInstTerm>();
}

Collection<ClusterPtr>
ClusterView::getClusterElements(ClusterPtr *clPtr)
{
    if (clPtr->get1()) {
	// a cluster
	return Collection<ClusterPtr>(clPtr->get1()->getMembers());
    }
    else if (clPtr->get2()) {
	// a inst
	List<ClusterPtr*> *list = new List<ClusterPtr*>;
	list->add(clPtr);
	return Collection<ClusterPtr>(list, true);
    }
    return Collection<ClusterPtr>();
}

Collection<ClusterPtr>
ClusterView::getNetClusters(oaNet *net)
{
    List<ClusterPtr*> *list = new List<ClusterPtr*>;
    oaIter<oaInstTerm> it(net->getInstTerms());
    while (oaInstTerm *term = it.getNext()) {
	list->add(getTermCluster(term));
    }
    return Collection<ClusterPtr>(list, true);
}

void
ClusterView::getIOIndexRange(IO_CLASS type, 
	unsigned int &begin, unsigned int &end)
{
    begin = end = 0;
    if (type >= PI && type < NUM_OF_IOTYPES) {
	// to get IO's ranges
	if (cvPadRanges.getNumElements() > (oaUInt4) type) {
	    begin = cvPadRanges[type].begin;
	    end = cvPadRanges[type].end;
	}
    }

}

void
ClusterView::getIndexRange(COMP_CLASS type, 
	unsigned int &begin, unsigned int &end)
{
    begin = end = 0;
    if (type >= INST_UNPLACED && type < NUM_OF_TYPES) {
	// to get normal insts' ranges
	if (cvClusterRanges.getNumElements() >  (oaUInt4) type) {
	    begin = cvClusterRanges[type].begin;
	    end = cvClusterRanges[type].end;
	}
    }
}

bool
ClusterView::isInCluster(ClusterPtr *elePtr, ClusterPtr *clRef)
{
    if (clRef->get2()) {
	// ref is a inst
	return (elePtr->get2() && elePtr->get2() == clRef->get2());
    }
    else if (clRef->get1()) {
	// ref is a parent cluster, justify if the elePtr is in it
	return clRef->get1()->isInCluster(elePtr);
    }
    return false;
}

bool
ClusterView::isClusterCutNet(oaNet *net, Array<ClusterPtr*> &parentMap)
{
    if (cvClusterType == CLUSTER_INST) {
        return true;
    }
    if (net->getTerms().getCount() > 0) {
        // has net to IOs
        return true;
    }
    // now, it is only a net connecting only normal clusters
    oaIter<oaInstTerm> it(net->getInstTerms());
    ClusterPtr *parent = NULL;
    while (oaInstTerm *term = it.getNext()) {
        if (!parent) {
            parent = parentMap[cvLowerLevelCV->getTermCluster(term)->id];
        }
        else if (parent != parentMap[cvLowerLevelCV->getTermCluster(term)->id]){
            return true;
        }
    }
    return false;
}

    void
ClusterView::createClusterCutNet(oaNet *net, Array<ClusterPtr*> &parentMap)
{
    oaIter<oaInstTerm> it(net->getInstTerms());
    ClusterPtr *parent = NULL;
    while (oaInstTerm *term = it.getNext()) {
        parent = parentMap[cvLowerLevelCV->getTermCluster(term)->id];
        if (parent) {
            addClusterTerm(term, parent);
        }
    }
}

// Create the cluster terminals
    void
ClusterView::createClusterNets()
{
    if (cvClusterType == CLUSTER_INST) {
        assert(cvTopBlock);
        // Given topBlock, this is the first block based CV
        oaIter<oaNet> nit(cvTopBlock->getNets());
        while (oaNet *net = nit.getNext()) {
            cvTopNets.append(net);
        }
        return;
    }

    // Otherwise, this is a cluster based CV, need to extract cluster nets
    //   from the lowerlevel connections
    //The sub-cluster to parent cluster mapper for lowerLevelCV->topClusterView 
    Array<ClusterPtr*> clusterMap(cvLowerLevelCV->cvTopClusterView.getNumElements());
    // reset and set the size to the volume
    clusterMap.setSize(cvLowerLevelCV->cvTopClusterView.getNumElements(), NULL);

    // create the cluster map for lower level CV's clusters
    for (oaUInt4 i = 0; i < cvTopClusterView.getNumElements(); i++) {
        Iter<ClusterPtr> it(getClusterElements(cvTopClusterView[i]));
        while (ClusterPtr *ptr = it.getNext()) {
            clusterMap[ptr->id] = cvTopClusterView[i];
        }
    }

     for (oaUInt4 i = 0; i < clusterMap.getNumElements(); i++) {
        if ( !clusterMap[i]) {
            cerr<<"Error: not clustered element found at "<<i<<", cluster view not constructed properly"<<endl;
            return;
        }
    }

    // create all cluster nets (indicated by cluster terms)
    // traverse all nets on the base CV
    for (oaUInt4 i = 0; i < cvLowerLevelCV->cvTopNets.getNumElements(); i++) {
        oaNet *net = cvLowerLevelCV->cvTopNets[i];
        if (isClusterCutNet(net, clusterMap)) {
            createClusterCutNet(net, clusterMap);
            cvTopNets.append(net);
        }
    }
}

void 
ClusterView::clear()
{
    if (cvClusterType == CLUSTER_INST) {
        return;
    }
    else if (cvClusterType == CLUSTER_CLUSTER) {
        if (cvTopClusters.getNumElements()) {
            for (unsigned int i = 0; i < cvTopClusters.getNumElements(); i++) {
                ClusterPtr *clPtr = &cvTopClusters[i];
                if (clPtr->get1()) {
                    // a cluster, need to destroy
                    delete clPtr->get1();
                }
            }
        }
        if (cvPortInstParents) {
            cvPortInstParents->clear();
        }
        cvTopClusters.clear();
        cvTopClusterView.clear();
        cvTopNets.clear();
        cvClusterRanges.clear();
    }
}

void 
ClusterView::completeCVInformation()
{
    if (cvTopClusterView.isEmpty()) {
        // by default, the fixed items will be added to cvTopClsuters here.
        oaUInt4 begin = 0, end = 0;
        cvLowerLevelCV->getIndexRange(INST_FIXED, begin, end);
        for (oaUInt4 i = begin; i < end; i++) {
            // copy the lower level's fixed items
            addTopCluster(cvLowerLevelCV->cvTopClusterView[i]);
        }
        // clusterView will be done here
        resetTopClusterIndex();
    }
    createClusterNets();
}

// convert oaGroup to oaCluster
oaCluster* 
Cluster::oaGroupToOACluster(oaBlock *topBlock,
        oaGroup *group, oaCluster *topCluster)
{
    if (!group) {
        return NULL;
    }

    if (topCluster) {
        // ignore topBlock if topCluster specified.
        topBlock = topCluster->getBlock();
        // clear the cluster before conversion, but not destroyed
        clearClusterHier(topCluster);
    }
    else if (!topBlock) {
        // must have top Block information
        return NULL;
    }

    // create a new top cluster
    if (!topCluster) {
        try {
            oaUInt4 clNum = topBlock->getClusters().getCount() + 1;
            oaString clName; 
            clName.format("%s_%d", CV::CV_TYPE_CLUSTER, clNum);
            topCluster = oaCluster::create(topBlock, clName,
                    oacClusterTypeInclusive);
        }
        catch (oaException &excp) {
            cerr<<"Error: creating oaCluster: "<<excp.getMsg()<<endl;;
            return NULL;
        }
    }

    // a group specified, process the group
    oaIter<oaGroupMember> iter(group->getMembers());

    while (oaGroupMember *memb = iter.getNext()) {
        if (memb->getObject()->getType() == oacGroupType) {
            oaGroup *group = (oaGroup*) memb->getObject();
            // a sub group, corresponding to a sub oaCluster
            oaCluster *sub = oaGroupToOACluster(topBlock, group);
            if (sub) {
                sub->addToCluster(topCluster);
            }
        }
        else {
            ((oaInst*) memb->getObject())->addToCluster(topCluster);
        }
    }

    return topCluster;
}

// convert Cluster to oaCluster
oaCluster* 
Cluster::clusterToOACluster(oaBlock *topBlock,
        Cluster *cluster,
        oaCluster *topCluster)
{
    if (!cluster) {
        return NULL;
    }

    if (topCluster) {
        // ignore topBlock if topCluster specified.
        topBlock = topCluster->getBlock();
        // clear the cluster before conversion, but not destroyed
        clearClusterHier(topCluster);
    }
    else if (!topBlock) {
        // must have top Block information
        return NULL;
    }

    // create a new top cluster
    if (!topCluster) {
        try {
            oaUInt4 clNum = topBlock->getClusters().getCount() + 1;
            oaString clName; 
            clName.format("%s_%d", CV::CV_TYPE_CLUSTER, clNum);
            topCluster = oaCluster::create(topBlock, clName,
                    oacClusterTypeInclusive);
            //cout<<" cl name "<<DB::getBuffer(clName)<<endl;
        }
        catch (oaException &excp) {
            cerr<<"Error: creating oaCluster: "<<excp.getMsg()<<endl;;
            return NULL;
        }
    }

    // a cluster specified, process the cluster
    // interprete the cluster members
    Iter<ClusterPtr> it(cluster->getMembers());

    while (ClusterPtr *ptr = it.getNext()) {
        if (ptr->get1()) {
            // its a cluster, need to create a sub cluster and add to it
            oaCluster *sub = clusterToOACluster(topBlock, ptr->get1());
            if (sub) {
                sub->addToCluster(topCluster);
            }
        }
        else if (ptr->get2()){
            // its a inst type, add to cluster directly
            ptr->get2()->addToCluster(topCluster);
        }
    }
    return topCluster;
}

// convert a oaCluster to a oaGroup
oaGroup* 
Cluster::oaClusterToOAGroup(oaDesign *database, 
        oaCluster *cluster, oaGroup *topGroup)
{
    if (!cluster) {
        return NULL;
    }
    if (topGroup) {
        database = (oaDesign*) topGroup->getOwner();
        if (!topGroup->isEmpty()) {
            // clear the group before conversion, but not destroyed
            clearGroupHier(topGroup);
        }
    }
    else if (!database) {
        return NULL;
    }

    // create a new top group
    if (!topGroup) {
        topGroup = oaGroup::create(database, //database
                CV::CV_TYPE_GROUP,
                oacCollectionGroupType, // group type
                false, // no name unique
                false, // no order
                oacNeverGroupDeleteWhen // delete manually
                );
    }
    // a oaCluster specified, process the cluster
    // interprete the cluster members

    // get top oaClusters
    oaIter<oaCluster> cit(cluster->getClusters());
    while (oaCluster *oacls = cit.getNext()) {
        // its a cluster type, need to create a sub group and add to it
        oaGroup *sub = oaClusterToOAGroup(database, oacls);
        if (sub) {
            oaGroupMember::create(topGroup, sub);
        }
    }

    // process top oaInst
    oaIter<oaInst> iit(cluster->getInsts());
    while (oaInst *inst = iit.getNext()) {
        // its a inst type, add to group directly
        oaGroupMember::create(topGroup, inst);
    }

    return topGroup;
}
// convert a Cluster to a oaGroup
oaGroup* 
Cluster::clusterToOAGroup(oaDesign *database, 
        Cluster *cluster,
        oaGroup *topGroup)
{
    if (!cluster) {
        return NULL;
    }
    if (topGroup) {
        database = (oaDesign*) topGroup->getOwner();
        if (!topGroup->isEmpty()) {
            // clear the group before conversion, but not destroyed
            clearGroupHier(topGroup);
        }
    }
    else if (!database) {
        return NULL;
    }

    // create a new top group
    if (!topGroup) {
        topGroup = oaGroup::create(database, //database
                CV::CV_TYPE_GROUP,
                oacCollectionGroupType, // group type
                false, // no name unique
                false, // no order
                oacNeverGroupDeleteWhen // delete manually
                );
    }
    // a cluster specified, process the cluster
    // interprete the cluster members
    Iter<ClusterPtr> it(cluster->getMembers());

    while (ClusterPtr *ptr = it.getNext()) {
        if (ptr->get1()) {
            // its a cluster type, need to create a sub group and add to it
            oaGroup *sub = clusterToOAGroup(database, ptr->get1());
            if (sub) {
                oaGroupMember::create(topGroup, sub);
            }
        }
        else if (ptr->get2()){
            // its a inst type, add to group directly
            oaGroupMember::create(topGroup, ptr->get2());
        }
    }
    return topGroup;
}

// topCluster: the topCluster if user has created one
// oaclusters, the sub oaClusters to patch to topCluster
// the final top oaCluster is returned
oaCluster*
Cluster::createTopCluster(oaCluster *topCluster,
        Array<oaCluster*> &oaclusters)
{
    if (!oaclusters.getNumElements() || !oaclusters[0]) {
        return topCluster;
    }
    oaBlock *top = oaclusters[0]->getBlock();

    for (oaUInt4 i = 1; i < oaclusters.getNumElements(); i++) {
        if (oaclusters[i] && oaclusters[i]->getBlock() != top) {
            cerr<<"Error: not consistent clusters specified "<<endl;
            return topCluster;
        }
    }

    if (topCluster) {
        // clear the cluster before conversion, but not destroyed
        clearClusterHier(topCluster);
    }
    else  {
        try {
            oaUInt4 clNum = top->getClusters().getCount() + 1;
            oaString clName = oaString(CV::CV_TYPE_CLUSTER) + "_" + clNum;
            topCluster = oaCluster::create(top, clName,
                    oacClusterTypeInclusive);
        }
        catch (oaException &excp) {
            cerr<<"Error: creating oaCluster: "<<excp.getMsg()<<endl;;
            return NULL;
        }
    }

    for (oaUInt4 i = 0; i < oaclusters.getNumElements(); i++) {
        if (oaclusters[i]) {
            oaclusters[i]->addToCluster(topCluster);
        }
    }
    return topCluster;
}
// topGroup: the topGroup if user has created one
// groups, the sub oaGroups to linked to topGroup
// the final top oaGroup is returned
oaGroup*
Cluster::createTopGroup(oaGroup *topGroup, Array<oaGroup*> &groups)
{
    if (!groups.getNumElements() || !groups[0]) {
        return topGroup;
    }
    oaDesign *top = (oaDesign*) groups[0]->getOwner();

    for (oaUInt4 i = 1; i < groups.getNumElements(); i++) {
        if (groups[i] && groups[i]->getOwner() != (oaObject*) top) {
            cerr<<"Error: not consistent groups specified "<<endl;
            return topGroup;
        }
    }

    if (topGroup) {
        // clear the cluster before conversion, but not destroyed
        if (topGroup->getOwner() != (oaObject*) top) {
            cerr<<"Error: not consistent top group specified "<<endl;
            return topGroup;
        }
        clearGroupHier(topGroup);
    }
    else  {
        try {
            topGroup = oaGroup::create(top, //database
                    CV::CV_TYPE_GROUP,
                    oacCollectionGroupType, // group type
                    false, // no name unique
                    false, // no order
                    oacNeverGroupDeleteWhen // delete manually
                    );
        }
        catch (oaException &excp) {
            cerr<<"Error: creating oaGroup: "<<excp.getMsg()<<endl;;
            return NULL;
        }
    }

    for (oaUInt4 i = 0; i < groups.getNumElements(); i++) {
        if (groups[i]) {
            oaGroupMember::create(topGroup, groups[i]);
        }
    }
    return topGroup;
}

// This function will only leaves the top oacluster (passed in) empty, 
// all its members (both insts and sub clusters) will all be destroyed.
void 
Cluster::clearClusterHier(oaCluster* oacluster, const bool delRoot)
{
    if (!oacluster) {
        // no need to clean
        return;
    }

    // check sub clusters
    oaIter<oaCluster> iter(oacluster->getClusters());

    while (oaCluster *memb = iter.getNext()) {
        // clear and destroy sub clusters
        clearClusterHier(memb, true);
    }

    if (delRoot) {
        oacluster->destroy();
    }
    else {
        // create a new one for the root cluster because the former oaCluster
        // doesn't offer a "clear" method to quickly clear all members
        oaString rootName;
        oacluster->getName(rootName);
        oaBlock *top = oacluster->getBlock();
        oaClusterType type = oacluster->getClusterType();
        oacluster->destroy();
        oacluster = oaCluster::create(top, rootName, type);
    }
}

// This function will only leaves the top oagroup (passed in) empty, 
// all its members (both insts and sub groups) will all be destroyed.
void 
Cluster::clearGroupHier(oaGroup *oagroup, const bool delRoot)
{
    if (!oagroup || oagroup->isEmpty()) {
        // no need to clean
        return;
    }

    oaCollection<oaGroupMember, oaGroup> subs = oagroup->getMembers();
    oaIter<oaGroupMember> iter(subs);

    while (oaGroupMember *memb = iter.getNext()) {
        if (memb->getObject()->getType() == oacGroupType) {
            oaGroup *group = (oaGroup*) memb->getObject();
            clearGroupHier(group, true);
        }
        else {
            memb->destroy();
        }
    }

    if (delRoot) {
        oagroup->destroy();
    }
}
// Clusters flattened alos use the CV's ClusterPtr address, please don't 
// release those intermediate CVs before you have properly store all
// ClusterPtrs, see example in CV::flatten which invokes this function
Cluster*
Cluster::flatten(Cluster *toCluster, const bool delSub)
{
    if (isEmpty()) {
        return NULL;
    }
    if (!toCluster) {
        toCluster = new Cluster();
    }

    Iter<ClusterPtr> it(getMembers());

    while (ClusterPtr *clPtr = it.getNext()) {
        if (clPtr->get1()) {
            clPtr->get1()->flatten(toCluster, delSub);
            if (delSub) {
                delete clPtr->get1();
                *clPtr = (Cluster*) NULL;
            }
        }
        else if (clPtr->get2()) {
            // just a top oaInst, add it directly
            toCluster->add(clPtr);
        }
    }

    return toCluster;
}

void
Cluster::add(Cluster *cluster)
{
    Iter<ClusterPtr> it(cluster->getMembers());
    while (ClusterPtr *clPtr = it.getNext()) {
        add(clPtr);
    }
}

// ClusterView testing utilities
void
ClusterView::checkClusterHierarchy(Array<ClusterPtr*> &topClusters)
{
    oaBox bbox;
    Array<ClusterPtr*> nextLevel;
    for (UInt4 i = 0; i < topClusters.getNumElements(); i++) {
        ClusterPtr *ptr = topClusters[i];
        cout<<"Checking "<<i<<"th cluster"<<endl;
        if (ptr->get1()) {
            Iter<ClusterPtr> it(ptr->get1()->getMembers());
            int count = 0;
	    while (ClusterPtr *cls = it.getNext()) {
                if (cls->get1()) {
                    cout<<"  Cluster ele: "<<cls->get1()<<"with "
                    <<cls->get1()->getNumElements()<<"elements."<<endl;
                    nextLevel.add(cls);
                }
                else if (cls->get2()) {
		    cout<<"i "<<count<<endl;
		    cout<<"get2() "<<cls->get2()<<" "<<endl;      
		    cls->get2()->getBBox(bbox);                   
		    cout<<"The approximate bounding box of inst is W:"<<bbox.getWidth()<<" H:"<<bbox.getHeight()<<endl;
		count++;
		}
                else {
                    cout<<"  Error: empty ClusterPtr found"<<endl;
                    assert(0);
                }
            }
	    cout<<"count "<<count<<endl;
        }
        else {
            cout<<"  It's not a cluster"<<endl;
        }
    }

    for (UInt4 i = 0; i < nextLevel.getNumElements(); i++) {
        cout<<"Checking sub level cluster "<<i<<endl;
        checkClusterHierarchy(nextLevel);
    }

}



//clusterview test cvTopcluster     
void
ClusterView::checkFpCluster(Array<ClusterPtr> &cvTopClusters)
{
    oaBox bbox;
    for (UInt4 i = 0; i < cvTopClusters.getNumElements(); i++) {
	ClusterPtr *ptr = &cvTopClusters[i];
	cout<<"Checking "<<i<<" th cluster"<<" have "<<ptr->get1()->getNumElements()<<" elements "<<endl;
	  cout<<" ptr->get1() add "<<ptr->get1()<<endl;
	if (ptr->get1()) {
	    Iter<ClusterPtr> itaddr(ptr->get1()->getMembers());
	    cout<<"itaddr.getNext address "<<endl;
	    for(UInt4 j = 0; j < ptr->get1()->getNumElements(); j++)
		cout<<itaddr.getNext()<<" ";
	    cout <<endl;
	    cout<<"it.getNext()->get2() address"<<endl;
	    Iter<ClusterPtr> itval(ptr->get1()->getMembers());
	    for(UInt4 j = 0; j < ptr->get1()->getNumElements(); j++) {
		ClusterPtr *cls = itval.getNext(); 	
		cout<<"j "<<j<<" ";	
		cout<<"getNext "<<cls<<" ";
		cout<<"get2() "<<cls->get2()<<" "<<endl;
		cls->get2()->getBBox(bbox);
		cout<<"The approximate bounding box of inst is W:"<<bbox.getWidth()<<" H:"<<bbox.getHeight()<<endl;
	    }
	    cout<<endl;
            
	    int count = 0;
	    cout<<"Begin checkFpcluster Test  "<<endl; 
	    Iter<ClusterPtr> it(ptr->get1()->getMembers());
	    while (ClusterPtr *cls = it.getNext()) {
		if (cls->get1()) {
		    cout<<"  Cluster ele: "<<cls->get1()<<"with "
			<<cls->get1()->getNumElements()<<"elements."<<endl;
		}
		else if (cls->get2()) {
		    count++;
		}
		else {
		    cout<<"  Error: empty ClusterPtr found"<<endl;
		    assert(0);
		}
	    }
	    cout<<"count "<<count<<endl;
	    cout<<"End checkFPcluster Test"<<endl;
	}                                                     
	else {                                                
	    cout<<"  It's not a cluster"<<endl;               
	}                                                     
    }                                                                          
}    
