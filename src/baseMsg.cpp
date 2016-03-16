//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <baseMsg.h>
//
// The message definitions for base module
//
// Author: Yongqiang Lu
// History: 2009/10/29 created by LU
//*****************************************************************************
//*****************************************************************************
#include "message.h"

MsgWarning sc1("SC-1","No SC_ROOT environment variable or -root is set, some features of this software might be infeasible. Please set either of them to the root installment directory");

MsgNote sc8("SC-8","While running %s,");
MsgNote sc9("SC-9","Running %s ends. %s");

MsgWarning db1("DB-1","Tri-state driver found on net %s");
MsgWarning db2("DB-2","No explicit driver found on net %s");
MsgWarning db3("DB-3","No timing information found on library %s");
MsgWarning db4("DB-4","No IO delay constrain (e.g. SDC file) information found on library %s");
MsgWarning db5("DB-5","No location information (pins or PadCells) found on term %s");
// specially for report database commands
MsgNote db10("DB-10", "%s");

MsgError db50("DB-50","Exception: %s");
MsgError db51("DB-51","Database read/write permission error: %s");
MsgError db52("DB-52","There are some unbound cells in this design, the physical layout might not be implemented correctly.");

