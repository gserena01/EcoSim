
#include <UT/UT_DSOVersion.h>
//#include <RE/RE_EGLServer.h>

#include <UT/UT_Math.h>
#include <UT/UT_Interrupt.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>
#include <CH/CH_LocalVariable.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_SpareData.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <OP/OP_Director.h>

#include <limits.h>
#include "LSYSTEMPlugin.h"
using namespace HDK_Sample;

//
// Help is stored in a "wiki" style text file. 
//
// See the sample_install.sh file for an example.
//
// NOTE : Follow this tutorial if you have any problems setting up your visual studio 2008 for Houdini 
//  http://www.apileofgrains.nl/setting-up-the-hdk-for-houdini-12-with-visual-studio-2008/


///
/// newSopOperator is the hook that Houdini grabs from this dll
/// and invokes to register the SOP.  In this case we add ourselves
/// to the specified operator table.
///
void
newSopOperator(OP_OperatorTable* table)
{
	table->addOperator(
		new OP_Operator("CusEcoSim",			// Internal name
			"ecoSim",			// UI name
			SOP_Lsystem::myConstructor,	// How to build the SOP
			SOP_Lsystem::myTemplateList,	// My parameters
			0,				// Min # of sources
			0,				// Max # of sources
			SOP_Lsystem::myVariables,	// Local variables
			OP_FLAG_GENERATOR)		// Flag it as generator
	);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//PUT YOUR CODE HERE
//You need to declare your parameters here
static PRM_Name		soilName("soil", "Soil Water");
static PRM_Name		vaporName("vapor", "Vapor Water");
static PRM_Name		treeName("tree", "TreeMeshFile");
static PRM_Name		iterationName("iterations", "Iterations");

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//				     ^^^^^^^^    ^^^^^^^^^^^^^^^
//				     internal    descriptive version


// PUT YOUR CODE HERE
// You need to setup the initial/default values for your parameters here
static PRM_Default soilDefault(1.0);
static PRM_Default vaporDefault(1.0);
static PRM_Default treeDefault(0, "");
static PRM_Default iterationDefault(0);

static PRM_Range iterationRange(PRM_RANGE_UI,  0, 
								PRM_RANGE_UI, 20);


////////////////////////////////////////////////////////////////////////////////////////

PRM_Template
SOP_Lsystem::myTemplateList[] = {
	// PUT YOUR CODE HERE
	// You now need to fill this template with your parameter name and their default value
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &soilName, &soilDefault, 0),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &vaporName, &vaporDefault, 0),
	PRM_Template(PRM_STRING, 1, &treeName, &treeDefault),
	PRM_Template(PRM_INT, PRM_Template::PRM_EXPORT_MIN, 1, &iterationName, &iterationDefault, 0, &iterationRange),

	/////////////////////////////////////////////////////////////////////////////////////////////

		PRM_Template()
};


// Here's how we define local variables for the SOP.
enum {
	VAR_PT,		// Point number of the star
	VAR_NPT		// Number of points in the star
};

CH_LocalVariable
SOP_Lsystem::myVariables[] = {
	{ "PT",	VAR_PT, 0 },		// The table provides a mapping
	{ "NPT",	VAR_NPT, 0 },		// from text string to integer token
	{ 0, 0, 0 },
};

bool
SOP_Lsystem::evalVariableValue(fpreal& val, int index, int thread)
{
	// myCurrPoint will be negative when we're not cooking so only try to
	// handle the local variables when we have a valid myCurrPoint index.
	if (myCurrPoint >= 0)
	{
		// Note that "gdp" may be null here, so we do the safe thing
		// and cache values we are interested in.
		switch (index)
		{
		case VAR_PT:
			val = (fpreal)myCurrPoint;
			return true;
		case VAR_NPT:
			val = (fpreal)myTotalPoints;
			return true;
		default:
			/* do nothing */;
		}
	}
	// Not one of our variables, must delegate to the base class.
	return SOP_Node::evalVariableValue(val, index, thread);
}

OP_Node*
SOP_Lsystem::myConstructor(OP_Network* net, const char* name, OP_Operator* op)
{
	return new SOP_Lsystem(net, name, op);
}

SOP_Lsystem::SOP_Lsystem(OP_Network* net, const char* name, OP_Operator* op)
	: SOP_Node(net, name, op)
{
	myCurrPoint = -1;	// To prevent garbage values from being returned
}

SOP_Lsystem::~SOP_Lsystem() {}

unsigned
SOP_Lsystem::disableParms()
{
	return 0;
}

OP_ERROR
SOP_Lsystem::cookMySop(OP_Context& context)
{
	fpreal		 now = context.getTime();

	std::cout << "Hello world" << std::endl;

	// PUT YOUR CODE HERE
	// Decare the necessary variables and get always keep getting the current value in the node
	// For example to always get the current angle thats set in the node ,you need to :
	//    float angle;
	//    angle = ANGLE(now)       
	//    NOTE : ANGLE is a function that you need to use and it is declared in the header file to update your values instantly while cooking 
	float soil = SOIL(now);
	float vapor = VAPOR(now);
	int itr = ITERATIONS(now);
	UT_String treeMeshFile;
	TREEFILE(now, treeMeshFile);
	LSystem myplant;

	EcoSim eco;

	///////////////////////////////////////////////////////////////////////////

	//PUT YOUR CODE HERE
	// Next you need to call your Lystem cpp functions 
	// Below is an example , you need to call the same functions based on the variables you declare
	// myplant.loadProgramFromString("F\nF->F[+F]F[-F]";  
	// myplant.setDefaultAngle(30.0f);
	// myplant.setDefaultStep(1.0f);

	eco.setVapor(vapor);
	eco.setSoilWater(soil);
	eco.setTrees();


	///////////////////////////////////////////////////////////////////////////////

	// PUT YOUR CODE HERE
	// You the need call the below function for all the generations, so that the end points points will be
	// stored in the branches vector, you need to declare them first

	for (int i = 0; i < itr; ++i) {
		eco.cycle();
	}

	///////////////////////////////////////////////////////////////////////////////////

	// Now that you have all the branches, which is the start and end point of each point, its time to render 
	// these branches into Houdini 

	// PUT YOUR CODE HERE 
	// Declare all the necessary variables for drawing cylinders for each branch 
	float		 rad, tx, ty, tz;
	int			 divisions, plane;
	int			 xcoord = 0, ycoord = 1, zcoord = 2;
	float		 tmp;
	UT_Vector4		 pos;
	GU_PrimPoly* poly;
	int			 i;
	UT_Interrupt* boss;


	// Since we don't have inputs, we don't need to lock them.

	divisions = 5;	// We need twice our divisions of points
	myTotalPoints = divisions;		// Set the NPT local variable value
	myCurrPoint = 0;			// Initialize the PT local variable

	// Check to see that there hasn't been a critical error in cooking the SOP.
	if (error() < UT_ERROR_ABORT)
	{
		boss = UTgetInterrupt();
		if (divisions < 4)
		{
			// With the range restriction we have on the divisions, this
			//	is actually impossible, but it shows how to add an error
			//	message or warning to the SOP.
			addWarning(SOP_MESSAGE, "Invalid divisions");
			divisions = 4;
		}
		gdp->clearAndDestroy();

		// Start the interrupt server
		if (boss->opStart("Building LSYSTEM"))
		{
			// PUT YOUR CODE HERE
			// Build a polygon

			// Loop through trees
			for (int i = 0; i < eco.trees.size(); ++i) {

				// Create geometry node
				OP_Network* geoParent;
				OP_Network* fileParent;
				OP_Node* geoNode;
				OP_Node* fileNode;

				// Call Create Node
				geoParent = (OP_Network*)OPgetDirector()->findNode("/obj");
				geoNode = geoParent->createNode("geo", "geo1");
				fileParent = (OP_Network*)OPgetDirector()->findNode("/obj/geo1");
				fileNode = fileParent->createNode("file", "file1");
				int fileLoaded = fileNode->setParameterOrProperty("file", 0, 0, "C:/Users/Kyra/Documents/_PENN_SP2023/CIS6600/EcoSim/objs/tree_mature.obj", CH_STRING_LITERAL);

				Tree t = eco.trees.at(i);

				vec3 p = t.position;
				float h = eco.TreeMass[t.growthStage];

				//UT_Vector3 start;
				//start(xcoord) = p[0];
				//start(ycoord) = p[2];
				//start(zcoord) = p[1];

				//UT_Vector3 end;
				//end(xcoord) = p[0];
				//end(ycoord) = p[2] + h;
				//end(zcoord) = p[1];

				//poly = GU_PrimPoly::build(gdp, 2, 0, 1);

				//GA_Offset ptoff = poly->getPointOffset(0);
				//gdp->setPos3(ptoff, start);

				//ptoff = poly->getPointOffset(1);
				//gdp->setPos3(ptoff, end);

			}

			////////////////////////////////////////////////////////////////////////////////////////////

			// Highlight the star which we have just generated.  This routine
			// call clears any currently highlighted geometry, and then it
			// highlights every primitive for this SOP. 
			select(GU_SPrimitive);
		}

		// Tell the interrupt server that we've completed. Must do this
		// regardless of what opStart() returns.
		boss->opEnd();
	}

	myCurrPoint = -1;
	return error();
}

