//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <tacArgs.cpp>
//
// The auto command args class
//
// Author: YAN Haixia
// History: 2009/6/16 created by YAN Haixia
//*****************************************************************************
//*****************************************************************************

#include "tacArgs.h"

TACArgs::TACArgs(int pArgc, Tcl_Obj *const pObjv[]) 
    : iNumArgs(pArgc-1),
      iCurArg(0),
      iArgVec(pObjv+1),
      iOriginalArgVec(pObjv) 
{
    iExplicitDelete = false;
    iOk = true; 
}

TACArgs::TACArgs(string* pArgList, int pNumArgs)
{
    Tcl_Obj** lObjVec;

    lObjVec = (Tcl_Obj**) Tcl_Alloc(sizeof(Tcl_Obj*) * pNumArgs);
    for (int lIdx = 0;  lIdx < pNumArgs;  lIdx++) {
            lObjVec[lIdx] = 
                Tcl_NewStringObj((char*)pArgList[lIdx].c_str(),
                        pArgList[lIdx].length());
    }
    iCurArg = 0;
    iArgVec = lObjVec;
    iOriginalArgVec = iArgVec;
    iNumArgs = pNumArgs;
    iExplicitDelete = true;
    iOk = true;
}

TACArgs::~TACArgs()
{
    if (iExplicitDelete) {

        int lIdx;

        for (lIdx = 0;  lIdx < iNumArgs;  lIdx++) {
            Tcl_DecrRefCount(iArgVec[lIdx]);
        }
        Tcl_Free((char*) iArgVec);
    }
}


bool 
TACArgs::nextArgAsInt(int& pResult)
{
    if ((iCurArg < iNumArgs) && Tcl_GetIntFromObj((Tcl_Interp*) 0,
                iArgVec[iCurArg], &pResult) == TCL_OK) {
            iCurArg++;
            return true;
    } else {
            return false;
    }
}

bool 
TACArgs::nextArgAsDbl(double& pResult)
{
    if ((iCurArg < iNumArgs) && Tcl_GetDoubleFromObj((Tcl_Interp*) 0, iArgVec[iCurArg], &pResult) == TCL_OK) {
            iCurArg++;
            return true;
    } else {
            return false;
    }
}

bool 
TACArgs::nextArgAsStr(string& pResult)
{
    if (iCurArg < iNumArgs) {
        pResult=	Tcl_GetStringFromObj(iArgVec[iCurArg], (int *)0);
        iCurArg++;
        return true;
    } else {
        return false;
    }
}

bool 
TACArgs::nextArgAsStrPara(string& pResult)
{
    if (iCurArg<iNumArgs) {
        pResult = Tcl_GetStringFromObj(iArgVec[iCurArg], (int*)0); 
        if(pResult[0]!='-')
        {
            iCurArg++;
            return true;
        }
        else
        {return false;}		
    }
    else{
        return false;
    }
}

string
TACArgs::commandLine(const string& pCmdName, unsigned int pMaxLength, 
        int pStartAt) const
{
    string lResult = pCmdName;
    int lSpot;

    for (lSpot = pStartAt;  lSpot < iNumArgs;  lSpot++) {
        string lArgVal = Tcl_GetStringFromObj(iArgVec[lSpot], (int*)0);
        if ((pMaxLength > 0) &&
                (lResult.length() + 1 + lArgVal.length()) > pMaxLength) {
            lResult += " ...";
            break;
        }
        lResult += " ";
        //l_result.append(temp);
        lResult.append(lArgVal);
    }
    return lResult;
}

void 
TACArgData::parseArgs(TACArgs *pArgs)
{
    string perArgs;
    pArgs->reset(0);
    while(pArgs->nextArgAsStr(perArgs))
    {
        //Params * argData;

        if(perArgs[0]=='-')
        {
            string temp;
            for(unsigned int i=1;i<perArgs.size();i++)
            {
                temp += perArgs[i];
            }
            string pString;

            if(pArgs->nextArgAsStrPara(pString))
            {
                argHashTable.add(temp, pString);			  
            }  		   
            else
            {
                // cout<<" add element "<<temp<<endl;
                argHashTable.add(temp, "");
            }
        }		
    }
}

bool 
TACArgData::argPresent(string pName)
{
    if (argHashTable.exists(pName)) { 
        return true;
    }
    return false;
}

bool 
TACArgData::getIntValue(string pName, int &pInt)
{
    string strVal;
    if (argHashTable.get(pName,strVal)) {
        if (strVal == "") {
            return false;
        }
        else
        { 
            pInt = atoi(strVal.c_str());
            return true;
        }
    }
    return false;
}
bool 
TACArgData::getStrValue(string pName, string &pStr)
{
    if(argHashTable.get(pName,pStr))
    {
        if (pStr == "") {
            return false;
        }
        else {
            return true;
        }
    }
    return false;
}

bool 
TACArgData::getDoubleValue(string pName, double &pDouble)
{
    string strVal;
    if (argHashTable.get(pName, strVal)) {
        if (strVal == "") {
            return false;
        }
        else {
            pDouble = atof(strVal.c_str());
            return true;
        }
    }
    return false;
}
