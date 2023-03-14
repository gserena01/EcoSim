

#ifndef __LSYSTEM_PLUGIN_h__
#define __LSYSTEM_PLUGIN_h__

//#include <GEO/GEO_Point.h>
//
#include <SOP/SOP_Node.h>
#include "LSystem.h"

///////// GLOBAL VARIABLES
//// Tree Growth State
// SEED = age 0, 1
#define SEED 0 
// JUVENILE = age 2, 3, 4
#define JUVENILE 1
// MATURE = age >= 5
#define MATURE 2 
// DECAY = if absorbed less than threshold of water 
#define DECAY 3

//// Tree Preferred Climate
// Threshold water absorption required for growth 
// Only mature trees can decay. Juvenile and seeds will die if they do not receive threshold of water 
#define WET_CLIMATE 3.0
#define DRY_CLIMATE 1.0

struct Tree {
    vec3 position;
    int age;
    int growthStage;
    int preferredClimate;
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
    /// The following list of accessors simplify evaluating the parameters
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
