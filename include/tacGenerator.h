//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <tacGenerator.h>
//
// The auto command traversal generator
//
// Author: YAN Haixia
// History: 2009/9/16 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined(_TAC_GENERATOR_H_)
#define _TAC_GENERATOR_H_

#define TACDEQUETCMDSIZE 20
#include "base.h"

class TACInterp;            
class TACCommand;          

class TACDequeTCmd 
{
    friend class TACDequeForwTCmd;
    friend class TACDequeBackTCmd;
public:
    enum DequeStyle { DEQUE_STYLE, QUEUE_STYLE, STACK_STYLE };

    /* -------------- Construction and Destruction -----------*/


    // Create a new instance of a deque.

    TACDequeTCmd(int pExpectedSize = TACDEQUETCMDSIZE,
            DequeStyle pStyle = DEQUE_STYLE);
    // Destroys and reclaims any memory associated with the deque.
    ~TACDequeTCmd();

    /* -------------- General Operations ---------------------- */

    inline int size() { return iCount; };

    inline bool isEmpty() { return size() == 0; };

    /* -------------- Stack Style Operations ------------------ */

    inline void push(TACCommand* pElement) {
        if (iLeft >= 0) iFront->set(iLeft--, pElement);
        else newLeft(pElement);
        if (iCount++ == 0) iRight++;
    };

    inline TACCommand* pop() {
        assert(iCount > 0);
        if (iLeft < (iFront == iBack ? iRight : iVecSize-1)) {
            if (--iCount == 0) iRight--;
            return iFront->get(++iLeft);
        } else {
            return popComplex();
        }
    };

    inline TACCommand* first() {
        assert(iCount > 0);
        if (iLeft < (iFront == iBack ? iRight : iVecSize-1)) {
            return iFront->get(iLeft+1);
        } else {
            return iFront->getRight()->get(0);
        }
    };

    /* -------------- Queue Style Operations ------------------ */

    inline void put(TACCommand* pElement) {
        if (iRight < iVecSize) {
            iBack->set(iRight++, pElement);
        }
        else {
            newRight(pElement);
        }
        if (iCount++ == 0) {
            iLeft--;
        }
    };

    inline TACCommand* get() { return pop(); };


    inline TACCommand* pull() {
        assert(iCount > 0);
        if (iRight > (iFront == iBack ? iLeft : 0)) {
            if (--iCount == 0) iLeft++;
            return iBack->get(--iRight);
        } else {
            return pullComplex();
        }
    };


    inline TACCommand* last() {
        assert(iCount > 0);
        if (iRight > (iFront == iBack ? iLeft : 0)) {
            return iBack->get(iRight-1);
        } else {
            return iBack->getLeft()->get(0);
        }
    };


    TACCommand* fromLeft(int pIndex);

    TACCommand* fromRight(int pIndex);

    class Vec {
        public:
            inline Vec(int pSz, Vec *pLeft, Vec *pRight) {
                iData = new TACCommand*[pSz];
                iLeft = pLeft;
                iRight = pRight;
            };

            inline TACCommand* get(int pLoc) { return iData[pLoc]; };
            inline void set(int pSpot, TACCommand* pElem) { iData[pSpot] = pElem; };
            inline Vec *getLeft() { return iLeft; };
            inline Vec *getRight() { return iRight; };
            inline void setLeft(Vec *pLeft) { iLeft = pLeft; };
            inline void setRight(Vec *pRight) { iRight = pRight; };
        protected:
            TACCommand* *iData;
            Vec *iLeft, *iRight;
    };

    int iVecSize;			
    int iCount;		
    Vec *iFront;	
    Vec *iBack;
    int iLeft;
    int iRight;		

    void newLeft(TACCommand* pElem);

    void newRight(TACCommand* pElem);

    TACCommand* popComplex();

    TACCommand* pullComplex();

};


class TACDequeForwTCmd 
{
public:
    inline TACDequeForwTCmd(const TACDequeTCmd* pDeque) 
    {
            iDeque = pDeque;
            iCurrent = 0;
    };

    
    inline void first() 
    {
  
            iSlot = iDeque->iLeft+1;
            iCurrent = (iDeque->iCount > 0 ? iDeque->iFront : 0);
    };

    
    inline void next() 
    {
        if (++iSlot >= (iCurrent == iDeque->iBack ? iDeque->iRight : iDeque->iVecSize)) {
            if (iCurrent != 0) iCurrent = iCurrent->getRight();
            iSlot = 0;
        }
    };

    inline bool isDone() { return (iCurrent == 0); };


    inline TACCommand* here() 
    { 
        assert(iCurrent != 0);
        return iCurrent->get(iSlot);
    };

protected:
    const TACDequeTCmd *iDeque;
    TACDequeTCmd::Vec *iCurrent;
    int iSlot;
};


class TACDequeBackTCmd 
{
public:
    
    inline TACDequeBackTCmd(const TACDequeTCmd* pDeque) {
        iDeque = pDeque;
        iCurrent = 0;
    };


    inline void first() {
        iSlot = iDeque->iRight-1;
        iCurrent = (iDeque->iCount > 0 ? iDeque->iBack : 0);
    };


    inline void next() {
        if (--iSlot <= (iCurrent == iDeque->iFront ? iDeque->iLeft : -1)) {
                if (iCurrent != 0) iCurrent = iCurrent->getLeft();
                iSlot = iDeque->iVecSize-1;
        }
    };

    
    inline bool isDone() { return (iCurrent == 0); };


    inline TACCommand* here() { assert(iCurrent != 0); return iCurrent->get(iSlot); };

protected:
    const TACDequeTCmd *iDeque;
    TACDequeTCmd::Vec *iCurrent;
    int iSlot;
};


class TACStaticRegister 
{
public:
    TACStaticRegister() { };
    ~TACStaticRegister() { };
public:
    static inline void delayRegister(TACCommand *pCmd) 
    {
        getAllCmds()->put(pCmd);
    }
private:
    static inline TACDequeTCmd* getAllCmds()
    {
        // signleton for static sAllCmds
        static TACDequeTCmd* sAllCmds = new TACDequeTCmd();
        return sAllCmds;
    }
private:

    friend class TACStaticGenerator;
};

class TACStaticGenerator 
{
public:

    inline TACStaticGenerator()
    {
        iEnumerator = new TACDequeForwTCmd(TACStaticRegister::getAllCmds());
        iEnumerator->first();
    };

    inline ~TACStaticGenerator()
    {
	delete iEnumerator;
	iEnumerator = (TACDequeForwTCmd*) 0;
    };

    TACCommand* next();

private:
    TACDequeForwTCmd* iEnumerator;
};

#endif
