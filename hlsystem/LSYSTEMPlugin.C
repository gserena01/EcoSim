


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
#include <OP/OP_Director.h>
#include <OP/OP_OperatorTable.h>


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
		new OP_Operator("CusLsystem",			// Internal name
			"MyLsystem",			// UI name
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
//Example to declare a variable for angle you can do like this :
//static PRM_Name		angleName("angle", "Angle");
static PRM_Name angleName("angle", "Angle");
static PRM_Name stepName("step", "Step");
static PRM_Name iterName("iter", "Iter");
static PRM_Name grammarName("grammar", "Grammar");

static PRM_Name startXName("startX", "StartX");
static PRM_Name startYName("startY", "StartY");
static PRM_Name startZName("startZ", "StartZ");

static PRM_Name widthName("width", "Width");

static PRM_Name terrainName("terrain", "Terrain");

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//				     ^^^^^^^^    ^^^^^^^^^^^^^^^
//				     internal    descriptive version

// PUT YOUR CODE HERE
// You need to setup the initial/default values for your parameters here
// For example : If you are declaring the inital value for the angle parameter
// static PRM_Default angleDefault(30.0);	
static PRM_Default angleDefault(30.0);
static PRM_Default stepDefault(1.0);
static PRM_Default iterDefault(4);
static PRM_Default grammarDefault(0, "F\nF->F[+F]F[-F]");

static PRM_Default startXDefault(0.0);
static PRM_Default startYDefault(0.0);
static PRM_Default startZDefault(0.0);

static PRM_Default widthDefault(0.5);

static PRM_Default terrainDefault(0, "");

////////////////////////////////////////////////////////////////////////////////////////

PRM_Template
SOP_Lsystem::myTemplateList[] = {
	// PUT YOUR CODE HERE
	// You now need to fill this template with your parameter name and their default value
	// EXAMPLE : For the angle parameter this is how you should add into the template
	// PRM_Template(PRM_FLT,	PRM_Template::PRM_EXPORT_MIN, 1, &angleName, &angleDefault, 0),
	// Similarly add all the other parameters in the template format here
		PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &angleName, &angleDefault, 0),
		PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &stepName, &stepDefault, 0),
		PRM_Template(PRM_INT, PRM_Template::PRM_EXPORT_MIN, 1, &iterName, &iterDefault, 0),
		PRM_Template(PRM_STRING, PRM_Template::PRM_EXPORT_MIN, 1, &grammarName, &grammarDefault, 0),
		PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &startXName, &startXDefault, 0),
		PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &startYName, &startYDefault, 0),
		PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &startZName, &startZDefault, 0),
		PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &widthName, &widthDefault, 0),

		PRM_Template(PRM_PICFILE, PRM_Template::PRM_EXPORT_MIN, 1, &terrainName, &terrainDefault, 0),

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
	{ "NPT",	VAR_NPT, 0 },   // from text string to integer token
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

	// PUT YOUR CODE HERE
	// Decare the necessary variables and get always keep getting the current value in the node
	// For example to always get the current angle thats set in the node ,you need to :
	//    float angle;
	//    angle = ANGLE(now)       
	//    NOTE : ANGLE is a function that you need to use and it is declared in the header file to update your values instantly while cooking 

	float           t = context.getTime();
	OP_Network* parent;
	OP_Node* node;
	OP_Node* input;
	// create node
	parent = (OP_Network*)OPgetDirector()->findNode("/obj/geo1");
	node = parent->createNode("file", "file1");
	if (!node)
		return error();
	// run creation script
	if (!node->runCreateScript())
		return error();
	// set parameters
	node->setFloat("t", 0, t, 1.0f);    // set tx to 1.0
	node->setFloat("t", 1, t, 2.0f);    // set ty to 2.0
	node->setFloat("t", 2, t, 0.0f);    // set tz to 0.0
	// connect the node
	input = parent->findNode("null1");  // find /obj/null1 as relative path
	if (input)
	{
		node->setInput(0, input);       // set first input to /obj/null1
	}
	// now that done we're done connecting it, position it relative to its
	// inputs
	node->moveToGoodPosition();


	LSystem myplant;
	float angle;
	angle = ANGLE(now);
	float step;
	step = STEP(now);
	float iter;
	iter = ITER(now);
	std::string grammar = GRAMMAR(now).toStdString();
	float startX;
	startX = STARTX(now);
	float startY;
	startY = STARTY(now);
	float startZ;
	startZ = STARTZ(now);
	float width;
	width = WIDTH(now);

	std::string terrainFile = TERRAIN(now).toStdString();

	///////////////////////////////////////////////////////////////////////////

	//PUT YOUR CODE HERE
	// Next you need to call your Lystem cpp functions 
	// Below is an example , you need to call the same functions based on the variables you declare
	myplant.loadProgramFromString(grammar);
	myplant.setDefaultAngle(angle);
	myplant.setDefaultStep(step);

	///////////////////////////////////////////////////////////////////////////////

	// PUT YOUR CODE HERE
	// You the need call the below function for all the genrations ,so that the end points points will be
	// stored in the branches vector , you need to declare them first
	std::vector<LSystem::Branch> branches;

	for (int i = 0; i < iter; i++)
	{
		myplant.process(i, branches);
	}

	///////////////////////////////////////////////////////////////////////////////////

	// Now that you have all the branches ,which is the start and end point of each point ,its time to render 
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
			// You need to build your cylinders inside Houdini from here
			// TIPS:
			// Use GU_PrimPoly poly = GU_PrimPoly::build(see what values it can take)
			// Also use GA_Offset ptoff = poly->getPointOffset()
			// and gdp->setPos3(ptoff,YOUR_POSITION_VECTOR) to build geometry.

			vec3 quadOffset = vec3(width, 0.0, 0.0);
			vec3 treeOffset = vec3(startX, startZ, startY);
			for (int j = 0; j < branches.size(); j++) {
				LSystem::Branch currBranch = branches[j];
				vec3 start = currBranch.first + treeOffset;
				vec3 end = currBranch.second + treeOffset;
				GU_PrimPoly* rect = GU_PrimPoly::build(gdp, 4);

				// We flip the y and z axis
				// p0
				UT_Vector3D p0 = UT_Vector3D(start[0], start[2], start[1]);
				GA_Offset ptoff0 = rect->getPointOffset(0);
				gdp->setPos3(ptoff0, p0);

				// p1
				UT_Vector3D p1 = UT_Vector3D(start[0], start[2], start[1])
					+ UT_Vector3D(quadOffset[0], quadOffset[2], quadOffset[1]);
				GA_Offset ptoff1 = rect->getPointOffset(1);
				gdp->setPos3(ptoff1, p1);

				// p2
				UT_Vector3D p2 = UT_Vector3D(end[0], end[2], end[1])
					+ UT_Vector3D(quadOffset[0], quadOffset[2], quadOffset[1]);
				GA_Offset ptoff2 = rect->getPointOffset(2);
				gdp->setPos3(ptoff2, p2);

				// p3
				UT_Vector3D p3 = UT_Vector3D(end[0], end[2], end[1]);
				GA_Offset ptoff3 = rect->getPointOffset(3);
				gdp->setPos3(ptoff3, p3);
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

