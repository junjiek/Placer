//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <tacCommand.cpp>
//
// The auto command main class
//
// Author: YAN Haixia
// History: 2009/6/16 created by YAN Haixia
//*****************************************************************************
//*****************************************************************************

#include "tacCommand.h"

extern "C" int 
TACFaceInit(Tcl_Interp *interp)
{
    // register the static commands here.
    TACInterp::registerStaticCommands(TACInterp::getMainInterp(interp));

    return 1;
}


// One function is called for all commands under the C++ interface.
// It is in charge of firing the TclCommand::go() method.
extern "C" int callTACCommand(
	ClientData pClientData, 
	Tcl_Interp *pInterp,
	int pObjc,
	Tcl_Obj *const pObjv[])
{
    TACAutoCommand *lCmd = (TACAutoCommand*) pClientData;
    //char *lCmdFormat = (char *) lCmd->getFormat().c_str();
    //char *lCmdName = (char *) lCmd->getName().c_str();
    //cout<<"command name: "<<lCmdName<<endl;
    //cout<<"command parameters: "<<lCmdFormat<<endl;
    TACArgs lArgs(pObjc, pObjv);
    TACArgData argsData(&lArgs); 
    return lCmd->go(&argsData);
}

void 
TACInterp::registerStaticCommands(TACInterp *pInterp)
{
    TACStaticGenerator lAllCmds;
    TACCommand* lCmd;

    while ((lCmd = lAllCmds.next())) {
        //cout<<"tcl register command "<< lCmd->getName().c_str()<<endl;
        pInterp->registerCmd(lCmd); 
    }
}

    //unregister all static commands
void TACInterp::unregisterStaticCommands()
{
    TACStaticGenerator lAllCmds;
    TACCommand* lCmd;

    while ((lCmd = lAllCmds.next())) {
        //cout<<"tcl register command "<< lCmd->getName().c_str()<<endl;
        unregisterCmd(lCmd);
    }
}
