


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
			1,				// Max # of sources
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
static PRM_Name	plant1Name("plant1", "Plant1");

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
static PRM_Default plant1Default(0, "");

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
		PRM_Template(PRM_GEOFILE, PRM_Template::PRM_EXPORT_MIN, 1, &plant1Name, &plant1Default, 0),

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
	networkCreated = -1;
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
	float           t = context.getTime();
	OP_Node* heightfield_file_node;
	OP_Network* parent;
	parent = (OP_Network*)OPgetDirector()->findNode("/obj/geo1");

	// PUT YOUR CODE HERE
	// Decare the necessary variables and get always keep getting the current value in the node
	// For example to always get the current angle thats set in the node ,you need to :
	//    float angle;
	//    angle = ANGLE(now)       
	//    NOTE : ANGLE is a function that you need to use and it is declared in the header file to update your values instantly while cooking 
	if (networkCreated < 0) {
		OP_Node* input;
		// create node
		heightfield_file_node = parent->createNode("heightfield_file", "heightfield_file1");
		if (!heightfield_file_node)
			return error();
		// run creation script
		if (!heightfield_file_node->runCreateScript())
			return error();
		// set parameters
		heightfield_file_node->setString(UT_String("`chs(\"../CusLsystem1/terrain\")`"), CH_STRING_LITERAL, "filename", 0, t);
		// connect the node
		input = parent->findNode("null1");  // find /obj/geo1/null1 as relative path
		if (input)
		{
			heightfield_file_node->setInput(0, input);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		heightfield_file_node->moveToGoodPosition();

		OP_Node* heightfield_node;
		OP_Node* input2;
		// create node
		parent = (OP_Network*)OPgetDirector()->findNode("/obj/geo1");
		heightfield_node = parent->createNode("heightfield", "heightfield1");
		if (!heightfield_node)
			return error();
		// run creation script
		if (!heightfield_node->runCreateScript())
			return error();
		// connect the node
		input2 = parent->findNode("null1");  // find /obj/geo1/null1 as relative path
		if (input2)
		{
			heightfield_node->setInput(0, input2);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		heightfield_node->moveToGoodPosition();

		OP_Node* heightfield_noise_node;
		OP_Node* input3;
		// create node
		heightfield_noise_node = parent->createNode("heightfield_noise", "heightfield_noise1");
		if (!heightfield_noise_node)
			return error();
		// run creation script
		if (!heightfield_noise_node->runCreateScript())
			return error();
		// connect the node
		input3 = parent->findNode("heightfield1");  // find /obj/geo1/heightfield1 as relative path
		if (input3)
		{
			heightfield_noise_node->setInput(0, input3);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		heightfield_noise_node->moveToGoodPosition();

		OP_Node* switch_node;
		OP_Node* input4;
		OP_Node* input5;
		// create node
		switch_node = parent->createNode("switch", "switch1");
		if (!switch_node)
			return error();
		// run creation script
		if (!switch_node->runCreateScript())
			return error();
		// connect the node
		input4 = parent->findNode("heightfield_noise1");  // find /obj/geo1/heightfield1 as relative path
		input5 = parent->findNode("heightfield_file1");
		if (input4)
		{
			switch_node->setInput(0, input4);       // set first input to /obj/null1
		}
		if (input5) {
			switch_node->setInput(1, input5);
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		switch_node->moveToGoodPosition();

		OP_Node* convert_heightfield_node;
		OP_Node* input6;
		// create node
		convert_heightfield_node = parent->createNode("convertheightfield", "convertheightfield1");
		if (!convert_heightfield_node)
			return error();
		// run creation script
		if (!convert_heightfield_node->runCreateScript())
			return error();
		// connect the node
		input6 = parent->findNode("switch1");  // find /obj/geo1/heightfield1 as relative path
		if (input6)
		{
			convert_heightfield_node->setInput(0, input6);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		convert_heightfield_node->moveToGoodPosition();

		// Scatter node
		OP_Node* scatter_node;
		OP_Node* input7;
		// create node
		scatter_node = parent->createNode("scatter", "scatter1");
		if (!scatter_node)
			return error();
		// run creation script
		if (!scatter_node->runCreateScript())
			return error();
		// connect the node
		input7 = parent->findNode("convertheightfield1");  // find /obj/geo1/convertheightfield1 as relative path
		if (input7)
		{
			scatter_node->setInput(0, input7);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		scatter_node->moveToGoodPosition();

		// File Node
		OP_Node* file_node;
		OP_Node* input8;
		// create node
		file_node = parent->createNode("file", "file1");
		if (!file_node)
			return error();
		// run creation script
		if (!file_node->runCreateScript())
			return error();
		// set parameters
		file_node->setString(UT_String("`chs(\"../CusLsystem1/plant1\")`"), CH_STRING_LITERAL, "file", 0, t);
		// connect the node
		input8 = parent->findNode("null1");  // find /obj/geo1/null1 as relative path
		if (input8)
		{
			file_node->setInput(0, input8);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		file_node->moveToGoodPosition();

		// Pack Node
		OP_Node* pack_node;
		OP_Node* input9;
		// create node
		pack_node = parent->createNode("pack", "pack1");
		if (!pack_node)
			return error();
		// run creation script
		if (!pack_node->runCreateScript())
			return error();
		// connect the node
		input9 = parent->findNode("file1");  // find /obj/geo1/null1 as relative path
		if (input9)
		{
			pack_node->setInput(0, input9);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		pack_node->moveToGoodPosition();

		// Copy to Points Node
		OP_Node* copy_to_points_node;
		OP_Node* input10;
		OP_Node* input11;
		// create node
		copy_to_points_node = parent->createNode("copytopoints", "copytopoints1");
		if (!copy_to_points_node)
			return error();
		// run creation script
		if (!copy_to_points_node->runCreateScript())
			return error();
		// connect the node
		input10 = parent->findNode("pack1");  // find /obj/geo1/null1 as relative path
		input11 = parent->findNode("scatter1");
		if (input10)
		{
			copy_to_points_node->setInput(0, input10);       // set first input to /obj/null1
		}
		if (input11) {
			copy_to_points_node->setInput(1, input11);
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		copy_to_points_node->moveToGoodPosition();

		// Merge Node
		OP_Node* merge_node;
		OP_Node* input12;
		OP_Node* input13;
		// create node
		merge_node = parent->createNode("merge", "merge1");
		if (!merge_node)
			return error();
		// run creation script
		if (!merge_node->runCreateScript())
			return error();
		//set parameters
		merge_node->setRender(true);
		merge_node->setDisplay(true);
		// connect the node
		input12 = parent->findNode("switch1");  // find /obj/geo1/null1 as relative path
		input13 = parent->findNode("copytopoints1");
		if (input12)
		{
			merge_node->setInput(0, input12);       // set first input to /obj/null1
		}
		if (input13) {
			merge_node->setInput(1, input13);
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		merge_node->moveToGoodPosition();
		
		// Select the custom node
		OPgetDirector()->clearPickedItems();
		OP_Node* custom_node;
		custom_node = parent->findNode("CusLsystem1");
		if (custom_node) {
			custom_node->pickRequest(true);
		}

		networkCreated = 1;
	}

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
	std::string plant1File = PLANT1(now).toStdString();

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

		// Maybe comment out, maybe not? ~SERENA
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

