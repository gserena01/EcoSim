#include "EcoSim.h"
#include "Texture.h"

EcoSim::EcoSim()
{
    treeID = 0;
    Texture tex = Texture(TERRAIN_SIZE, TERRAIN_HEIGHT);
    vapor_values = tex.tex_arr_3d;
    soilWater_values = tex.tex_arr_2d;
    // trees must be set before you can begin cycle
}

void EcoSim::setVaporManual(float v) {
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            for (int z = 0; z < 4; ++z) {
                vapor_values[x][y][z] = v;
            }
        }
    }
}

void EcoSim::setSoilWaterManual(float s) {
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            soilWater_values[x][y] = s;
        }
    }
}

void EcoSim::setBiomassManual(float s) {
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            biomass_values[x][y] = s;
        }
    }
}

void EcoSim::setTreesManual() {
    // USED FOR TESTING 
    // sets trees vector, biomass map, vegetationNeeds map
    // sets manually created trees for testing
    treeSet = true;

    Tree tr;
    tr.position = vec3(1.0, 1.0, 0.0);
    tr.age = 3;
    tr.growthStage = JUVENILE;
    tr.id = treeID++;
    // populates trees vector
    trees.push_back(tr);

    Tree tr1;
    tr1.position = vec3(2.0, 1.0, 0.0);
    tr1.age = 10;
    tr1.growthStage = MATURE;
    tr1.id = treeID++;
    // populates trees vector
    trees.push_back(tr1);

    // update biomass values
    update_biomass();
    // set vegetationNeeds
    absorptionReqs();
}

void::EcoSim::setTreesNoise() {
    // sets trees vector, biomass map, vegetationNeeds map
    // sets based on soil water noise function

    for (int x = 0; x < TERRAIN_SIZE; x += 2) {
        for (int y = 0; y < TERRAIN_SIZE; y += 2) {
            float w = soilWater_values[x][y];
            Tree t;
            t.position = vec3(x, y, 0.0);
            t.id = treeID++;
            if (w > 0.7 && w < 1.3) {
                t.growthStage = SEED;
                t.age = std::round((w - 0.5));
                trees.push_back(t);
            }
            else if (w >= 2.1 && w < 2.5) {
                t.growthStage = JUVENILE;
                t.age = std::round((w - 2.1) * 17.5 + 2.0);
                trees.push_back(t);
            }
            else if (w >= 2.5) {
                t.growthStage = MATURE;
                t.age = std::round((w - 0.5) * 5.0);
                trees.push_back(t);
            }
        }
    }

    // update biomass values
    update_biomass();
    // set vegetationNeeds
    absorptionReqs();

    treeSet = true;
}

void EcoSim::setTreesString(std::string input, int treeType) {
    // sets trees vector, biomass map, vegetationNeeds map
    // sets based on input string

    // converts the input string into a vector of ints
    std::vector<int> pos = processPosInput(input);

    int x;
    int y;
    for (int i : pos) {
        // convert grid int into x, y indices
        x = i / TERRAIN_SIZE;
        y = i - (TERRAIN_SIZE * x);
        Tree t;
        t.position = vec3(float(x), float(y), 0.0);
        t.id = treeID++;
        t.growthStage = treeType;
        t.age = TreeMinAge[treeType];
        trees.push_back(t);
    }


    // update biomass values
    update_biomass();
    // set vegetationNeeds
    absorptionReqs();

    treeSet = true;

}

void EcoSim::resetVegetation() {
    if (treeSet) {
        // trees have already been set
        trees.clear();
        setBiomassManual(0.0);
        treeSet = false;
    }
}


void EcoSim::cycle()
{
    // check if initial values are set
    if (!treeSet) {
        std::cout << "ERROR: starting TREES not set" << std::endl;
        return;
    }

    // turn VAPOR into PRECIPITATION
    condensation();

    // turn PRECIPITATION into SOIL WATER
    soilWaterDiffusion();

    // turn SOIL WATER into VEGETATION
    absorption();

    // add seeds to Trees and Biomass
    updateSeeds();

    // update the water needs of VEGETATION
    absorptionReqs();

    // turn VEGETATION into VAPOR
    evaporation();

}


std::vector<float> EcoSim::getTreePositions(std::string& seed_pos, std::string& juvenile_pos,
    std::string& mature_pos, std::string& decay_pos) {
    // Updates the given strings with numbers of the new tree positions for display
    // Ex: tree at 0,0 = 0 ; tree at 31,31 = 1023
    // Output strings in the form "0 1 2 3"
    // Returns vector of num of each tree age
    // ie treenum[0] = 2 means there are 2 seed trees

    // empty the strings
    seed_pos = "";
    juvenile_pos = "";
    mature_pos = "";
    decay_pos = "";

    // counters
    float numS = 0;
    float numJ = 0;
    float numM = 0;
    float numD = 0;

    // populate position strings
    for (Tree& t : trees) {
        int pos = int(TERRAIN_SIZE * t.position[1] + t.position[0]);
        if (t.growthStage == SEED) {
            numS += 1.0;
            seed_pos.append(std::to_string(pos));
            seed_pos.push_back(' ');
        }
        else if (t.growthStage == JUVENILE) {
            numJ += 1.0;
            juvenile_pos.append(std::to_string(pos));
            juvenile_pos.push_back(' ');
        }
        else if (t.growthStage == MATURE) {
            numM += 1.0;
            mature_pos.append(std::to_string(pos));
            mature_pos.push_back(' ');
        }
        else if (t.growthStage == DECAY) {
            numD += 1.0;
            decay_pos.append(std::to_string(pos));
            decay_pos.push_back(' ');
        }
    }

    std::vector<float> treenum;
    treenum.push_back(numS);
    treenum.push_back(numJ);
    treenum.push_back(numM);
    treenum.push_back(numD);

    return treenum;
}

std::vector<float> EcoSim::getCloudPositions(std::string& small_pos, std::string& med_pos, std::string& big_pos) {
    // Updates the given strings with numbers of the cloud positions for display
    // Ex: cloud at 0,0 = 0 ; cloud at 31,31 = 1023
    // Output strings in the form "0 1 2 3"
    // Returns vector of num of each cloud size
    // ie clous_num[0] = 2 means there are 2 small clouds

    // empty the strings
    small_pos = "";
    med_pos = "";
    big_pos = "";

    // counters
    float numS = 0;
    float numM = 0;
    float numB = 0;

    // populate position strings
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            int pos = int(TERRAIN_SIZE * y + x);
            if (precipitation_values[x][y] >= small_cloud &&
                precipitation_values[x][y] < med_cloud) {
                numS += 1.0;
                small_pos.append(std::to_string(pos));
                small_pos.push_back(' ');
            }
            else if (precipitation_values[x][y] >= med_cloud &&
                precipitation_values[x][y] < big_cloud) {
                numM += 1.0;
                med_pos.append(std::to_string(pos));
                med_pos.push_back(' ');
            }
            else if (precipitation_values[x][y] >= big_cloud) {
                numB += 1.0;
                big_pos.append(std::to_string(pos));
                big_pos.push_back(' ');
            }
        }
    }

    std::vector<float> cloud_num;
    cloud_num.push_back(numS);
    cloud_num.push_back(numM);
    cloud_num.push_back(numB);

    return cloud_num;
}


//// CLIMATIC PROCESSES

void EcoSim::condensation() {
    // Update precipitation map based on vapor values
    // sum all vapor into precipitation
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            float total_rain = 0.0;
            for (int z = 0; z < TERRAIN_HEIGHT; ++z) {
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
            float updatedSoil = soilWater_values[x][y] + precipitation_values[x][y] - vegetationNeeds_values[x][y] - EVAP_CONSTANT;
            soilWater_values[x][y] = max(updatedSoil, 0.f);
        }
    }
}

void EcoSim::absorption() {
    // Updates all vegetation based on soil water
    
    float new_biomass[TERRAIN_SIZE][TERRAIN_SIZE] = { {0.0} };

    for (Tree& t : trees) {
        if (t.growthStage != DEAD) {
            int x = floor(t.position[0]);
            int y = floor(t.position[1]);
            vegetationGrowth(t, x, y);
            new_biomass[x][y] += TreeMass[t.growthStage];
        }
    }

    // update biomass values with new biomass 
    for (int x = 0; x < TERRAIN_SIZE; ++x) {
        for (int y = 0; y < TERRAIN_SIZE; ++y) {
            biomass_values[x][y] = new_biomass[x][y];
        }
    }

    // remove all DEAD trees
    std::vector<Tree> aliveTrees;
    for (Tree& t : trees) {
        if (t.growthStage != DEAD) {
            aliveTrees.push_back(t);
        }
    }
    trees = aliveTrees;
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
            }
        }
    }
}


//// TREE HELPER FUNCTIONS

void EcoSim::vegetationGrowth(Tree& t, int x, int y) {
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
        }
        else {
            t.growthStage = DECAY;
        }
    }
}

void EcoSim::increaseGrowthStage(Tree& t) {
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
    // Spawns a seeds within a random spot along the radius of the parent tree
    // Called if mature tree has enough water to grow

    Tree babyTree;
    babyTree.age = 0;
    babyTree.growthStage = SEED;
    babyTree.id = treeID++;

    float deltaX = cos(babyTree.id * theta) * seedR;
    float deltaY = sin(babyTree.id * theta) * seedR;

    babyTree.position = vec3(deltaX + parentPos[0], deltaY + parentPos[1], parentPos[2]);

    if ((babyTree.position[0] < 0 || babyTree.position[0] > TERRAIN_SIZE)
        || (babyTree.position[1] < 0 || babyTree.position[1] > TERRAIN_SIZE)) {
        // baby tree is out of range
        return;
    }
    else {
        seedlings.push_back(babyTree);
    }
}

void EcoSim::updateSeeds() {
    // Removes seeds from seedlings vectors and puts in trees vector
    // Updates biomass 

    while (seedlings.size() > 0) {
        Tree& t = seedlings.back();
        seedlings.pop_back();
        trees.push_back(t);

        int x = floor(t.position[0]);
        int y = floor(t.position[1]);
        biomass_values[x][y] += TreeMass[t.growthStage];
    }
}

void EcoSim::update_biomass() {
    // Update biomass values for all trees at any given time 
    for (Tree& t : trees) {
        int x = floor(t.position[0]);
        int y = floor(t.position[1]);
        biomass_values[x][y] += TreeMass[t.growthStage];
    }
}



//// RANDOM HELPER FUNCTIONS

bool EcoSim::isNumber(char l) {
    // Returns true is input char represents a number 0-9
    if (l == '0' ||
        l == '1' ||
        l == '2' ||
        l == '3' ||
        l == '4' ||
        l == '5' ||
        l == '6' ||
        l == '7' ||
        l == '8' ||
        l == '9') {
        return true;
    }
    else {
        return false;
    }
}

void EcoSim::processRange(std::string startRange, std::string endRange, std::vector<int>& pos) {
    // Helper function for process input 
    // pushed back all the numbers given start and end range 
    for (int i = stoi(startRange); i <= stoi(endRange); ++i) {
        pos.push_back(i);
    }
}


std::vector<int> EcoSim::processPosInput(std::string input) {
    // converts the input string into a vector of ints
    // ie "1 2 3" into {1, 2, 3}
    // also handles weird spacing and ranges
    // ie "   1 , 2, 4-6  " into {1, 2, 4, 5, 6}

    input.push_back(' ');

    std::vector<int> pos;
    std::string curPos;
    std::string startRange;
    bool inRange = false;
    for (int i = 0; i < input.size(); ++i) {
        char l = input[i];
        if (isNumber(l)) {
            curPos.push_back(l);
        }
        else if (l == '-') {
            startRange = curPos;
            curPos = "";
            inRange = true;
        }
        else {
            if (curPos.size() != 0) {
                if (!inRange) {
                    pos.push_back(stoi(curPos));
                    curPos = "";
                }
                else {
                    processRange(startRange, curPos, pos);
                    curPos = "";
                    startRange = "";
                    inRange = false;
                }
            }
        }
    }
    return pos;
}


//// PRINTERS
// used for testing 

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
