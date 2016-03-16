//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <regionQuery.h>
//
// includes all region queries used
//
// Author: Lu Yongqiang
// History: 2009/10/16 created by Yongqiang
//*****************************************************************************
//*****************************************************************************

#if !defined(_SC_REGION_QUERY_H_)
#define _SC_REGION_QUERY_H_

#include "base.h"
#include "design.h"

//*****************************************************************************
// Placement Blockage query
//*****************************************************************************

class QueryPlcBlockage : public oaPlacementBlockageQuery
{
public:
    inline QueryPlcBlockage() { };

    // This would return an array reference which is in the class object
    // This is fast but need careful operation due to the reference to class obj
    inline Array<oaBlockage*>& queryBlockages(const oaBox &region)
    {
        qbBlockages.clear();
        query(Design::getCurrent(), region);
        return qbBlockages;
    };

    // This would just return an array copy
    // This is safe but a little slow since there is an array copy op
    inline void queryBlockages(const oaBox &region, Array<oaBlockage*> &blockages)
    {
        blockages = queryBlockages(region);
    };

    // return the parts inside the window, i.e. if the blockage is intersecting
    // the window, return the part totally inside the window; if the blockage
    // is inside the window, return it directly
    // NOTE, only rectangle blockages supported now
    void queryBlockageIntersected(const oaBox &region, Array<oaBox> &blockages);

protected:
    inline void queryBlockage(oaBlockage *blockage)
    {
        qbBlockages.add(blockage);
    };

private:
    Array<oaBlockage*> qbBlockages;

}; 

//*****************************************************************************
// Routing LayerRange Blockage query 
//*****************************************************************************

class QueryLayerRangeBlockage : public oaLayerRangeBlockageQuery
{
public:
    inline QueryLayerRangeBlockage() { };

    // This would return an array reference which is in the class object
    // This is fast but need careful operation due to the reference to class obj
    inline Array<oaBlockage*>& queryBlockages(const oaBox &region)
    {
        qbBlockages.clear();
        query(Design::getCurrent(), region);
        return qbBlockages;
    };

    // This would just return an array copy
    // This is safe but a little slow since there is an array copy op
    inline void queryBlockages(const oaBox &region,
            Array<oaBlockage*> &blockages)
    {
        blockages = queryBlockages(region);
    };

    // query LayerRange blockages that must have layer "layer" contained
    void queryBlockages(oaLayerNum layer,
            const oaBox &region, 
            Array<oaBlockage*> &blockages);

    // return the parts inside the window, i.e. if the blockage is intersecting
    // the window, return the part totally inside the window; if the blockage
    // is inside the window, return it directly
    // NOTE, only rectangle blockages supported now
    void queryBlockageIntersected(oaLayerNum layer, 
            const oaBox &region, Array<oaBox> &blockages);

protected:
    inline void queryBlockage(oaBlockage *blockage)
    {
        qbBlockages.add(blockage);
    };

private:
    Array<oaBlockage*> qbBlockages;

}; 

//*****************************************************************************
// oaInst query 
//*****************************************************************************

class QueryInst : public oaInstQuery
{
public:
    inline QueryInst () { };

    // This would return an array reference which is in the class object
    // This is fast but need careful operation due to the reference to class obj
    inline Array<oaInst*>& queryInsts(const oaBox &region)
    {
        qiInsts.clear();
        query(Design::getCurrent(), region); 
        return qiInsts;
    };
    // This would just return an array copy
    // This is safe but a little slow since there is an array copy op
    inline void queryInsts(const oaBox &region, Array<oaInst*> &insts)
    {
        insts = queryInsts(region);
    };

protected:
    inline void queryInst(oaInst *inst)
    {
        qiInsts.add(inst);
    };

private:
    Array<oaInst*> qiInsts;
};

//*****************************************************************************
// oaShape query 
//*****************************************************************************
class QueryShape : public oaShapeQuery
{
public:
    inline QueryShape() { };

    // This would return an array reference which is in the class object
    // This is fast but need careful operation due to the reference to class obj
    inline Array<oaShape*>& queryShapes(oaLayerNum layer, 
            oaPurposeNum purposeNum,
            const oaBox &region)
    {
        qsShapes.clear();
        query(Design::getCurrent(), layer, purposeNum, region); 
        return qsShapes;
    };
    // This would just return an array copy
    // This is safe but a little slow since there is an array copy op
    inline void queryShapes(oaLayerNum layer, 
            oaPurposeNum purposeNum,
            const oaBox &region,
            Array<oaShape*> &shapes)
    {
        shapes = queryShapes(layer, purposeNum, region);
    };

protected:
    inline void queryShape(oaShape *shape)
    {
        qsShapes.add(shape);
    };

private:
    Array<oaShape*> qsShapes;
};

#endif
