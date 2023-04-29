

#ifndef __LSYSTEM_PLUGIN_h__
#define __LSYSTEM_PLUGIN_h__

//#include <GEO/GEO_Point.h>
//
#include <SOP/SOP_Node.h>
#include "LSystem.h"
#include "EcoSim.h"

namespace HDK_Sample {
    class SOP_Lsystem : public SOP_Node
    {
    public:
        static OP_Node* myConstructor(OP_Network*, const char*,
            OP_Operator*);

        /// Stores the description of the interface of the SOP in Houdini.
        /// Each parm template refers to a parameter.
        static PRM_Template		 myTemplateList[];

        /// This optional data stores the list of local variables.
        static CH_LocalVariable	 myVariables[];

    protected:

        SOP_Lsystem(OP_Network* net, const char* name, OP_Operator* op);
        virtual ~SOP_Lsystem();

        /// Disable parameters according to other parameters.
        virtual unsigned		 disableParms();


        /// cookMySop does the actual work of the SOP computing, in this
        /// case, a LSYSTEM
        virtual OP_ERROR		 cookMySop(OP_Context& context);

        /// This function is used to lookup local variables that we have
        /// defined specific to our SOP.
        virtual bool		 evalVariableValue(
            fpreal& val,
            int index,
            int thread);
        // Add virtual overload that delegates to the super class to avoid
        // shadow warnings.
        virtual bool		 evalVariableValue(
            UT_String& v,
            int i,
            int thread)
        {
            return evalVariableValue(v, i, thread);
        }

    private:
        /// The following list of accessors simplify evaluating the parameters
        /// of the SOP.

        // Here we declare functions which need to be called from the .C file to 
        // constantly update the cook function, these functions help you get the current value that the node has
        int YEARS(fpreal t) { return evalInt("years", 0, t); }
        float EVAP(fpreal t) { return evalFloat("evap", 0, t); }
        void SEEDSTR(fpreal t, UT_String& str) { return evalString(str, "seedPlacement", 0, t); }
        void JUVSTR(fpreal t, UT_String& str) { return evalString(str, "juvenilePlacement", 0, t); }
        void MATURESTR(fpreal t, UT_String& str) { return evalString(str, "maturePlacement", 0, t); }
        void DECAYSTR(fpreal t, UT_String& str) { return evalString(str, "decayingPlacement", 0, t); }
        int CUSTOMTERRAIN(fpreal t) { return evalInt("customterrain", 0, t); }


        UT_String SEEDGEO(fpreal t) {
            UT_String str;
            evalString(str, "seedgeo", 0, t);
            return str;
        }

        UT_String TERRAIN(fpreal t) {
            UT_String str;
            evalString(str, "terrain", 0, t);
            return str;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////

        /// Member variables are stored in the actual SOP, not with the geometry
        /// In this case these are just used to transfer data to the local 
        /// variable callback.
        /// Another use for local data is a cache to store expensive calculations.

        // NOTE : You can declare local variables here like this  
        int		myCurrPoint;
        int		myTotalPoints;
        int     networkCreated;
    };
} // End HDK_Sample namespace

#endif
