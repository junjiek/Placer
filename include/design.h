//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <design.h>
//
// The design/flow/database manapulation class
//
// Author: Lu Yongqiang
// History: 2009/5/27 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined (_SC_DESIGN_H_) 
#define _SC_DESIGN_H_
#include "base.h"
#include "staticData.h"
#include "extension.h"
#include "rowOA.h"
#include "term.h"
#include "instOA.h"
#include "netOA.h"
//#include "../src/oa/oa/src/design/oaDesignPvt.h"

// All the operations are on current design by default.
// All member/data are static since only one working Deisgn would be valid at 
// the same time.
class Design 
{
public:
    enum DesignStatus {
        NONE = 0,
        LOGICAL,      // only logical, not technogloy mapped
        TECH_MAPPED,  // technolgoy mapped
        FLOORPLANED,  // floorplan finished
        GLOBAL_PLACED,// global place finished
        DETAIL_PLACED,// detail place finished
        GLOBAL_ROUTED,// global routing finished
        TRACK_ROUTED, // track routing finished
        DETAIL_ROUTED,// detail routing finished
        CTSED,        // CTS finished
        PGED          // PG finished
    };

    enum RowPGLoc {
        PUP_GDOWN = 0,  // p is on the top of the row (or left if row is 
                        // vertical) and g is on the bottom of the row
                        // This would be the default
        PDOWN_GUP = 1   // p is on the bottom and g is on the top
    };

    class DesignStatics : public StaticData 
    {
    public:
        DesignStatics() {};
    private:
        void clear();
        void setup();
    };
protected:
    enum DataSheetHeader {
        DESIGN_STATUS = 0,
        NUM_OF_DATA = 1
    };

public:
    // open the top design
    static oaDesign* open(const string &library, const string &cell,
              const string &view, const char mode = 'r');

    // open a non-top design
    static oaDesign* openNormal(
            const oaScalarName &libName,
            const oaScalarName &cellName,
            const oaScalarName &viewName,
            const char mode = 'r');

    static inline oaDesign *getCurrent()
    {
        return desCurrent;
    };

public:
    // Once this function is invoked, all hierarchy is flattened and uniquified,
    // any physical synthesis/optimization and extraction annotation methods
    // can be freely used
    static inline void uniquify(oaDesign *design)
    {
        // descend hierarchy and uniquify if necessary
        OccUniqTraverser utra(design->getTopOccurrence());
    };

    // report design data
    static void reportDesignData( oaDesign *design,
            const bool checkNet, const bool completeCheck,
            const bool checkInst);

    static void reportTerms(oaDesign *design);

    // for design files
    // get SDC file name
    static void getSDCFile(oaDesign *design, String &filePath);
    // set SDC file path by user
    static void setSDCFile(oaDesign *design, const char *filePath);
    // for Design flows
    static DesignStatus getDesignStatus(oaBlock *block);
    static void setDesignStatus(oaBlock *block, DesignStatus status);

    // for design instance ID assemble
    // shift: the id shift for each inst
    // e.g. instArray[2]'s id is 2 + shift
    static void assembleInstID(Array<oaInst*> instArray, UInt4 shift = 0);
    // This inst ID factory is all-in-one object, please keep the data 
    // consistency at your own risk especially in crossing deisgn flows
    static inline UInt4 getID(oaInst *inst)
    {
        return (UInt4) dsvInstIdDef->get(inst);
    };
    static inline void setID(oaInst *inst, UInt4 id)
    {
        dsvInstIdDef->set(inst, id);
    };

    // get any view type;
    static oaCellView* getCellView(oaCell *cell);
    // get specified view type
    static oaCellView* getCellView(oaCell *cell, 
            oaReservedViewTypeEnum);
    static oaScalarName getCellViewName(oaCellView *view);

    // get/set cell row pg location
    static inline Design::RowPGLoc getRowPGLoc(oaRow *row)
    {
        return (RowPGLoc) dsvRowPGDef->get(row);
    };
    static inline void setRowPGLoc(oaRow *row, RowPGLoc loc)
    {
        dsvRowPGDef->set(row, loc);
    };

    static inline double getTotalInstArea(const oaBlock *block)
    {
        double area = 0.0;
        oaIter<oaInst> it(block->getInsts());
        while (oaInst *inst = it.getNext()) {
            area += InstOA::getArea(inst);
        }
        return area;
    };


    //=======================hjy20131026==================

    static void saveAs(const string &libNameIn,const string &cellNameIn,const string &viewNameIn, const string &newNameIn);
    //=====================================================


public:
    // the current opened design object
    static oaDesign *desCurrent;

protected:
    static const char* DESIGN_DATA_TABLE;
    static const char* DESIGN_INST_ID;
    static const char* DESIGN_ROW_PG;
    static const char* DESIGN_SDC_FILE;
    
    // database data sheet
    static DBExtension<oaBlock> *dsvDataSheet;
    // oaInst ID factory
    static oaIntAppDef<oaInst> *dsvInstIdDef;
    // design cell row pg location
    static oaIntAppDef<oaRow> *dsvRowPGDef;
};

#endif
