
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
newSopOperator(OP_OperatorTable *table)
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
static PRM_Name		angleName("angle", "Angle");
static PRM_Name		stepName("step", "Step");
static PRM_Name		grammarName("grammar", "Grammar");
static PRM_Name		iterationName("iterations", "Iterations");

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//				     ^^^^^^^^    ^^^^^^^^^^^^^^^
//				     internal    descriptive version


// PUT YOUR CODE HERE
// You need to setup the initial/default values for your parameters here
static PRM_Default angleDefault(30.0);	
static PRM_Default stepDefault(4.0);
static PRM_Default grammarDefault(0, "");
static PRM_Default iterationDefault(1);


////////////////////////////////////////////////////////////////////////////////////////

PRM_Template
SOP_Lsystem::myTemplateList[] = {
	// PUT YOUR CODE HERE
	// You now need to fill this template with your parameter name and their default value
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &angleName, &angleDefault, 0),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &stepName, &stepDefault, 0),
	PRM_Template(PRM_STRING, 1, &grammarName, &grammarDefault),
	PRM_Template(PRM_INT, PRM_Template::PRM_EXPORT_MIN, 1, &iterationName, &iterationDefault, 0),

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
SOP_Lsystem::evalVariableValue(fpreal &val, int index, int thread)
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
		val = (fpreal) myCurrPoint;
		return true;
	    case VAR_NPT:
		val = (fpreal) myTotalPoints;
		return true;
	    default:
		/* do nothing */;
	}
    }
    // Not one of our variables, must delegate to the base class.
    return SOP_Node::evalVariableValue(val, index, thread);
}

OP_Node *
SOP_Lsystem::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_Lsystem(net, name, op);
}

SOP_Lsystem::SOP_Lsystem(OP_Network *net, const char *name, OP_Operator *op)
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

SOP_Lsystem::spawnSeed(vec3 parentPos) {
	Tree babyTree;
	float deltaX = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float deltaY = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

	// TODO: z value needs to be from height field Subtask 4.3
	babyTree.position = vec3(deltaX, deltaY, parentPos[2]);
	babyTree.age = 0;
	babyTree.growthStage = SEED;
	// TODO: connect to particle node????

	seedlings.push(babyTree);
}

SOP_Lsystem::updateGrowthStage(Tree t) {
	// update growth stage in tree struct
	// tree was decaying
	if (t.growStage == DECAY) {
		t.growthStage = MATURE;
	} else if (t.age < JUVENILE_MIN_AGE) { 
		t.growthStage == SEED;
	} else if (t.age < MATURE_MIN_AGE) {
		t.growthStage == JUVENILE;
	} else if (t.age == MATURE_MIN_AGE) { // just became MATURE
		t.growthStage == MATURE;
		spawnSeed(t.position);
	}
}

SOP_Lsystem::vegetationGrowth(Tree t, int x, int y) {
	// Update vegetation based on water absorption
	t.waterAbsorbed = soilWater_values[x, y];
	if(t.waterAbsorbed >= WET_CLIMATE) {
		// GROW
		t.age += 1;
		updateGrowthStage(&t);
	} else {
		// DECAY 
		if (t.growthStage != MATURE) {
			// SEED, JUVENILE, and DECAYing trees die
			t.growthStage = DEAD;
		} else {
			t.growthStage = DECAY;
		}
	}
}

void SOP_Lsystem::condensation() {
	// Update precipitation map based on vapor values
	// sum all vapor into precipitation
	for (int x = 0; x < TERRAIN_SIZE; ++x) {
		for(int y = 0; y < TERRAIN_SIZE; ++y) {
			float total_rain = 0.0;
			for(int z = 0; z < TERRAIN_SIZE; ++z) {
				total_rain += vapor_values[x][y][z];
			}
			precipitation_values[x][y] = total_rain;
		}
	}

}

void SOP_Lsystem::soilWaterDiffusion() {
	// Updates soilWater_values map based on precipitation
	for(int x = 0; x < TERRAIN_SIZE; ++x) {
		for(int y = 0; y < TERRAIN_SIZE; ++y) {
			soilWater_values[x][y] += precipitation_values[x][y] - vegetationWater_values[x][y] - EVAP_CONSTANT;
		}
	}
}

void SOP_Lsystem::absorption() {
	// Updates vegetationWater_values based on how much water is needed by the plants in the cell
	for (int x = 0; x < TERRAIN_SIZE; ++x) {
		for (int y = 0; y < TERRAIN_SIZE; ++y) {
			vegetationWater_values[x][y] = MAX_ABSORB * biomass_values[x][y];
		}
	}
}

void SOP_Lsystem::evaporation() {
	// Updates vapor map based on vegetaion biomass
	for (int x = 0; x < TERRAIN_SIZE; ++x) {
		for (int y = 0; y < TERRAIN_SIZE; ++y) {
			for(int z = 0; z < TERRAIN_SIZE; ++z) {
				vapor_values[x][y][z] = TRANSPIRATION * (1.0 / z) * biomass_values[x][y];
				// TODO: apply noise (Gaussian), Subtask 6.1
			}
		}
	}
}

SOP_Lsystem::ecoSim() 
{
	// turn VAPOR into PRECIPITATION
	condensation(); // updates precipitation map
			
	// turn PRECIPITATION into SOIL WATER
	soilWaterDiffusion(); // updates soil water map

	// turn VEGETATION WATER into VEGETATION
	float[TERRAIN_SIZE][TERRAIN_SIZE] new_biomass_values = 0.0;
	for(Tree t : trees) {
		int x = floor(t.position[0]);
		int y = floor(t.position[1]);
		// update tree waterAbsorbed with VEGETATION WATER
		vegetationGrowth(&t);
		displayTree(&t); // TODO: Subtask 3.1
		new_biomass_values[x][y] += TreeMeshFiles[t.growthStage];
	}
	biomass_values = new_biomass_values;

	// update the water needs of VEGETATION
	absorption(); // update vegetation water map

	// Add seedlings to Trees and Biomass
	while(seedlings.size() > 0) {
		Tree t = seedlings.pop_back();
		trees.push_back(t);

		int x = floor(t.position[0]);
		int y = floor(t.position[1]);
		biomass_values[x][y] += TreeMeshFiles[t.growthStage];
	}
	
	// turn VEGETATION into EVAPORATION
	evaporation(x, y, biomass_values[x][y]);

}


OP_ERROR
SOP_Lsystem::cookMySop(OP_Context &context)
{
	fpreal		 now = context.getTime();

	// PUT YOUR CODE HERE
	// Decare the necessary variables and get always keep getting the current value in the node
	// For example to always get the current angle thats set in the node ,you need to :
	//    float angle;
	//    angle = ANGLE(now)       
    //    NOTE : ANGLE is a function that you need to use and it is declared in the header file to update your values instantly while cooking 
	float angle = ANGLE(now);
	float step = STEP(now);
	int itr = ITERATIONS(now);
	UT_String grammar;
	GRAMMAR(now, grammar);
	LSystem myplant;
	
	///////////////////////////////////////////////////////////////////////////

	//PUT YOUR CODE HERE
	// Next you need to call your Lystem cpp functions 
	// Below is an example , you need to call the same functions based on the variables you declare
    // myplant.loadProgramFromString("F\nF->F[+F]F[-F]";  
    // myplant.setDefaultAngle(30.0f);
    // myplant.setDefaultStep(1.0f);

	myplant.loadProgram(grammar.toStdString());
	myplant.setDefaultAngle(angle);
	myplant.setDefaultStep(step);

	///////////////////////////////////////////////////////////////////////////////

	// PUT YOUR CODE HERE
	// You the need call the below function for all the generations, so that the end points points will be
	// stored in the branches vector, you need to declare them first

	std::vector<LSystem::Branch> branches;

	for (int i = 0; i < itr ; i++)
	{
		  myplant.process(i, branches);
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
    GU_PrimPoly		*poly;
    int			 i;
    UT_Interrupt	*boss;

    // Since we don't have inputs, we don't need to lock them.

    divisions  = 5;	// We need twice our divisions of points
    myTotalPoints = divisions;		// Set the NPT local variable value
    myCurrPoint   = 0;			// Initialize the PT local variable

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

		// loop through branches
		for (int i = 0; i < branches.size(); ++i) {
			LSystem::Branch b = branches.at(i);

			vec3 s = b.first;
			vec3 e = b.second;

			UT_Vector3 start;
			start(xcoord) = s[0];
			start(ycoord) = s[2];
			start(zcoord) = s[1];

			UT_Vector3 end;
			end(xcoord) = e[0];
			end(ycoord) = e[2];
			end(zcoord) = e[1];

			poly = GU_PrimPoly::build(gdp, 2, 0, 1);

			GA_Offset ptoff = poly->getPointOffset(0);
			gdp->setPos3(ptoff, start);

			ptoff = poly->getPointOffset(1);
			gdp->setPos3(ptoff, end);

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

