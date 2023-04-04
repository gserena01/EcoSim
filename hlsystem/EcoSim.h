#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "vec.h"
#include <array>

struct Tree {
    vec3 position;
    int age;
    int growthStage;
    // int preferredClimate;  TODO: Subtask 6.3
    int id;
};


class EcoSim
{
public:
    ////// STATIC GLOBAL VALUES

    // default 32x32x4 cell grid for terrain
    // 0x0 is left bottom corner
    // Paper uses ~31x31 cell grid where each grid is 1.5m^2
    const static int TERRAIN_SIZE = 32;
    const static int TERRAIN_HEIGHT = 4;

    // evaporation constance
    constexpr const static float EVAP_CONSTANT = 1.5;

    // water that a wet climate plant requires to grow
    // all wet climate trees requitrre half their mass in water
    // Only mature trees can decay. Juvenile and seeds will die if they do not receive threshold of water
    constexpr static const float ABSORB_WET_CLIMATE = 0.1;

    // transpiration coefficient
    // Paper cites 0.08 for Conifers and 0.16 for Deciduous
    constexpr static const float TRANSPIRATION = 0.16;


    ////// TREE STRUCT INFORMATION

    // Data for tree seed spawn
    int treeID;
    float theta = 17.0;
    float seedR = 2.05; // paper cites numbers 1.5 - 2.6

    //// Tree Growth State INDICES
    const static int SEED = 0;
    const static int JUVENILE = 1;
    const static int MATURE = 2;
    const static int DECAY = 3;
    const static int DEAD = 4;

    //// Tree age years -> growth stage
    // SEED = age 0, 1,
    const static int SEED_MIN_AGE = 0;
    // JUVENILE = age 2 - 9
    const static int JUVENILE_MIN_AGE = 2;
    // MATURE = age >= 10
    const static int MATURE_MIN_AGE = 10;

    // TreeMass AKA Heights and biomass values
    constexpr const static float TreeMass[5] = { 0.1, 5.0, 10.0, 7.5, 0.0 };


    ////// CLASS FUNCTIONALITY

    EcoSim();

    void cycle();

    void getTreePositions(std::string& seed_pos, std::string& juvenile_pos,
        std::string& mature_pos, std::string& decay_pos);

    std::vector<Tree> trees;

    // SETTERS
    void setTrees();
    void setVapor(float v);
    void setSoilWater(float s);

    // PRINTERS (for testing)
    void printVapor();
    void printPrecipitation();
    void printSoilWater();
    void printVegNeeds();
    void printBiomass();

protected:

    //// Climatic Processes
    // VAPOR to PRECIPITATION
    void condensation();
    // PRECIPITATION to SOIL WATER
    void soilWaterDiffusion();
    // SOIL WATER to BIOMASS and VEGETATION growth
    void absorption();
    // Updates water requirements for biomass on that cell
    void absorptionReqs();
    // BIOMASS to VAPOR
    void evaporation();

    //// Helper Tree functions
    // Updates tree growth/decay based on soil water
    void vegetationGrowth(Tree& t, int x, int y);
    // Updates the growth stage of tree based on age
    void increaseGrowthStage(Tree& t);
    // Spawns a seed into seedling vector
    void spawnSeed(vec3 parentPos);
    // Pops seedlings into trees vector
    void updateSeeds();


private:

    // Must be set to TRUE for cycle to run
    bool treeSet = false;

    ///// WATER CYCLE MAP VALUES

    // 3D volume of vapor in column
    // assuming information stored, x, y, z (where z is height)
    std::array<std::array<std::array<float, TERRAIN_HEIGHT>, TERRAIN_SIZE>, TERRAIN_SIZE> vapor_values;

    // amount of rain
    float precipitation_values[TERRAIN_SIZE][TERRAIN_SIZE] = { {0.0} };

    // amount of water in the soil
    std::array<std::array<float, TERRAIN_SIZE>, TERRAIN_SIZE> soilWater_values;

    // how much water the plants on that cell need
    float vegetationNeeds_values[TERRAIN_SIZE][TERRAIN_SIZE] = { {0.0} };

    // amount of biomass on the grid cell
    float biomass_values[TERRAIN_SIZE][TERRAIN_SIZE] = { {0.0} };

    // keeps track of the seeds from that new round
    // depletes into trees after growth stage (we don't want seeds to also grow this iteration)
    std::vector<Tree> seedlings;
};
