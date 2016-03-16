//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <tacCommand.h>
//
// The auto command main class
//
// Author: YAN Haixia
// History: 2009/6/16 created by YAN Haixia
//*****************************************************************************
//*****************************************************************************

#if !defined(_TAC_COMMAND_H_)
#define _TAC_COMMAND_H_

#include <string>
#include "base.h"
#include "tcl.h"
#include "tacArgs.h"
#include "tacGenerator.h"


using namespace std;

class TACInterp;        
class TACCommand;      
class TACStaticCommand;
class TACAutoCommand; 
class TACHandler;
class TACStaticRegister;
class TACArgs;         

class TACArgData;

class TACStaticGenerator;

enum TclParseCode { TCL_PARSE_ERROR=0, TCL_PARSE_HELP, TCL_PARSE_OK};
enum TclTrapKind { TCL_STD_OUT, TCL_STD_ERR };
enum TclEchoMode { TCL_ALL, TCL_MESSAGE, TCL_NONE };

typedef struct Tcl_Interp Tcl_Interp;
typedef struct Tcl_Obj Tcl_Obj;

extern "C" int callTACCommand(
	ClientData pClientData, 
	Tcl_Interp *pInterp,
	int pObjc,
	Tcl_Obj *const pObjv[]);


class TACCommand 
{
    friend class TACInterp;
public:
    inline TACCommand(const char *pCmdName)
        : iName(pCmdName) 
    { 
    };

    virtual ~TACCommand() { };

    virtual void setup() { };

    virtual inline string getName()
    {
	return iName;
    };

    virtual inline void setName(const string &pName)
    {
	iName = pName;
    };

    virtual bool isAutoCommand() { return false; }

protected:
    string iName;            // Name of command
  
};

class TACInterp 
{
public:
    TACInterp(Tcl_Interp *pInterp)
        : iInterp(pInterp)
    {
    };
    ~TACInterp()
    {
        unregisterStaticCommands();
    };

    Tcl_Interp *interpreter() { return iInterp; }


    inline void registerCmd(TACCommand *pCmd)
    {
        Tcl_CreateObjCommand(iInterp, 
                pCmd->getName().c_str(), 
                callTACCommand, (ClientData) pCmd, NULL);
    };


    inline void unregisterCmd(TACCommand *pCmd)
    {
        Tcl_DeleteCommand(iInterp, (char*) pCmd->getName().c_str());
    };

    // Register the static commands.  These cannot be unregistered.
    static void registerStaticCommands(TACInterp *pInterp);

    //unregister all static commands
    void unregisterStaticCommands();
    inline static TACInterp* getMainInterp(Tcl_Interp *interp)
    {
        static TACInterp *sMainInterp = new TACInterp(interp);
        return sMainInterp;
    };

public:

protected:
    Tcl_Interp *iInterp;

};


class TACAutoCommand : public TACCommand 
{
public:
    
    inline TACAutoCommand(const char* pName, const char* pFormat)
        : TACCommand(pName), iFormat(pFormat) 
    {
        TACStaticRegister::delayRegister(this);
    };

  
    virtual int start(TACArgData *pArgData) = 0;
    //-
    // Don't override this method.  It is used to automatically parse the
    
    inline int go(TACArgData *pArgData)
    {
        return start(pArgData);
    };

    string getFormat() { return iFormat; }

protected:
     const char* iFormat;

};

#endif
