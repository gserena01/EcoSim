#include "EcoSim.h"

EcoSim::EcoSim() 
{
    // Default constructor

    // amount of water in the soil
    float[TERRAIN_SIZE][TERRAIN_SIZE] soilWater_values = {1.0};

}

void EcoSim::setVapor() {
    // TODO: pull from noise vapor map
}

void EcoSim::setTrees() {
    // TODO: pull from existing starter trees on terrain
    // populates trees vector
    // update biomass values

    // set vegetationNeeds
    absorption(); // set vegetationNeeds
}


void EcoSim::cycle() 
{
	// turn VAPOR into PRECIPITATION
	condensation(); // updates precipitation map
			
	// turn PRECIPITATION into SOIL WATER
	soilWaterDiffusion(); // updates soil water map

	// turn SOIL WATER into VEGETATION 
	float[TERRAIN_SIZE][TERRAIN_SIZE] new_biomass_values = {0.0};
	for(Tree t : trees) {
		int x = floor(t.position[0]);
		int y = floor(t.position[1]);
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

//// CLIMATIC PROCESSES

void EcoSim::condensation() {
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

void EcoSim::soilWaterDiffusion() {
	// Updates soilWater_values map based on precipitation
	for(int x = 0; x < TERRAIN_SIZE; ++x) {
		for(int y = 0; y < TERRAIN_SIZE; ++y) {
			soilWater_values[x][y] += precipitation_values[x][y] - vegetationNeeds_values[x][y] - EVAP_CONSTANT;
		}
	}
}

float EcoSim::absorption() {
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
			for(int z = 0; z < TERRAIN_SIZE; ++z) {
				vapor_values[x][y][z] = TRANSPIRATION * (1.0 / z) * biomass_values[x][y];
				// TODO: apply noise (Gaussian), Subtask 6.1
			}
		}
	}
}


//// TREE HELPER FUNCTIONS

void EcoSim::vegetationGrowth(Tree t, int x, int y) {
	// Update vegetation based on soil water 
	if(soilWater_values[x, y] >= vegetationNeeds_values[x][y]) {
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

void EcoSim::updateGrowthStage(Tree t) {
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

void EcoSim::spawnSeed(vec3 parentPos) {
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