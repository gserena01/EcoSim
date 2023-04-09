
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
			1,				// Max # of sources
			SOP_Lsystem::myVariables,	// Local variables
			OP_FLAG_GENERATOR)		// Flag it as generator
	);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//PUT YOUR CODE HERE
//You need to declare your parameters here
static PRM_Name terrainName("terrain", "Terrain");
static PRM_Name	soilName("soil", "Soil Water");
static PRM_Name	vaporName("vapor", "Vapor Water");
static PRM_Name	seedgeoName("seedgeo", "Seed Geometry");
static PRM_Name	juvenilegeoName("juvenilegeo", "Juvenile Geometry");
static PRM_Name	maturegeoName("maturegeo", "Mature Geometry");
static PRM_Name	decayinggeoName("decayinggeo", "Decaying Geometry");
static PRM_Name	iterationName("iterations", "Iterations");
static PRM_Name seedName("seedPlacement", "Seed Placement");
static PRM_Name juvenileName("juvenilePlacement", "Juvenile Tree Placement");
static PRM_Name matureName("maturePlacement", "Mature Tree Placement");
static PRM_Name decayingName("decayingPlacement", "Decaying Tree Placement");

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//				     ^^^^^^^^    ^^^^^^^^^^^^^^^
//				     internal    descriptive version


// PUT YOUR CODE HERE
// You need to setup the initial/default values for your parameters here
static PRM_Default soilDefault(1.0);
static PRM_Default vaporDefault(1.0);
static PRM_Default iterationDefault(0);
static PRM_Range iterationRange(PRM_RANGE_UI,  0, 
								PRM_RANGE_UI, 30);
static PRM_Default terrainDefault(0, "$HIP/Assets/noise_texture.jpg");
static PRM_Default seedgeoDefault(0, "$HIP/Assets/seed.obj");
static PRM_Default juvenilegeoDefault(0, "$HIP/Assets/juvenile_tree.obj");
static PRM_Default maturegeoDefault(0, "$HIP/Assets/mature_tree.obj");
static PRM_Default decayinggeoDefault(0, "$HIP/Assets/decaying_tree.obj");
static PRM_Default seedDefault(0, "");
static PRM_Default juvenileDefault(0, "");
static PRM_Default matureDefault(0, "");
static PRM_Default decayingDefault(0, "");


////////////////////////////////////////////////////////////////////////////////////////

PRM_Template
SOP_Lsystem::myTemplateList[] = {
	// PUT YOUR CODE HERE
	// You now need to fill this template with your parameter name and their default value
	PRM_Template(PRM_PICFILE, PRM_Template::PRM_EXPORT_MIN, 1, &terrainName, &terrainDefault, 0),
	PRM_Template(PRM_GEOFILE, PRM_Template::PRM_EXPORT_MIN, 1, &seedgeoName, &seedgeoDefault, 0),
	PRM_Template(PRM_GEOFILE, PRM_Template::PRM_EXPORT_MIN, 1, &juvenilegeoName, &juvenilegeoDefault, 0),
	PRM_Template(PRM_GEOFILE, PRM_Template::PRM_EXPORT_MIN, 1, &maturegeoName, &maturegeoDefault, 0),
	PRM_Template(PRM_GEOFILE, PRM_Template::PRM_EXPORT_MIN, 1, &decayinggeoName, &decayinggeoDefault, 0),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &soilName, &soilDefault, 0),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &vaporName, &vaporDefault, 0),
	PRM_Template(PRM_INT, PRM_Template::PRM_EXPORT_MIN, 1, &iterationName, &iterationDefault, 0, &iterationRange),
	PRM_Template(PRM_STRING, PRM_Template::PRM_EXPORT_MIN, 1, &seedName, &seedDefault, 0),
	PRM_Template(PRM_STRING, PRM_Template::PRM_EXPORT_MIN, 1, &juvenileName, &juvenileDefault, 0),
	PRM_Template(PRM_STRING, PRM_Template::PRM_EXPORT_MIN, 1, &matureName, &matureDefault, 0),
	PRM_Template(PRM_STRING, PRM_Template::PRM_EXPORT_MIN, 1, &decayingName, &decayingDefault, 0),

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
	OP_Network* parent;
	parent = (OP_Network*)OPgetDirector()->findNode("/obj/geo1");

	// Create the node network
	if (networkCreated < 0) {
		// List of Nodes in the network
		OP_Node* null_input;
		OP_Node* heightfield_file_node;
		OP_Node* heightfield_node;
		OP_Node* heightfield_noise_node;
		OP_Node* switch_node;
		OP_Node* convert_heightfield_node;
		OP_Node* seed_group_node;
		OP_Node* seed_scatter_node;
		OP_Node* seed_file_node;
		OP_Node* seed_pack_node;
		OP_Node* seed_copy_to_points_node;
		OP_Node* juvenile_group_node;
		OP_Node* juvenile_scatter_node;
		OP_Node* juvenile_file_node;
		OP_Node* juvenile_pack_node;
		OP_Node* juvenile_copy_to_points_node;
		OP_Node* mature_group_node;
		OP_Node* mature_scatter_node;
		OP_Node* mature_file_node;
		OP_Node* mature_pack_node;
		OP_Node* mature_copy_to_points_node;
		OP_Node* decaying_group_node;
		OP_Node* decaying_scatter_node;
		OP_Node* decaying_file_node;
		OP_Node* decaying_pack_node;
		OP_Node* decaying_copy_to_points_node;
		OP_Node* color_node;
		OP_Node* merge_node;
		OP_Node* custom_node;

		// create node
		heightfield_file_node = parent->createNode("heightfield_file", "heightfield_file1");
		if (!heightfield_file_node)
			return error();
		// run creation script
		if (!heightfield_file_node->runCreateScript())
			return error();
		// set parameters
		heightfield_file_node->setString(UT_String("`chs(\"../CusEcoSim1/terrain\")`"), CH_STRING_LITERAL, "filename", 0, t);
		// set parameters
		heightfield_file_node->setInt("size", 0, 0, 65);
		// connect the node
		null_input = parent->findNode("null1");  // find /obj/geo1/null1 as relative path
		if (null_input)
		{
			heightfield_file_node->setInput(0, null_input);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		heightfield_file_node->moveToGoodPosition();

		// create node
		parent = (OP_Network*)OPgetDirector()->findNode("/obj/geo1");
		heightfield_node = parent->createNode("heightfield", "heightfield1");
		if (!heightfield_node)
			return error();
		// run creation script
		if (!heightfield_node->runCreateScript())
			return error();
		// set the parameters
		heightfield_node->setFloat("size", 0, t, 65.0f);
		heightfield_node->setFloat("size", 1, t, 65.0f);
		// connect the node
		if (null_input)
		{
			heightfield_node->setInput(0, null_input);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		heightfield_node->moveToGoodPosition();

		// create node
		heightfield_noise_node = parent->createNode("heightfield_noise", "heightfield_noise1");
		if (!heightfield_noise_node)
			return error();
		// run creation script
		if (!heightfield_noise_node->runCreateScript())
			return error();
		heightfield_noise_node->setFloat("amp", 0, t, 20.0f);
		heightfield_noise_node->setFloat("elementsize", 0, t, 20.0f);
		// connect the node
		if (heightfield_node)
		{
			heightfield_noise_node->setInput(0, heightfield_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		heightfield_noise_node->moveToGoodPosition();

		// create node
		switch_node = parent->createNode("switch", "switch1");
		if (!switch_node)
			return error();
		// run creation script
		if (!switch_node->runCreateScript())
			return error();
		// connect the node
		if (heightfield_noise_node)
		{
			switch_node->setInput(0, heightfield_noise_node);       // set first input to /obj/null1
		}
		if (heightfield_file_node) {
			switch_node->setInput(1, heightfield_file_node);
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		switch_node->moveToGoodPosition();

		// create node
		convert_heightfield_node = parent->createNode("convertheightfield", "convertheightfield1");
		if (!convert_heightfield_node)
			return error();
		// run creation script
		if (!convert_heightfield_node->runCreateScript())
			return error();
		// connect the node
		if (switch_node)
		{
			convert_heightfield_node->setInput(0, switch_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		convert_heightfield_node->moveToGoodPosition();

		// Seed Pipeline
		// (Seed) Group Node
		// create node
		seed_group_node = parent->createNode("groupcreate", "seedgroup");
		if (!seed_group_node)
			return error();
		// run creation script
		if (!seed_group_node->runCreateScript())
			return error();
		// set parameters
		seed_group_node->setString(UT_String("seedgroup"), CH_STRING_LITERAL, "groupname", 0, t);
		seed_group_node->setString(UT_String("`chs(\"../CusEcoSim1/seedPlacement\")`"), CH_STRING_LITERAL, "basegroup", 0, t);
		// connect the node
		if (convert_heightfield_node)
		{
			seed_group_node->setInput(0, convert_heightfield_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		seed_group_node->moveToGoodPosition();

		// Scatter node
		// create node
		seed_scatter_node = parent->createNode("scatter", "seedscatter");
		if (!seed_scatter_node)
			return error();
		// run creation script
		if (!seed_scatter_node->runCreateScript())
			return error();
		// set parameters
		seed_scatter_node->setString(UT_String("seedgroup"), CH_STRING_LITERAL, "group", 0, t);
		seed_scatter_node->setInt("generateby", 0, t, 1);
		seed_scatter_node->setInt("primcountattrib", 0, t, 1);

		// connect the node
		if (seed_group_node)
		{
			seed_scatter_node->setInput(0, seed_group_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		seed_scatter_node->moveToGoodPosition();

		// File Node
		// create node
		seed_file_node = parent->createNode("file", "seedfile");
		if (!seed_file_node)
			return error();
		// run creation script
		if (!seed_file_node->runCreateScript())
			return error();
		// set parameters
		seed_file_node->setString(UT_String("`chs(\"../CusEcoSim1/seedgeo\")`"), CH_STRING_LITERAL, "file", 0, t);
		// connect the node
		if (null_input)
		{
			seed_file_node->setInput(0, null_input);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		seed_file_node->moveToGoodPosition();

		// Pack Node
		// create node
		seed_pack_node = parent->createNode("pack", "seedpack");
		if (!seed_pack_node)
			return error();
		// run creation script
		if (!seed_pack_node->runCreateScript())
			return error();
		// connect the node
		if (seed_file_node)
		{
			seed_pack_node->setInput(0, seed_file_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		seed_pack_node->moveToGoodPosition();

		// Copy to Points Node
		// create node
		seed_copy_to_points_node = parent->createNode("copytopoints", "seedcopytopoints");
		if (!seed_copy_to_points_node)
			return error();
		// run creation script
		if (!seed_copy_to_points_node->runCreateScript())
			return error();
		// connect the node
		if (seed_pack_node)
		{
			seed_copy_to_points_node->setInput(0, seed_pack_node);       // set first input to /obj/null1
		}
		if (seed_scatter_node) {
			seed_copy_to_points_node->setInput(1, seed_scatter_node);
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		seed_copy_to_points_node->moveToGoodPosition();

		// Juveline Tree Pipeline
		// Juvenile Group Node
		// create node
		juvenile_group_node = parent->createNode("groupcreate", "juvenilegroup");
		if (!juvenile_group_node)
			return error();
		// run creation script
		if (!juvenile_group_node->runCreateScript())
			return error();
		// set parameters
		juvenile_group_node->setString(UT_String("juvenilegroup"), CH_STRING_LITERAL, "groupname", 0, t);
		juvenile_group_node->setString(UT_String("`chs(\"../CusEcoSim1/juvenilePlacement\")`"), CH_STRING_LITERAL, "basegroup", 0, t);
		// connect the node
		if (convert_heightfield_node)
		{
			juvenile_group_node->setInput(0, convert_heightfield_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		juvenile_group_node->moveToGoodPosition();

		// Scatter node
		// create node
		juvenile_scatter_node = parent->createNode("scatter", "juvenilegroup");
		if (!juvenile_scatter_node)
			return error();
		// run creation script
		if (!juvenile_scatter_node->runCreateScript())
			return error();
		// set parameters
		juvenile_scatter_node->setString(UT_String("juvenilegroup"), CH_STRING_LITERAL, "group", 0, t);
		juvenile_scatter_node->setInt("generateby", 0, t, 1);
		juvenile_scatter_node->setInt("primcountattrib", 0, t, 1);

		// connect the node
		if (juvenile_group_node)
		{
			juvenile_scatter_node->setInput(0, juvenile_group_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		juvenile_scatter_node->moveToGoodPosition();

		// File Node
		// create node
		juvenile_file_node = parent->createNode("file", "juvenilefile");
		if (!juvenile_file_node)
			return error();
		// run creation script
		if (!juvenile_file_node->runCreateScript())
			return error();
		// set parameters
		juvenile_file_node->setString(UT_String("`chs(\"../CusEcoSim1/juvenilegeo\")`"), CH_STRING_LITERAL, "file", 0, t);
		// connect the node
		if (null_input)
		{
			juvenile_file_node->setInput(0, null_input);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		juvenile_file_node->moveToGoodPosition();

		// Pack Node
		// create node
		juvenile_pack_node = parent->createNode("pack", "juvenilepack");
		if (!juvenile_pack_node)
			return error();
		// run creation script
		if (!juvenile_pack_node->runCreateScript())
			return error();
		// connect the node
		if (juvenile_file_node)
		{
			juvenile_pack_node->setInput(0, juvenile_file_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		juvenile_pack_node->moveToGoodPosition();

		// Copy to Points Node
		// create node
		juvenile_copy_to_points_node = parent->createNode("copytopoints", "juvenilecopytopoints");
		if (!juvenile_copy_to_points_node)
			return error();
		// run creation script
		if (!juvenile_copy_to_points_node->runCreateScript())
			return error();
		// connect the node
		if (juvenile_pack_node)
		{
			juvenile_copy_to_points_node->setInput(0, juvenile_pack_node);       // set first input to /obj/null1
		}
		if (juvenile_scatter_node) {
			juvenile_copy_to_points_node->setInput(1, juvenile_scatter_node);
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		juvenile_copy_to_points_node->moveToGoodPosition();

		// Mature Tree Pipeline
		// (Mature) Group Node
		// create node
		mature_group_node = parent->createNode("groupcreate", "maturegroup");
		if (!mature_group_node)
			return error();
		// run creation script
		if (!mature_group_node->runCreateScript())
			return error();
		// set parameters
		mature_group_node->setString(UT_String("maturegroup"), CH_STRING_LITERAL, "groupname", 0, t);
		mature_group_node->setString(UT_String("`chs(\"../CusEcoSim1/maturePlacement\")`"), CH_STRING_LITERAL, "basegroup", 0, t);
		// connect the node
		if (convert_heightfield_node)
		{
			mature_group_node->setInput(0, convert_heightfield_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		mature_group_node->moveToGoodPosition();

		// Scatter node
		// create node
		mature_scatter_node = parent->createNode("scatter", "maturescatter");
		if (!mature_scatter_node)
			return error();
		// run creation script
		if (!mature_scatter_node->runCreateScript())
			return error();
		// set parameters
		mature_scatter_node->setString(UT_String("maturegroup"), CH_STRING_LITERAL, "group", 0, t);
		mature_scatter_node->setInt("generateby", 0, t, 1);
		mature_scatter_node->setInt("primcountattrib", 0, t, 1);

		// connect the node
		if (mature_group_node)
		{
			mature_scatter_node->setInput(0, mature_group_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		mature_scatter_node->moveToGoodPosition();

		// File Node
		// create node
		mature_file_node = parent->createNode("file", "maturefile");
		if (!mature_file_node)
			return error();
		// run creation script
		if (!mature_file_node->runCreateScript())
			return error();
		// set parameters
		mature_file_node->setString(UT_String("`chs(\"../CusEcoSim1/maturegeo\")`"), CH_STRING_LITERAL, "file", 0, t);
		// connect the node
		if (null_input)
		{
			mature_file_node->setInput(0, null_input);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		mature_file_node->moveToGoodPosition();

		// Pack Node
		// create node
		mature_pack_node = parent->createNode("pack", "maturepack");
		if (!mature_pack_node)
			return error();
		// run creation script
		if (!mature_pack_node->runCreateScript())
			return error();
		// connect the node
		if (mature_file_node)
		{
			mature_pack_node->setInput(0, mature_file_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		mature_pack_node->moveToGoodPosition();

		// Copy to Points Node
		// create node
		mature_copy_to_points_node = parent->createNode("copytopoints", "maturecopytopoints");
		if (!mature_copy_to_points_node)
			return error();
		// run creation script
		if (!mature_copy_to_points_node->runCreateScript())
			return error();
		// connect the node
		if (mature_pack_node)
		{
			mature_copy_to_points_node->setInput(0, mature_pack_node);       // set first input to /obj/null1
		}
		if (mature_scatter_node) {
			mature_copy_to_points_node->setInput(1, mature_scatter_node);
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		mature_copy_to_points_node->moveToGoodPosition();

		// Decaying Tree Pipeline
		// (Decaying) Group Node
		// create node
		decaying_group_node = parent->createNode("groupcreate", "decayinggroup");
		if (!decaying_group_node)
			return error();
		// run creation script
		if (!decaying_group_node->runCreateScript())
			return error();
		// set parameters
		decaying_group_node->setString(UT_String("decayinggroup"), CH_STRING_LITERAL, "groupname", 0, t);
		decaying_group_node->setString(UT_String("`chs(\"../CusEcoSim1/decayingPlacement\")`"), CH_STRING_LITERAL, "basegroup", 0, t);
		// connect the node
		if (convert_heightfield_node)
		{
			decaying_group_node->setInput(0, convert_heightfield_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		decaying_group_node->moveToGoodPosition();

		// Scatter node
		// create node
		decaying_scatter_node = parent->createNode("scatter", "decayingscatter");
		if (!decaying_scatter_node)
			return error();
		// run creation script
		if (!decaying_scatter_node->runCreateScript())
			return error();
		// set parameters
		decaying_scatter_node->setString(UT_String("decayinggroup"), CH_STRING_LITERAL, "group", 0, t);
		decaying_scatter_node->setInt("generateby", 0, t, 1);
		decaying_scatter_node->setInt("primcountattrib", 0, t, 1);

		// connect the node
		if (decaying_group_node)
		{
			decaying_scatter_node->setInput(0, decaying_group_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		decaying_scatter_node->moveToGoodPosition();

		// File Node
		// create node
		decaying_file_node = parent->createNode("file", "decayingfile");
		if (!decaying_file_node)
			return error();
		// run creation script
		if (!decaying_file_node->runCreateScript())
			return error();
		// set parameters
		decaying_file_node->setString(UT_String("`chs(\"../CusEcoSim1/decayinggeo\")`"), CH_STRING_LITERAL, "file", 0, t);
		// connect the node
		if (null_input)
		{
			decaying_file_node->setInput(0, null_input);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		decaying_file_node->moveToGoodPosition();

		// Pack Node
		// create node
		decaying_pack_node = parent->createNode("pack", "decayingpack");
		if (!decaying_pack_node)
			return error();
		// run creation script
		if (!decaying_pack_node->runCreateScript())
			return error();
		// connect the node
		if (decaying_file_node)
		{
			decaying_pack_node->setInput(0, decaying_file_node);       // set first input to /obj/null1
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		decaying_pack_node->moveToGoodPosition();

		// Copy to Points Node
		// create node
		decaying_copy_to_points_node = parent->createNode("copytopoints", "decayingcopytopoints");
		if (!decaying_copy_to_points_node)
			return error();
		// run creation script
		if (!decaying_copy_to_points_node->runCreateScript())
			return error();
		// connect the node
		if (decaying_pack_node)
		{
			decaying_copy_to_points_node->setInput(0, decaying_pack_node);       // set first input to /obj/null1
		}
		if (decaying_scatter_node) {
			decaying_copy_to_points_node->setInput(1, decaying_scatter_node);
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		decaying_copy_to_points_node->moveToGoodPosition();

		// Color Node
		color_node = parent->createNode("color", "color1");
		if (!color_node)
			return error();
		if (!color_node->runCreateScript())
			return error();
		color_node->setInt("colortype", 0, t, 3);
		color_node->setString(UT_String("height"), CH_STRING_LITERAL, "rampattribute", 0, t);
		color_node->setString(UT_String("@height.min"), CH_STRING_LITERAL, "ramprange", 0, t);
		color_node->setString(UT_String("@height.max"), CH_STRING_LITERAL, "ramprange", 1, t);

		color_node->setFloat("ramp1c", 0, t, 0.075f);
		color_node->setFloat("ramp1c", 1, t, 0.3f);
		color_node->setFloat("ramp1c", 2, t, 0.075f);

		color_node->setFloat("ramp2c", 0, t, 0.3f);
		color_node->setFloat("ramp2c", 1, t, 0.1875f);
		color_node->setFloat("ramp2c", 2, t, 0.075f);
		if (convert_heightfield_node)
		{
			color_node->setInput(0, convert_heightfield_node);
		}
		color_node->moveToGoodPosition();

		// Merge Node
		// create node
		merge_node = parent->createNode("merge", "merge1");
		if (!merge_node)
			return error();
		// run creation script
		if (!merge_node->runCreateScript())
			return error();
		// connect the node
		if (color_node)
		{
			merge_node->setInput(0, color_node);       // set first input to /obj/null1
		}
		if (seed_copy_to_points_node) {
			merge_node->setInput(1, seed_copy_to_points_node);
		}
		if (juvenile_copy_to_points_node) {
			merge_node->setInput(2, juvenile_copy_to_points_node);
		}
		if (mature_copy_to_points_node) {
			merge_node->setInput(3, mature_copy_to_points_node);
		}
		if (decaying_copy_to_points_node) {
			merge_node->setInput(4, decaying_copy_to_points_node);
		}
		// now that done we're done connecting it, position it relative to its
		// inputs
		merge_node->moveToGoodPosition();
		
		// Select the custom node
		OPgetDirector()->clearPickedItems();
		custom_node = parent->findNode("CusEcoSim1");
		if (custom_node) {
			custom_node->setInput(0, merge_node);
			custom_node->pickRequest(true);
			custom_node->moveToGoodPosition();
		}

		printf("\nWELCOME TO THE ECOSIM PLUGIN!\nPlease select a terrain image and four different tree growth geometry files");
		networkCreated = 1;
	}

	// gather parameters from GUI
	std::string terrainFile = TERRAIN(now).toStdString();
	std::string seedgeoFile = SEEDGEO(now).toStdString();

	float soil = SOIL(now);
	float vapor = VAPOR(now);
	int itr = ITERATIONS(now);

	// update eco simulation
	EcoSim eco;
	eco.setTrees();
	for (int i = 0; i < itr; ++i) {
			eco.cycle();
	}

	// get list of trees to render
	std::string seeds;
	std::string juveniles;
	std::string mature;
	std::string decaying;
	eco.getTreePositions(seeds, juveniles, mature, decaying);
	
	// Update node parameters that changed during iteration:
	OP_Node* seed_group_node;
	seed_group_node = parent->findNode("seedgroup");
	if (seeds.length() > 0) {
		seed_group_node->setInt("groupbase", 0, t, 1);
		seed_group_node->setString(UT_String(seeds), CH_STRING_LITERAL, "basegroup", 0, t);
	}
	else {
		seed_group_node->setInt("groupbase", 0, t, 0);
	}

	OP_Node* juvenile_group_node;
	juvenile_group_node = parent->findNode("juvenilegroup");
	if (juveniles.length() > 0) {
		juvenile_group_node->setInt("groupbase", 0, t, 1);
		juvenile_group_node->setString(UT_String(juveniles), CH_STRING_LITERAL, "basegroup", 0, t);
	}
	else {
		juvenile_group_node->setInt("groupbase", 0, t, 0);
	}	

	OP_Node* mature_group_node;
	mature_group_node = parent->findNode("maturegroup");
	if (mature.length() > 0) {
		mature_group_node->setInt("groupbase", 0, t, 1);
		mature_group_node->setString(UT_String(mature), CH_STRING_LITERAL, "basegroup", 0, t);
	}
	else {
		mature_group_node->setInt("groupbase", 0, t, 0);
	}

	OP_Node* decaying_group_node;
	decaying_group_node = parent->findNode("decayinggroup");
	if (decaying.length() > 0) {
		decaying_group_node->setString(UT_String(decaying), CH_STRING_LITERAL, "basegroup", 0, t);
	}
	else {
		decaying_group_node->setInt("groupbase", 0, t, 0);
	}

	// 1. Lock inputs, causes them to be cooked first.
	if (lockInputs(context) >= UT_ERROR_ABORT)
		return error();
	// 2. Copy input geometry into our gdp
	duplicateSource(0, context);
	// 3. Parse and create myGroup
	if (cookInputGroups(context) >= UT_ERROR_ABORT)
		return error();
	// 4. Unlock inputs
	unlockInputs();

	// PUT YOUR CODE HERE 
	UT_Interrupt* boss;

	int divisions = 5;	// We need twice our divisions of points
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

		// Start the interrupt server
		if (boss->opStart("Building LSYSTEM"))
		{
			
		}

		// Tell the interrupt server that we've completed. Must do this
		// regardless of what opStart() returns.
		boss->opEnd();
	}

	myCurrPoint = -1;
	return error();
}

