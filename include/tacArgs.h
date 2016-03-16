//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <tacArgs.h>
//
// The auto command args class
//
// Author: YAN Haixia
// History: 2009/6/16 created by YAN Haixia
//*****************************************************************************
//*****************************************************************************
#if !defined(_TAC_ARGS_H_)
#define _TAC_ARGS_H_

#include "tacCommand.h"

// The arguments to TCL command
class TACArgs 
{
public:
    TACArgs(int pArgc, Tcl_Obj *const pObjv[]);

    TACArgs(string* pArgList, int pNumArgs);

    ~TACArgs();

    inline operator const void* () const { return iOk ? this : 0; };

    inline int getCount() { return iNumArgs; };

    inline bool hasMoreArgs() { return iCurArg < iNumArgs; };

    bool nextArgAsInt(int& pResult);

    bool nextArgAsDbl(double& pResult);

    bool nextArgAsStr(string& pResult);

    bool nextArgAsStrPara(string& pResult);

    inline void back() { if (iCurArg > 0) iCurArg--; };

    inline void reset(int pSpot = 0) 
    { 
        iCurArg = pSpot; 
    };

    inline int currentPosition(void) { return iCurArg; };

    string commandLine(const string& pCmdName, unsigned int pMaxLength = 0, 
            int pStartAt = 0) const;

    inline Tcl_Obj*const* getArgVec() 
    { 
        return iOriginalArgVec;
    };

protected:
    int iNumArgs, iCurArg;
    Tcl_Obj *const *iArgVec;
    Tcl_Obj *const *iOriginalArgVec;
    bool iExplicitDelete;
    bool iOk;
};


class TACArgData 
{
public:
    inline TACArgData(TACArgs *pArgs)
    { 
        parseArgs(pArgs); 
    };
    inline ~TACArgData() {};	

    void parseArgs(TACArgs *pArgs);
    bool argPresent(string pName);
    bool getIntValue(string pName, int &pInt);
    bool getStrValue(string pName, string &pStr);
    bool getDoubleValue(string pName, double &pDouble);

protected:
    
    HashStr<string> argHashTable;
};


#endif
