//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <net.h>
//
// The net related manipulations
//
// Author: Lu Yongqiang
// History: 2009/9/14 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined (_SC_NET_H_) 
#define _SC_NET_H_

#include <iostream>
#include "oa/oaDesignDB.h"
#include "term.h"
#include "array.h"
#include "occTraverser.h"

using namespace std;
using namespace oa;


class NetOA
{
public:
    inline NetOA();
    inline ~NetOA();

public:
    static inline oaNet* create(oaBlock *block) 
    {
        return oaScalarNet::create(block);
    };

public:
    // utilities for net

    // get net driver, a instterm or term
    static NetTerm getDriver(const oaNet *net);
    // must return a oaInstTerm driver, otherwise NULL
    static oaInstTerm* getDriverInstTerm(const oaNet *net);
    // must return a oaInstTerm driver, otherwise NULL
    static OccNetTerm getDriver(const oaOccNet *net);
    // must return a oaInstTerm driver, otherwise NULL
    static oaOccInstTerm* getDriverInstTerm(const oaOccNet *net);

    // get bbox
    static void getBBox(const oaNet *net, oaBox &bbox);
    // get all single bits
    // collect all bits of the net (if it is bus/bundle net), otherwise only
    // includes itself
    static void getAllBits(const oaNet *net, Array<oaBitNet*> &bits)
    {
        oaIter<oaBitNet> nbIt(net->getSingleBitMembers());
        while (oaBitNet *bitNet = nbIt.getNext()) {
            bits.add(bitNet);
        }
    };
    static void getAllBits(const oaOccNet *net, Array<oaOccBitNet*> &bits)
    {
        oaIter<oaOccBitNet> nbIt(net->getSingleBitMembers());
        while (oaOccBitNet *bitNet = nbIt.getNext()) {
            bits.add(bitNet);
        }
    };
    
    // get power/ground net
    static oaNet* getPower(const oaBlock *block);
    static oaNet* getGround(const oaBlock *block);

    // is power/ground net
    static inline bool isPG(const oaNet *net)
    {
        return isPower(net) || isGround(net);
    };

    static inline bool isPower(const oaNet *net)
    {
        return (net && net->getSigType() == oacPowerSigType);
    };

    static inline bool isGround(const oaNet *net)
    {
        return (net && net->getSigType() == oacGroundSigType);
    };
    // is clock
    static inline bool isClock(const oaNet *net) 
    {
        return (net && net->getSigType() == oacClockSigType);
    };
    // is power/ground net
    static inline bool isPG(const oaOccNet *net)
    {
        return isPower(net) || isGround(net);
    };

    static inline bool isPower(const oaOccNet *net)
    {
        return (net && isPower(net->getNet()));
    };

    static inline bool isGround(const oaOccNet *net)
    {
        return (net && isGround(net->getNet()));
    };

    // OCC
    static inline void getAllOccNets(oaOccurrence *top, Array<oaOccNet*> &nets)
    {
        OccNets onets(top, nets);
    };

    // for report
    static void reportNet(const oaNet *net);
    static void reportNet(const oaOccNet *net);
    static void reportNet(oaOccurrence *top, const oaString &netName);
    static void reportNet(oaBlock *top, const oaString &netName);
    
};


#endif
