#include "base.h"
#include "design.h"
#include "tacCommand.h"
#include "commandMonitor.h"
#include "block.h"
#include "simPlPlace.h"
#include "dplace.h"
#include "ldplace.h"

using namespace std;
class TacDetailPlace : public TACAutoCommand
{
  public:
	TacDetailPlace(const char* p_name,const char* p_format)
	: TACAutoCommand(p_name,p_format){	};

	int start(TACArgData *pArgData)
	{

	  string lib,cell,view;
	  oaDesign *design=NULL;

	  if(pArgData->argPresent("lib"))
	  {
		pArgData->getStrValue("lib",lib);
		pArgData->getStrValue("cell",cell);
	    pArgData->getStrValue("view",view);

	    cout<<"opening design <"<<lib<<"> <"<<cell<<"> <"<<view<<">"<<endl;

		design = Design::open(lib.c_str(),cell.c_str(),view.c_str());	
	  }
	  else
	  {
		design=Design::getCurrent();
	  }
	  assert(design);
	  cout<<"begin to run cada089!"<<endl;
	  CommandMonitor mon("DetailPlace");

	  oaBlock *oaBlk= design->getTopBlock();

	  Block block;
	  block.setBloatSize(1);
	  block.setMaxDisplacement(INT_MAX);
	  block.setTargetUtil(0.8);
	  block.parseOA(oaBlk);

	  //string num("6");

//	  //GLOBAL PLACEMENT
//	  SimPlPlace placement(&block);
//	  placement.setVisible(false);
	  //TODO changed
//	  placement.iPlace(1);
//
//	  //block.saveOA(oaBlk,&block);
//	  //Design::saveAs(lib.c_str(),cell.c_str(),view.c_str(), (view + string("_initial_")).c_str());
//
//	  placement.gPlace();
//
//	  block.saveOA(oaBlk,&block);
//	  //Design::saveAs(lib.c_str(),cell.c_str(),view.c_str(), (view + string("_global")).c_str());
//	  Design::saveAs(lib.c_str(),cell.c_str(),view.c_str(), (view + string("_test1")).c_str());


	  //LEGALIZATION
	  fdplDetailPlace legalization(&block);
	  legalization.setVisible_LG(false);
	  legalization.dplace();

//	  block.saveOA(oaBlk,&block);
//	  Design::saveAs(lib.c_str(),cell.c_str(),view.c_str(), (view + string("_legalization")).c_str());

	  //DETAILED PLACEMENT
	  Ldplace dplace(&block);
	  dplace.setVisible_DP(false);
	  dplace.run();

	  legalization.dplace();

	  block.saveOA(oaBlk,&block);
	  Design::saveAs(lib.c_str(),cell.c_str(),view.c_str(), (view + string("_detail")).c_str());
	  //Design::saveAs(lib.c_str(),cell.c_str(),view.c_str(), (view + string("_test")).c_str());


	  //cout<<"Finishing the Detailed Place!"<<endl;
	  return TCL_OK;
	};
};

static TacDetailPlace tacvDetailPlace("DetailPlace", "-lib" "-cell" "-view");
