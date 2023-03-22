#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "vec.h"

struct Tree {
    vec3 position;
    int age;
    int growthStage;
    // int preferredClimate;  TODO: Subtask 6.3
    int id;
    std::string meshFile;
};


class EcoSim
{
public:
    ////// STATIC GLOBAL VALUES

    // default 10x10x10 cell grid for terrain
    // 0x0 is left bottom corner
    const static int TERRAIN_SIZE = 3;

    // evaporation constance
    constexpr const static float EVAP_CONSTANT = 1.0;

    // water that a wet climate plant requires to grow
    // all wet climate trees requitrre half their mass in water
    // Only mature trees can decay. Juvenile and seeds will die if they do not receive threshold of water
    constexpr static const float ABSORB_WET_CLIMATE = 0.5;

    // transpiration coefficient
    constexpr static const float TRANSPIRATION = 2.0;

    int treeID;

    float theta = 17.0;
    float seedR = 0.35;

    //// Tree Growth State
    // Tree indices
    const static int SEED = 0;
    const static int JUVENILE = 1;
    const static int MATURE = 2;
    // DECAY = if absorbed less than threshold of water
    const static int DECAY = 3;
    // DEAD = to be deleted when displaying
    const static int DEAD = 4;

    // Tree age years -> growth stage
    // SEED = age 0, 1
    const static int SEED_MIN_AGE = 0;
    // JUVENILE = age 2, 3, 4
    const static int JUVENILE_MIN_AGE = 2;
    // MATURE = age >= 5
    const static int MATURE_MIN_AGE = 5;

    //// TreeMass AKA Heights and biomass values
    // TODO: will store strings to file locations
    constexpr const static float TreeMass[5] = { 0.1, 0.5, 1.0, 0.75, 0.0 };

    EcoSim();

    // takes in all trees currently in the scene and 
    // currently vapor can a 2D grid array representing all the summed vapor within that grid cell 
    // Ex: if vapor_values[0][0] = 3.0, then all the soil in the cell 0,0 (representing the left hand corner of the 1x1 cell) has 3.0 vapor
    void cycle();


    std::vector<Tree> trees;

    // SETTERS
    void setVapor(float v);
    void setTrees();
    void setSoilWater(float s);

    void setTreesDecayTest();

    // TODO: delete later
    // PRINTERS
    int getTreeSize();
    void printVapor();
    void printPrecipitation();
    void printSoilWater();
    void printVegNeeds();
    void printBiomass();

protected:

    /// Climatic Processes on that grid cell
    void condensation();
    void soilWaterDiffusion();
    void absorption();
    void absorptionReqs();  // Set the amount of water required for the vegetation on a grid cell
    void vegetationGrowth(Tree& t, int x, int y);
    void evaporation();

    // Helper functions
    // updates the growth stage of tree based on age
    void increaseGrowthStage(Tree& t);
    void updateSeeds();

    // DISPLAY FUNCTIONS SHOULD CONNECT TO PLUGIN
    // updates display of tree on terrain based on growthStage
    // Ex: TreeMeshFile[t.growthStage]
    void displayTree(Tree& t); // TODO: Subtask 3.1
    // Spawns a seedling in the grid cell
    void spawnSeed(vec3 parentPos);

private:

    ///// WATER CYCLE MAP VALUES

    // 3D volume of vapor in column 
    // assuming information stored, x, y, z (where z is height)
    float vapor_values[TERRAIN_SIZE][TERRAIN_SIZE][TERRAIN_SIZE];

    // amount of rain
    float precipitation_values[TERRAIN_SIZE][TERRAIN_SIZE] = { {0.0} };

    // amount of water in the soil
    float soilWater_values[TERRAIN_SIZE][TERRAIN_SIZE];

    // how much water the plants on that cell need
    float vegetationNeeds_values[TERRAIN_SIZE][TERRAIN_SIZE] = { {0.0} };

    // amount of biomass on the grid cell 
    float biomass_values[TERRAIN_SIZE][TERRAIN_SIZE] = { {0.0} };

    // keeps track of the seeds from that new round 
    // depletes into trees after growth stage (we don't want seeds to also grow this iteration)
    std::vector<Tree> seedlings;
};
