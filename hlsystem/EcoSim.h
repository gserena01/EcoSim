#pragma once

#include <string>
#include <vector>
#include "vec.h"

struct Tree {
    vec3 position;
    int age;
    int growthStage;
    /// int preferredClimate;  TODO: Subtask 6.3
    string meshFile;
};


class EcoSim
{
public:
    EcoSim();

    // takes in all trees currently in the scene and 
    // currently vapor can a 2D grid array representing all the summed vapor within that grid cell 
    // Ex: if vapor_values[0][0] = 3.0, then all the soil in the cell 0,0 (representing the left hand corner of the 1x1 cell) has 3.0 vapor
    void cycle();


    void setVapor();
    void setTrees();

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

    // DISPLAY FUNCTIONS SHOULD CONNECT TO PLUGIN
    // updates display of tree on terrain based on growthStage
    // Ex: TreeMeshFile[t.growthStage]
    void displayTree(Tree t); // TODO: Subtask 3.1
    // Spawns a seedling in the grid cell
    void spawnSeed(vec3 parentPos);

private:

    ////// STATIC GLOBAL VALUES

    // default 10x10x10 cell grid for terrain 
    // 0x0 is left bottom corner
    static int TERRAIN_SIZE = 10;
    
    // evaporation constance
    static float EVAP_CONSTANT = 3.0;

    // water that a wet climate plant requires to grow
    // all wet climate trees require half their mass in water 
    // Only mature trees can decay. Juvenile and seeds will die if they do not receive threshold of water 
    static float ABSORB_WET_CLIMATE = 0.5;

    // transpiration coefficient
    static float TRANSPIRATION = 3.0;

    //// Tree Growth State
    // Tree indices
    static int SEED = 0; 
    static int JUVENILE = 1;
    static int MATURE = 2;
    // DECAY = if absorbed less than threshold of water 
    static int DECAY = 3;
    // DEAD = to be deleted when displaying
    static int DEAD = 4;

    // Tree age years -> growth stage
    // SEED = age 0, 1
    static int SEED_MIN_AGE = 0;
    // JUVENILE = age 2, 3, 4
    static int JUVENILE_MIN_AGE = 2;
    // MATURE = age >= 5
    static int MATURE_MIN_AGE = 5;

    //// Tree Mesh Files AKA Heights and biomass values 
    // TODO: will store strings to file locations
    // height values for rectangles for now
    static float TreeMeshFiles[4];
    TreeMeshFiles[SEED] = 0.1;
    TreeMeshFiles[JUVENILE] = 0.5;
    TreeMeshFiles[MATURE] = 1.0;
    TreeMeshFiles[DECAY] = 0.75;
    
    ///// WATER CYCLE MAP VALUES

    // vector of Trees
    std::vector<Tree> trees;

    // 3D volume of vapor in column 
    // assuming information stored, x, y, z (where z is height)
    float[TERRAIN_SIZE][TERRAIN_SIZE][TERRAIN_SIZE] vapor_values;

    // amount of rain
    float[TERRAIN_SIZE][TERRAIN_SIZE] precipitation_values;

    // amount of water in the soil
    float[TERRAIN_SIZE][TERRAIN_SIZE] soilWater_values;

    // how much water the plants on that cell need
    float[TERRAIN_SIZE][TERRAIN_SIZE] vegetationNeeds_values;

    // amount of biomass on the grid cell 
	float[TERRAIN_SIZE][TERRAIN_SIZE] biomass_values;

    // keeps track of the seeds from that new round 
    // depletes into trees after growth stage (we don't want seeds to also grow this iteration)
    std::vector<Tree> seedlings; 
};
