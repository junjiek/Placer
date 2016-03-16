#include "block.h"
#include "simPlPlace.h"
#include "dplace.h"
#include "ldplace.h"
#include "fstream"
#include "sys/time.h"

#include "base.h"
#include "tcl.h"
#include "baseMsg.h"
#include "sessionManager.h"
#include "tacCommand.h"

void usage() {
	cout << "[LEF/DEF]" << endl;
	cout
			<< "  Usage : ./placer -tech_lef <tech.lef> -cells_lef <cells.lef> -floorplan_def <bench.def> -verilog <bench.v> -output <output.def>"
			<< endl;
	cout
			<< "  An example: ./placer -tech_lef tech.lef -cells_lef cells.lef -floorplan_def cordic.def -verilog design.v -output cordic.sol.def"
			<< endl << endl;

	cout << "[BOOKSHELF]" << endl;
	cout << "  Usage : ./placer <bench.aux> <solution.pl>" << endl;
	cout << "  An example: ./placer bench/bench.aux sol.pl" << endl << endl;

	cout << "[OA]" << endl;
	cout << "  Usage : ./placer -oa " << endl;
	cout << "  An example: ./placer -oa " << endl << endl;

}

enum format {
	LEFDEF = 1, BOOKSHELF = 2, OA = 3, OTHER = 4
};

//*******************************************************
//                   for OA input
//*******************************************************
static string initTclFile = "";
const char* RELEASE_TARGET_NAME = "OAPlacer";
static Tcl_Interp *scgTclInterp = NULL;

// static manager array body
// For whole wool flow management
SessionManager *scgSessionMan = NULL;
string scgRootDir = "";

extern "C" int TACFaceInit(Tcl_Interp *interp);
extern "C" int Oa_Init(Tcl_Interp *interp);

// init proc for tcl init
static int tclInitProc(Tcl_Interp *interp) {
	// do some initlization work here for tk
	// tcl init
	if (TCL_ERROR == Tcl_Init(interp)) {
		Tcl_DeleteInterp(interp);
		return TCL_ERROR;
	}

	// setup oa tcl commands
	Oa_Init(interp);

	// setup sc own commands
	TACFaceInit(interp);

	return TCL_OK;
}

//**************************************************************

int main(int argc, char**argv) {
	long format = 0;

	if (argc == 11) {
		format = LEFDEF; //lefdef benchmark
	} else if (argc == 3) {
		format = BOOKSHELF; //bookshelf benchmark
	} else if (argc == 2) {
		format = OA; //oa benchmark
	} else {
		usage();
		return -1;
	}

	struct timeval tv1, tv2, tv3;
	Block block;
	char* defFile;
	char* techFile;
	char* cellFile;
	char* solFile;
	string design_full_name;
	size_t sz;
	string design_aux;
	string sol_file;
	string plFileName;
	// (1) parse design and construct block
	if (format == LEFDEF) {
		for (int i = 1; i < argc; i += 2) {
			if (!strcmp(argv[i], "-floorplan_def")) {
				defFile = argv[i + 1];
			} else if (!strcmp(argv[i], "-output")) {
				solFile = argv[i + 1];
			} else if (!strcmp(argv[i], "-tech_lef")) {
				techFile = argv[i + 1];
			} else if (!strcmp(argv[i], "-cells_lef")) {
				cellFile = argv[i + 1];
			} else if (!strcmp(argv[i], "-verilog")) {
				//TODO verilog
			} else {
				usage();
				return -1;
			}
		}
		cout << endl;
		design_full_name = string(defFile);
		sz = design_full_name.size() - 4;
		design_full_name.resize(sz);
		//cout << "design full name: " << design_full_name << endl;
		//cout << "design legal name: "<< design_legalfile << endl;
		//cout<<"block targetUtil = "<<block.getTargetUtil()<<endl;
		//cout<<"block max displacement = "<<block.getMaxDisplacement()<<endl<<endl;
		gettimeofday(&tv1, 0);

		block.parseLefDef(techFile, cellFile, defFile);
		block.setMaxDisplacement(INT_MAX);
		block.setTargetUtil(1.0);
		block.defFile = defFile;
		sol_file = string(solFile);
	} else if (format == BOOKSHELF) {
		design_aux = string(argv[1]);
		design_full_name = string(design_aux);
		sz = design_full_name.size() - 4;
		design_full_name.resize(sz);
		cout << "design full name: " << design_full_name << endl;

		sol_file = string(argv[2]);
		plFileName = design_full_name + ".pl";
		block.parseBookShelf(design_full_name);
	} else if (format == OA) {
		if (strcmp(argv[1], "-oa")) {
			usage();
			return -1;
		}
		scgSessionMan = new SessionManager;

		oaDesignInit(oacAPIMajorRevNumber, oacAPIMinorRevNumber, 4);
		oaRegionQuery::init("oaRQXYTree");

		if (scgRootDir == "") {
			char *rootEnv = getenv("ROOT");
			if (rootEnv) {
				scgRootDir = rootEnv;
			} else {
				char tclPath[512];
				memset(tclPath, 0, sizeof(char) * 512);
				getcwd(tclPath, 512);
				scgRootDir = tclPath;
			}
		}

		string newEnv = "ROOT=";
		newEnv += scgRootDir;
		putenv(const_cast<char*> (newEnv.c_str()));
		Tcl_Main(argc, argv, tclInitProc);

		delete scgSessionMan;

		if (scgTclInterp) {
			Tcl_DeleteInterp(scgTclInterp);
		}

		return 1;
	} else {
		usage();
		return -1;
	}

	// (2) initial placement
	gettimeofday(&tv1, 0);
	SimPlPlace placement(&block);

	placement.setVisible(false);

	placement.iPlace(1); // 1-random or 2-hybrid

//	placement.guiFile("iplace.gnu");

	//DEBUG
	placement.setClusternum();

	// (3) global placement
	placement.gPlace();
//	placement.guiFile("gplace.gnu");

	//DEBUG
	cout << "[INFO:] The narrow Clusters num is : " << placement.getClusternum() << endl;

//	gettimeofday(&tv_gp2, 0);
//	cout << endl << "global placement time : " << double(
//			tv_gp2.tv_sec - tv_gp1.tv_sec + double(
//					tv_gp2.tv_usec - tv_gp1.tv_usec) / CLOCKS_PER_SEC) << endl
//			<< endl;

	//placement.congestionEstimate("");

	//	gettimeofday(&tv2, 0);
	//	bool rippleDP = true;

//	 (4) legalization
//	fdplDetailPlace legalization(&block);
//	legalization.setVisible_LG(false);
//	legalization.dplace();
//	placement.congestionEstimate();
//	placement.guiFile("endplace.gnu");


	// (5) detailed placement
//	cout << endl << "************ detailed placement ***********" << endl;
//	Ldplace dplace(&block);
//	dplace.setVisible_DP(false);
//	dplace.run();
//	legalization.refineOrient();
//	placement.guiFile("detailedplace.gnu");

	// (6) save the placement result to file
	if (format == LEFDEF) {
		block.saveDef(solFile, defFile);
	} else if (format == BOOKSHELF) {
		//block.savePl(sol_file.c_str(), plFileName.c_str());
		block.savePlAccurate(sol_file.c_str(), plFileName.c_str());
	} else if (format == OA) {
		//TODO
	} else {
		cout << "invalid output!!!" << endl;
	}

	gettimeofday(&tv3, 0);
	//	cout << endl << "Detailed placement time : " << double(
	//			tv3.tv_sec - tv2.tv_sec + double(tv3.tv_usec - tv2.tv_usec)
	//					/ CLOCKS_PER_SEC) << endl << endl;

	cout << endl << "************ Placement solution is saved in file "
			<< sol_file << ". ************" << endl;
	cout << endl << "Total time : " << double(
			tv3.tv_sec - tv1.tv_sec + double(tv3.tv_usec - tv1.tv_usec)
					/ CLOCKS_PER_SEC) << endl << endl;
	return 0;
}
