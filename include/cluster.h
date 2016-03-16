//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <cluster.h>
//
// class Cluster main entry classes, for foorplan/placement block domain usage
//
// Author: Lu Yongqiang
// History: 2009/4/21 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined (_CLUSTER_H_)
#define _CLUSTER_H_
#include "base.h"
#include "diptr.h"
#include "mathtype.h"

// *****************************************************************************
// class ClusterView
// A circuit view based on cluster-type class Cluster.
// ClusterView regards Cluster as top objects, which includes member objects. 
// It records top level connections (oaNet) based on top level clusters
// NOTE: one can derive a new class from a template class of this template, 
//       which implements specific clustering algorithm by overrides 
//       createClusters()
// When the topmost CV is deleted, all the lower-level CV are all deleted 
//      automatically too.
// Usage: user only need to custimize the createCluster() function, in which
//        a clustering algorithm is used to create the clusters and construct
//        the cvTopClusters array. After that, user just need to call 
//        doClustering() to finish this CV.
//
//E.g.   class MyCV: public CV {.overload fuction createClusters()..};
//       MyCV *cvBase = new MyCV(topBlock);
//       MyCV *cv1 = new MyCV(cvBase);
//       cv1.doClustering();
// *****************************************************************************


// forward declarations
class ClusterView;
typedef class ClusterView CV;
class Cluster;

// *****************************************************************************
// class ClusterPtr
// a di-cluster-inst ptr class which has two pointers inside
// This a wrapper for Cluster and oaInst pointers together, which help the 
// Cluster class manipulate clustering algorithm implemenation uniformly.
// *****************************************************************************
class ClusterPtr : public DiPtr<Cluster, oaInst>
{
public:
    inline ClusterPtr() : id(0) {};
    inline ClusterPtr(ClusterPtr *ptr)
        : DiPtr<Cluster, oaInst>(ptr),
          id(0) {};
    inline ClusterPtr(Cluster *ptr) 
        : DiPtr<Cluster, oaInst>(ptr),
          id(0) {};
    inline ClusterPtr(oaInst *ptr) 
        : DiPtr<Cluster, oaInst>(ptr),
          id(0) {};
    // please use CV's getClusterID/setClusterID to get/set its ID, 
    // this is for the safety reason

private:
    // id for some indexng need, such as setting up quadratic equation
    // NOTE: no get/set function for this id, you must get the ID from
    //       ClusterView since this id is managed by it automatically.
    unsigned int id; 
    friend class ClusterView;
};
typedef OneDRange<unsigned int> OneDUIntRange;

class ClusterView
{
public:
    typedef enum _CLUSTER_TYPE_ {
        CLUSTER_INST = 0,
        CLUSTER_CLUSTER = 1
    } ClusterType;
   
    typedef enum _IO_CLASS_ {
	    PI = 0,
	    PIO = 1,
	    PO = 2,
	    NUM_OF_IOTYPES =3 
    } IO_CLASS;

    typedef enum _COMPONENT_CLASS_ {
	    INST_UNPLACED = 0,
	    INST_FIXED = 1,
	    NUM_OF_TYPES = 2
    } COMP_CLASS;

public:
    // NO default constructor offered

    ClusterView(oaBlock *topBlock);
    ClusterView(ClusterView *cv);
    virtual ~ClusterView();

    // create on empty CV
    // pairs returns the oaCluster/oaGroup and Cluster correlation pairs
    void create(oaCluster *topCluster, HashPtr<oaCluster*, Cluster*> &pairs);
    void create(oaGroup *topGroup, HashPtr<oaGroup*, Cluster*> &pairs);

public:
    // For floorplan, flatten the CV hierarchy to only two-level CVs:
    // baseCV and top CV
    void flatten();
    static unsigned int getNumElements(const ClusterPtr *clPtr);
    static oaPlacementStatus getPlacementStatus(const ClusterPtr *clPtr);
    static bool compareClusters(ClusterPtr* const &p1, ClusterPtr* const &p2);
  
public:

    // After you create a CV, you at first need to set the cluster num,
    // otherwise the automatic scheme is needed in clustering algorithm
    inline void setMaxClusterNum(const unsigned int maxNum)
    {
        cvMaxClusterNum = maxNum;
    };

    // Then you can use doClustering to finish the cluster view
    void doClustering();

    // The following are some utility functions to help implement floorplan or
    // placement, including, reading, writing operations
    inline Array<ClusterPtr*>& getTopClusters()
    {
        return cvTopClusterView;
    }
  
  
    inline Array<oaNet*>& getTopNets()
    {
        return cvTopNets;
    };
  
  
    // the root CV, that is the oaBlock based CV, the all-inst CV (CLUSTER_INST)
    inline ClusterView* getRoot()
    {
        CV *cv = this;
        while (cv && cv->cvClusterType != CLUSTER_INST) {
            cv = cv->cvLowerLevelCV;
        }
        return cv;
    }
    // only feasible for CLUSTER_INST
    inline unsigned int getClusterID(oaInst *inst)
    {
        assert(cvClusterType == CLUSTER_INST);
        return (unsigned int) cvInstIdDef->get(inst);
    };

    inline unsigned int getClusterID(ClusterPtr *clPtr)
    {
        // verify sanity
        // if this fails, please check if you use addTopCluster
        // to operate the TopCluster array.
        assert(clPtr->id < cvTopClusterView.getNumElements() &&
               clPtr == cvTopClusterView[clPtr->id]);
        return clPtr->id;
    };

    // user can customize the cluster sequence, but please make sure using
    // this function to change the ids.
    // NOTE: after id changed to target, the target slot is also overwritten 
    bool setClusterID(ClusterPtr *clPtr, unsigned int idIn);

    // if this function doesn't return the range, that means those types
    // don't exist in the pads.
    void getIOIndexRange(IO_CLASS type, 
		    unsigned int &begin, unsigned int &end);  
    
    // if this function doesn't return the range, that means those types
    // don't exist in the clusters.
    void getIndexRange(COMP_CLASS type, 
            unsigned int &begin, unsigned int &end);
    // get the term's parent cluster ptr
    ClusterPtr* getTermCluster(oaInstTerm *term);
    // add one term to cluster
    void addClusterTerm(oaInstTerm *term, ClusterPtr *clPtr);
    // get Cluster's terms
    Collection<oaInstTerm> getClusterTerms(ClusterPtr *clPtr);
    Collection<ClusterPtr> getClusterElements(ClusterPtr *clPtr);
    Collection<ClusterPtr> getNetClusters(oaNet *net);
    // clear all clusters
    void clear();


protected:
    virtual void createClusters();
    // only protected default constructor for an empty CV
    inline ClusterView()
        : cvClusterType(CLUSTER_CLUSTER),
          cvLowerLevelCV(NULL) {};
    // create their connections among them
    void createClusterNets();
    void completeCVInformation();
    inline ClusterType getClusterType()
    {
        return cvClusterType;
    }
    inline void addTopCluster(ClusterPtr *clPtr)
    {
        cvTopClusters.append(*clPtr);

    };
    inline void addTopCluster(oaInst *inst)
    {
        ClusterPtr ptr(inst);
        cvTopClusters.append(ptr);

    };
    void addTopCluster(Cluster *cluster)
    {
        ClusterPtr ptr(cluster);
        cvTopClusters.append(ptr);
    };

    void resetPadIndex(oaBlock*);
    void resetTopClusterIndex();
    bool isInCluster(ClusterPtr *elePtr, ClusterPtr *clRef);
    bool isClusterCutNet(oaNet *net, Array<ClusterPtr*> &parentMap);
    void createClusterCutNet(oaNet *net, Array<ClusterPtr*> &parentMap);
    // serve for oaCluster to CV
    ClusterView* convertOAClusterToCV(ClusterView *cv, 
            ClusterPtr *oaclPtr, oaUInt4 &begin, oaUInt4 &end);
    // serve for oaCluster to CV
    void convertOAClusterHierToClusterHier(
            HashPtr<oaCluster*, Cluster*> &pairs);
    // serve for oaGroup to CV
    ClusterView* convertOAGroupToCV(ClusterView *cv, 
            ClusterPtr *oaclPtr, oaUInt4 &begin, oaUInt4 &end);
    // serve for oaGroup to CV
    void convertOAGroupHierToClusterHier(HashPtr<oaGroup*, Cluster*> &pairs);

    // set up the CVs after Clusters created in cvTopClusters
    //void bottomupSetupCVsByClusters();

protected:
    // for testing
    void checkClusterHierarchy(Array<ClusterPtr*> &topClusters);
    void checkFpCluster(Array<ClusterPtr> &cvTopClusters);
    
public:
    static const char *CV_INST_ID_NAME;
    static const char* CV_TYPE_GROUP;
    static const char* CV_TYPE_CLUSTER;

protected:
    // When the CV is a inst-CV, it stores a oaIntAppDef for mapping inst id to 
    // ClusterPtr's id in cvTopClusterView; 
    // Otherwise, records the cluster's ports' inst and their parent 
    // clusterPtr (the // address of the array cvTopClusters' element,
    // i.e. ClusterPtr*)
    union {
        HashPtr<oaInst*, ClusterPtr*> *cvPortInstParents;
        oaIntAppDef<oaInst> *cvInstIdDef;
    };
    // top-level clusters of current CV, dia ptrs with possible cluster or inst
    // e.g. the first level CV based on oaBlock is a inst based CV which 
    //      points to oaInst rather than Clusters in InstClsuterPtrs.
    // This is the ClusterPtr body storage, please don't release it in lifetime
    // NOTE: if this is empty but cvTopClusterView is not empty, please maintain
    //       the pointers stored in cvTopClusterView, they need to be deleted
    //       explicitly because the are not using the array body address
    Array<ClusterPtr> cvTopClusters;

    // all nets, CLUSTER_INST mode has no this array, using block->getNets()
    Array<oaNet*> cvTopNets;

    // The user view on cvTopClusters, in which one can rearrange the sequence
    // By default, [0, numOfFixed) are the insts that not placed,
    // the rest is placed.
    // Use getIndexRange(INST_UNPLACED) to get the corresponding index range
    // in array cvTopClusterView.
    // The array will be setup and reset the sequence to default by function
    // resetTopClusterIndex
    // NOTE: DO not cluster fixed instances to any clusters,
    //       just leave them as insts in ClusterPtr.
    Array<ClusterPtr*> cvTopClusterView;
    // The array is classified to three-types of objects: PI, PIO PO 
    // Default sequence is , PI-PIO-PO
    Array<oaTerm*> cvPads;
    // give the range of the PAD_CLASS, [begin, end) in the array cvPads
    // e.g. PI's range is in cvClassRangs[PI] (PI is the enum 0)
    Array<OneDUIntRange> cvPadRanges;
    // the index range of floated and fixed inst
    Array<OneDUIntRange> cvClusterRanges;
    unsigned int cvMaxClusterNum;
    ClusterType cvClusterType;

    // If the CV is a block based CV (the base level CV), 
    // it stores the base oaBlock;
    // otherwise, it sotres the lower-level CV which it creates from.
    union {
        ClusterView *cvLowerLevelCV;
        oaBlock *cvTopBlock;
    };
};

// stores void* pointers, user is responsible for the type conversion of 
// the cluster members
class Cluster
{

public:
    // clear hierarchical oaGroup, only keep the top *group (passed in) left
    // by default (defRoot = false); otherwise the top will also be deleted.
    static void clearGroupHier(oaGroup *group, const bool delRoot = false);
    // clear hierarchical oaCluster, function is similar with clearGroupHier
    static void clearClusterHier(oaCluster *oacluster, const bool delRoot = false);
    // convert clusters to oaGroups/oaClusters
    static oaCluster* createTopCluster(oaCluster *topCluster, 
            Array<oaCluster*> &oaclusters);
    static oaGroup* createTopGroup(oaGroup *topGroup, Array<oaGroup*> &groups);
    // convert one Cluster to one oaGroup
    static oaGroup* clusterToOAGroup(oaDesign *database,
             Cluster *cluster, oaGroup *topGroup = NULL);
    // convert one Cluster to one oaCluster
    static oaCluster* clusterToOACluster(oaBlock *topBlock,
            Cluster *cluster, oaCluster *topCluster = NULL); 
    // convert one oaCluster to one oaGroup
    static oaGroup* oaClusterToOAGroup(oaDesign *database,
             oaCluster *cluster, oaGroup *topGroup = NULL);
    // convert one oaGroup to one oaCluster
    static oaCluster* oaGroupToOACluster(oaBlock *topBlock,
            oaGroup *group, oaCluster *topCluster = NULL); 

public:
    inline Cluster()
        : clOrigin(0, 0), 
          comType(CV::INST_UNPLACED)
    {
    };
    virtual inline ~Cluster()
    {
    };


public:
    // flatten hierarchical cluster (*this) to one-level cluster
    // toCluster: the flattened cluster stroage, default NULL
    // delSub: if to delete the sub clusters nested under this
    // return: the final flattened cluster
    // NOTE: this will not be delete after flattening.
    Cluster* flatten(Cluster *toCluster = NULL, const bool delSub = false);
    inline bool add(ClusterPtr *clPtr)
    {
        return clMembers.add(clPtr);
    };
    // add a clusters' members to current
    void add(Cluster *cluster);
    inline void removeFromCluster(ClusterPtr *clPtr)
    {
        clMembers.remove(clPtr);
    };
    inline bool isInCluster(ClusterPtr *clPtr) 
    {
        return clMembers.exists(clPtr);
    };
    inline unsigned int getNumElements() const
    {
        return clMembers.getNumElements();
    };
    inline bool isEmpty() const
    {
        return (clMembers.getNumElements() == 0);
    };
    inline Collection<ClusterPtr> getMembers()
    {
        return Collection<ClusterPtr>(&clMembers);
    };
    inline void addClusterTerm(oaInstTerm *term) 
    {
        clPortList.add(term);
    };

    inline Collection<oaInstTerm> getClusterTerms()
    {
        return Collection<oaInstTerm>(&clPortList);
    };

    // clear all members and ports
    inline void clear()
    {
        // The sub Cluster ptr are managed in CV, so, don't clear them here
        clMembers.clear();
        clPortList.clear();
        clOrigin = oaPoint(0, 0);
        clBoundary.setSize(0);
    };
    
    inline void setBoundary(oaPointArray &points)
    {
        clBoundary = points;
    };
    inline const oaPointArray& getBoundary() const
    {
        return clBoundary;
    };
    inline oaPoint& getOrigin()
    {
        return clOrigin;
    };
    inline void setOrigin(oaPoint &orig) 
    {
        clOrigin = orig;
    };

    inline void setType(CV::COMP_CLASS ctype) 
    {
        comType = ctype;
    };
    inline CV::COMP_CLASS getType() 
    {
        return comType;
    };

protected:
    // clusterPtr table
    HashPtrSet<ClusterPtr*> clMembers;
    List<oaInstTerm*> clPortList;
    // lef-bottom origin point
    oaPoint clOrigin;
    // bounding boundary
    oaPointArray clBoundary;
    CV::COMP_CLASS comType;
};



#endif
