#include "EcoSim.h"

EcoSim::EcoSim()
{
    treeID = 0;
    // Default constructor
    // no empty trees, vapor is garbage
    // Precip map is 0.0
    // Soil map is 1.0
    setSoilWater(1.0);
    // Biomass map is 0.0
}

void EcoSim::setVapor(float v) {
    // TODO: pull from noise vapor map
    // sets vapor map
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            for (int z = 0; z < TERRAIN_SIZE; ++z) {
                vapor_values[x][y][z] = v;
            }
        }
    }
}

void EcoSim::setSoilWater(float s) {
    // TODO: set with a noise function?? or a better default?
    // sets soil water map
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            soilWater_values[x][y] = s;
        }
    }
}

void EcoSim::setTrees() {
    // TODO: pull from existing starter trees on terrain
    // sets trees vector, biomass map, vegetationNeeds map

    Tree tr;
    tr.position = vec3(1.0, 1.0, 0.0);
    tr.age = 3;
    tr.growthStage = JUVENILE;
    tr.id = treeID++;
    // populates trees vector
    trees.push_back(tr);

    // update biomass values
    for (Tree &t : trees) {
        int x = floor(t.position[0]);
        int y = floor(t.position[1]);
        biomass_values[x][y] += TreeMass[t.growthStage];
    }
    // set vegetationNeeds
    absorptionReqs(); // set vegetationNeeds
}

void EcoSim::setTreesDecayTest() {
    // sets trees vector, biomass map, vegetationNeeds map

    Tree tr;
    tr.position = vec3(1.0, 1.0, 0.0);
    tr.age = 5;
    tr.growthStage = MATURE;

    Tree tr1;
    tr1.position = vec3(1.0, 1.0, 0.0);
    tr1.age = 5;
    tr1.growthStage = MATURE;

    Tree tr2;
    tr2.position = vec3(1.0, 1.0, 0.0);
    tr2.age = 5;
    tr2.growthStage = MATURE;

    Tree tr3;
    tr3.position = vec3(2.0, 2.0, 0.0);
    tr3.age = 2;
    tr3.growthStage = JUVENILE;
    // populates trees vector
    trees.push_back(tr);
    trees.push_back(tr1);
    trees.push_back(tr2);
    trees.push_back(tr3);

    // update biomass values
    for (Tree t : trees) {
        int x = floor(t.position[0]);
        int y = floor(t.position[1]);
        biomass_values[x][y] += TreeMass[t.growthStage];
    }
    // set vegetationNeeds
    absorptionReqs(); // set vegetationNeeds
}


void EcoSim::cycle()
{
    // turn VAPOR into PRECIPITATION
    condensation(); // updates precipitation map

    // turn PRECIPITATION into SOIL WATER
    soilWaterDiffusion(); // updates soil water map

    // turn SOIL WATER into VEGETATION 
    absorption();

    // Add seedlings to Trees and Biomass
    updateSeeds();

    // update the water needs of VEGETATION
    absorptionReqs();

    // turn VEGETATION into EVAPORATION
    evaporation();

    std::cout << getTreeSize() << std::endl;
}

//// CLIMATIC PROCESSES

void EcoSim::condensation() {
    // Update precipitation map based on vapor values
    // sum all vapor into precipitation
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            float total_rain = 0.0;
            for (int z = 0; z < TERRAIN_SIZE; ++z) {
                total_rain += vapor_values[x][y][z];
            }
            precipitation_values[x][y] = total_rain;
        }
    }

}

void EcoSim::soilWaterDiffusion() {
    // Updates soilWater_values map based on precipitation
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            soilWater_values[x][y] += precipitation_values[x][y] - vegetationNeeds_values[x][y] - EVAP_CONSTANT;
        }
    }
}

void EcoSim::absorption() {

    // updates all vegetation based on soil water
    float new_biomass[TERRAIN_SIZE][TERRAIN_SIZE] = { {0.0} };

    for (Tree &t : trees) {
        // TODO: find a way to remove DEAD trees (Subtask 6.1)
        if (t.growthStage != DEAD) {
            int x = floor(t.position[0]);
            int y = floor(t.position[1]);
            vegetationGrowth(t, x, y);
            //displayTree(t); // TODO: Subtask 3.1
            new_biomass[x][y] += TreeMass[t.growthStage];
        }
    }

    // update biomass values
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            biomass_values[x][y] = new_biomass[x][y];
        }
    }
}

void EcoSim::absorptionReqs() {
    // Set the amount of water required for the vegetation on a grid cell
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            vegetationNeeds_values[x][y] = ABSORB_WET_CLIMATE * biomass_values[x][y];
        }
    }

}

void EcoSim::evaporation() {
    // Updates vapor map based on vegetaion biomass
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            for (int z = 0; z < TERRAIN_SIZE; ++z) {
                vapor_values[x][y][z] = TRANSPIRATION * (1.0 / (z + 1.0)) * biomass_values[x][y];
                // TODO: apply noise (Gaussian), Subtask 6.1
            }
        }
    }
}


//// TREE HELPER FUNCTIONS

void EcoSim::vegetationGrowth(Tree &t, int x, int y) {
    // Update vegetation based on soil water 
    if (soilWater_values[x][y] >= vegetationNeeds_values[x][y]) {
        // GROW
        t.age += 1;
        increaseGrowthStage(t);
    }
    else {
        // DECAY 
        if (t.growthStage != MATURE) {
            // SEED, JUVENILE, and DECAYing trees die
            t.growthStage = DEAD;
            std::cout << "TREE DIED! ID: " << t.id << std::endl;
        }
        else {
            t.growthStage = DECAY;
        }
    }
}

void EcoSim::increaseGrowthStage(Tree &t) {
    // update growth stage in tree struct
    // tree was decaying
    if (t.growthStage == DECAY) {
        t.growthStage = MATURE;
    }
    else if (t.age < JUVENILE_MIN_AGE) {
        t.growthStage = SEED;
    }
    else if (t.age < MATURE_MIN_AGE) {
        t.growthStage = JUVENILE;
    }
    else if (t.age == MATURE_MIN_AGE) { // just became MATURE
        t.growthStage = MATURE;
    }
    else if (t.age > MATURE_MIN_AGE) { // still MATURE
        spawnSeed(t.position);
    }
}

void EcoSim::spawnSeed(vec3 parentPos) {
    Tree babyTree;
    babyTree.age = 0;
    babyTree.growthStage = SEED;
    babyTree.id = treeID++;
    // TODO: connect to particle node????

    //float deltaX = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    //float deltaY = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    float deltaX = cos(babyTree.id * theta) * seedR;
    float deltaY = sin(babyTree.id * theta) * seedR;

    // TODO: z value needs to be from height field Subtask 4.3
    babyTree.position = vec3(deltaX + parentPos[0], deltaY + parentPos[1], parentPos[2]);
    //babyTree.position = vec3(parentPos[0] + 1.0, parentPos[1] + 1.0, 0.0);


    seedlings.push_back(babyTree);
}

void EcoSim::updateSeeds() {
    while (seedlings.size() > 0) {
        Tree &t = seedlings.back();
        seedlings.pop_back();
        trees.push_back(t);

        int x = floor(t.position[0]);
        int y = floor(t.position[1]);
        biomass_values[x][y] += TreeMass[t.growthStage];
    }
}

void EcoSim::displayTree(Tree& t) {
    // TODO

}


//// GETTERS & PRINTERS
int EcoSim::getTreeSize() {
    int aliveT = 0;
    for (Tree& t : trees) {
        if (t.growthStage != DEAD) {
            aliveT++;
        }
    }
    return aliveT;
}

void EcoSim::printVapor() {
    for (int z = 0; z < TERRAIN_SIZE; ++z) {
        for (int x = 0; x < TERRAIN_SIZE; ++x) {
            for (int y = 0; y < TERRAIN_SIZE; ++y) {
                std::cout << vapor_values[x][y][z] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void EcoSim::printPrecipitation() {
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            std::cout << precipitation_values[x][y] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void EcoSim::printSoilWater() {
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            std::cout << soilWater_values[x][y] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void EcoSim::printVegNeeds() {
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            std::cout << vegetationNeeds_values[x][y] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void EcoSim::printBiomass() {
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            std::cout << biomass_values[x][y] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
