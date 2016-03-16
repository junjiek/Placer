//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <tacGenerator.cpp>
//
// The auto command traversal generator
//
// Author: YAN Haixia
// History: 2009/9/16 created by Yongqiang
//*****************************************************************************
//*****************************************************************************

#include "tacGenerator.h"

TACDequeTCmd::TACDequeTCmd(int pExpectedSize, DequeStyle pStyle)
{

    iVecSize = pExpectedSize;
    iCount = 0;
    iFront = iBack = new Vec(iVecSize, 0, 0);
    switch (pStyle) {
        case DEQUE_STYLE:
            iLeft = iRight = iVecSize/2;
            break;
        case QUEUE_STYLE:
            iLeft = iRight = iVecSize-1;
            break;
        case STACK_STYLE:
            iLeft = iRight = 0;
            break;
    }
}
    // Destroys and reclaims any memory associated with the deque.
TACDequeTCmd::~TACDequeTCmd()
{
    Vec *lDoomed, *lHere;

    lHere = iFront;
    while (lHere != 0) {
        lDoomed = lHere;
        lHere = lHere->getRight();
        delete lDoomed;
    }
    iFront = iBack = 0;
}


TACCommand* 
TACDequeTCmd::fromLeft(int pIndex)
{
    assert((pIndex >= 0) && (pIndex < iCount));

    if (pIndex < (iFront == iBack ? iCount : (iVecSize - iLeft - 1))) {
        return iFront->get(iLeft + pIndex + 1);
    } else if (pIndex >= (iCount - iRight)) {

        return iBack->get(pIndex - (iCount - iRight));
    } else {
        Vec *lHere = iFront->getRight();
        pIndex -= iVecSize - iLeft - 1;
        while (lHere && (pIndex >= iVecSize)) {
            pIndex -= iVecSize;
            lHere = lHere->getRight();
        }
        assert(lHere != 0);
        return lHere->get(pIndex);
    }
}

TACCommand* 
TACDequeTCmd::fromRight(int pIndex)
{
    assert((pIndex >= 0) && (pIndex < iCount));

    pIndex = iCount - pIndex - 1;

    if (pIndex < (iFront == iBack ? iCount : (iVecSize - iLeft - 1))) {

        return iFront->get(iLeft + pIndex + 1);
    } 
    else if (pIndex >= (iCount - iRight)) {
        return iBack->get(pIndex - (iCount - iRight));
    } 
    else {

        Vec *lHere = iBack->getLeft();
        int lOffset = iCount - iRight - iVecSize;
        while (lHere && (lOffset > pIndex)) {
            lOffset -= iVecSize;
            lHere = lHere->getLeft();
        }
        assert(lHere != 0);
        return lHere->get(pIndex - lOffset);
    }
}
void 
TACDequeTCmd::newLeft(TACCommand* pElem)
{
    Vec *lNewFront = new Vec(iVecSize, (Vec*) 0, iFront);

    iFront->setLeft(lNewFront);
    iLeft = iVecSize-1;
    iFront = lNewFront;
    iFront->set(iLeft--, pElem);
}

void 
TACDequeTCmd::newRight(TACCommand* pElem)
{
    Vec *lNewBack = new Vec(iVecSize, iBack, (Vec*) 0);

    iBack->setRight(lNewBack);
    iRight = 0;
    iBack = lNewBack;
    iBack->set(iRight++, pElem);
}

TACCommand* 
TACDequeTCmd::popComplex()
{
    Vec *lNewFront = iFront->getRight();

    lNewFront->setLeft((Vec*)0);
    iFront->setRight((Vec*)0);
    iLeft = 0;
    delete iFront;
    iFront = lNewFront;
    iCount--;
    return iFront->get(iLeft);
}

TACCommand* 
TACDequeTCmd::pullComplex()
{
    Vec *lNewBack = iBack->getLeft();

    lNewBack->setRight((Vec*)0);
    iBack->setLeft((Vec*)0);
    iRight = iVecSize-1;
    delete iBack;
    iBack = lNewBack;
    iCount--;
    return iBack->get(iRight);
}

TACCommand* 
TACStaticGenerator::next()
{
    TACCommand* lResult = (TACCommand*) 0;

    if (!iEnumerator->isDone()) {
    //printf("this is exist one\n");
            lResult = iEnumerator->here();
            iEnumerator->next();
    }
    return lResult;
}
