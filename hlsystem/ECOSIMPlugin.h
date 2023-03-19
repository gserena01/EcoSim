

#ifndef __LSYSTEM_PLUGIN_h__
#define __LSYSTEM_PLUGIN_h__

//#include <GEO/GEO_Point.h>
//
#include <SOP/SOP_Node.h>
#include "LSystem.h"

///////// GLOBAL VARIABLES

// 10x10 cell grid for terrain 
// 0x0 is left bottom corner
#define TERRAIN_SIZE 10
// evaporation constance
#define EVAP_CONSTANT 3.0
// maximum water that a plant can absorb
#define MAX_ABSORB 5.0
// transpiration coefficient
#define TRANSPIRATION 3.0

//// Tree Growth State
// SEED = age 0, 1
#define SEED 0 
#define SEED_MIN_AGE 0
// JUVENILE = age 2, 3, 4
#define JUVENILE 1
#define JUVENILE_MIN_AGE 2
// MATURE = age >= 5
#define MATURE 2
#define MATURE_MIN_AGE 5
// DECAY = if absorbed less than threshold of water 
#define DECAY 3
// DEAD = to be deleted when displaying
#define DEAD 4


//// Tree Preferred Climate
// Threshold water absorption required for growth 
// Only mature trees can decay. Juvenile and seeds will die if they do not receive threshold of water 
#define WET_CLIMATE 3.0
#define DRY_CLIMATE 1.0

//// Tree Mesh Files
// TODO: will store strings to file locations
// height values for rectangles for now
float TreeMeshFiles[4];
TreeMeshFiles[SEED] = 0.1;
TreeMeshFiles[JUVENILE] = 0.5;
TreeMeshFiles[MATURE] = 1.0;
TreeMeshFiles[DECAY] = 0.75;


struct Tree {
    vec3 position;
    int age;
    int growthStage;
    /// int preferredClimate;  TODO: Subtask 6.3
    float waterAbsorbed;
    string meshFile;
};



namespace HDK_Sample {
class SOP_Lsystem : public SOP_Node
{
public:
    static OP_Node		*myConstructor(OP_Network*, const char *,
							    OP_Operator *);

    /// Stores the description of the interface of the SOP in Houdini.
    /// Each parm template refers to a parameter.
    static PRM_Template		 myTemplateList[];

    /// This optional data stores the list of local variables.
    static CH_LocalVariable	 myVariables[];

protected:

    // Climatic Processes on that grid cell
    void condensation();
    void soilWaterDiffusion();
    void absorption();
    void vegetationGrowth(Tree t, int x, int y);
    void evaporation(int x, int y, float biomass);

    // Helper functions
    // updates the growth stage of tree based on age
    void updateGrowthStage(Tree t);
    // updates display of tree on terrain based on growthStage
    // Ex: TreeMeshFile[t.growthStage]
    void displayTree(Tree t); // TODO: Subtask 3.1
    // Spawns a seedling in the grid cell
    void spawnSeed(vec3 parentPos);

    // takes in all trees currently in the scene and 
    // currently vapor can a 2D grid array representing all the summed vapor within that grid cell 
    // Ex: if vapor_values[0][0] = 3.0, then all the soil in the cell 0,0 (representing the left hand corner of the 1x1 cell) has 3.0 vapor
    void ecoSim();



	     SOP_Lsystem(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_Lsystem();

    /// Disable parameters according to other parameters.
    virtual unsigned		 disableParms();


    /// cookMySop does the actual work of the SOP computing, in this
    /// case, a LSYSTEM
    virtual OP_ERROR		 cookMySop(OP_Context &context);

    /// This function is used to lookup local variables that you have
    /// defined specific to your SOP.
    virtual bool		 evalVariableValue(
				    fpreal &val,
				    int index,
				    int thread);
    // Add virtual overload that delegates to the super class to avoid
    // shadow warnings.
    virtual bool		 evalVariableValue(
				    UT_String &v,
				    int i,
				    int thread)
				 {
				     return evalVariableValue(v, i, thread);
				 }

private:
    
    std::vector<Tree> trees;
    float[TERRAIN_SIZE][TERRAIN_SIZE][TERRAIN_SIZE] vapor_values;
    // assuming information stored, x, y, z (where z is height)
    float[TERRAIN_SIZE][TERRAIN_SIZE] precipitation_values;
    float[TERRAIN_SIZE][TERRAIN_SIZE] soilWater_values;
    float[TERRAIN_SIZE][TERRAIN_SIZE] vegetationWater_values;
    // how much water the plants on that cell need
	float[TERRAIN_SIZE][TERRAIN_SIZE] biomass_values;

    // keeps track of the seeds from that new round 
    // depletes into trees after growth stage (we don't want seeds to also grow this iteration)
    std::vector<Tree> seedlings; 
    
    // The following list of accessors simplify evaluating the parameters
    /// of the SOP.

    // PUT YOUR CODE HERE
	// Here you need to declare functions which need to be called from the .C file to 
	// constantly update the cook function, these functions help you get the current value that the node has
	// Example : To declare a function to fetch angle you need to do it this way 
	fpreal ANGLE(fpreal t) { return evalFloat("angle", 0, t); }
    fpreal STEP(fpreal t) { return evalFloat("step", 0, t); }
    void GRAMMAR(fpreal t, UT_String &str) { return evalString(str, "grammar", 0, t); }
    int ITERATIONS(fpreal t) { return evalInt("iterations", 0, t); }



	











    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    /// Member variables are stored in the actual SOP, not with the geometry
    /// In this case these are just used to transfer data to the local 
    /// variable callback.
    /// Another use for local data is a cache to store expensive calculations.

	// NOTE : You can declare local variables here like this  
    int		myCurrPoint;
    int		myTotalPoints;
};
} // End HDK_Sample namespace

#endif
