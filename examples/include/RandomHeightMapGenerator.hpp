//
// Created by jemin on 2/25/20.
//

#ifndef _RAISIM_GYM_ANYMAL_RAISIMGYM_ENV_ANYMAL_ENV_RANDOMHEIGHTMAPGENERATOR_HPP_
#define _RAISIM_GYM_ANYMAL_RAISIMGYM_ENV_ANYMAL_ENV_RANDOMHEIGHTMAPGENERATOR_HPP_

#include "raisim/World.hpp"

namespace raisim {

class RandomHeightMapGenerator {
 public:

  enum class GroundType : int {
    HEIGHT_MAP = 0,
    HEIGHT_MAP_DISCRETE = 1,
    STEPS = 2,
    STAIRS = 3
  };

  RandomHeightMapGenerator() = default;
  
  void setSeed(int seed) {
    terrain_seed_ = seed;
  }

  raisim::HeightMap* generateTerrain(raisim::World* world,
      GroundType groundType,
      double curriculumFactor,
      bool createHoles,
      std::mt19937& gen) {
    std::vector<double> heightVec;
    heightVec.resize(heightMapSampleSize_*heightMapSampleSize_);
    std::unique_ptr<raisim::TerrainGenerator> genPtr;
    double targetRoughness = 1.;
    std::uniform_real_distribution<double> uniDist(0., 1.);

    switch (groundType) {
      case GroundType::HEIGHT_MAP:
        terrainProperties_.frequency = 0.8;
        terrainProperties_.zScale = targetRoughness * curriculumFactor;
        terrainProperties_.xSize = 8.0;
        terrainProperties_.ySize = 8.0;
        terrainProperties_.xSamples = 250;
        terrainProperties_.ySamples = 250;
        terrainProperties_.fractalOctaves = 5;
        terrainProperties_.fractalLacunarity = 3.0;
        terrainProperties_.fractalGain = 0.45;
        terrainProperties_.seed = terrain_seed_++;
        terrainProperties_.stepSize = 0.;
        genPtr = std::make_unique<raisim::TerrainGenerator>(terrainProperties_);
        heightVec = genPtr->generatePerlinFractalTerrain();

        if(createHoles) {
          for(int i=0; i<90*curriculumFactor* curriculumFactor* curriculumFactor; i++) {
            std::uniform_int_distribution<> dis(0, 3600-1);
            heightVec[dis(gen)] -= 0.5;
          }
        }

        return world->addHeightMap(250, 250, 18.0, 18.0, 0., 0., heightVec);
        break;

      case GroundType::HEIGHT_MAP_DISCRETE:
        terrainProperties_.frequency = 0.3;
        terrainProperties_.zScale = targetRoughness * curriculumFactor * 1.2;
        terrainProperties_.xSize = 8.0;
        terrainProperties_.ySize = 8.0;
        terrainProperties_.xSamples = 80;
        terrainProperties_.ySamples = 80;
        terrainProperties_.fractalOctaves = 3;
        terrainProperties_.fractalLacunarity = 3.0;
        terrainProperties_.fractalGain = 0.45;
        terrainProperties_.seed = terrain_seed_++;
        terrainProperties_.stepSize = 0.1 * curriculumFactor;
        genPtr = std::make_unique<raisim::TerrainGenerator>(terrainProperties_);
        heightVec = genPtr->generatePerlinFractalTerrain();

        if(createHoles) {
          for(int i=0; i<130*curriculumFactor * curriculumFactor * curriculumFactor; i++) {
            std::uniform_int_distribution<> dis(0, 6400-1);
            heightVec[dis(gen)] -= 0.5;
          }
        }

        return world->addHeightMap(80, 80, 8.0, 8.0, 0., 0., heightVec);
        break;

      case GroundType::STEPS:
        heightVec.resize(120*120);
        for(int xBlock = 0; xBlock < 15; xBlock++) {
          for(int yBlock = 0; yBlock < 15; yBlock++) {
            double height = 0.1 * uniDist(gen) * curriculumFactor;
            for(int i=0; i<8; i++) {
              for(int j=0; j<8; j++) {
                heightVec[120 * (8*xBlock+i) + (8*yBlock+j)] = height + xBlock * 0.15 * curriculumFactor;
              }
            }
          }
        }

        if(createHoles) {
          for(int i=0; i<5 * curriculumFactor * curriculumFactor * curriculumFactor; i++) {
            std::uniform_int_distribution<> dis(0, 120 - 1);
            int row = dis(gen), col = dis(gen);
            for(int j=0; j<120; j++) {
              heightVec[row * 120 + j] -= 0.5;
            }

            for(int j=0; j<120; j++) {
              heightVec[col + 120 * j] -= 0.5;
            }
          }
        }

        return world->addHeightMap(120, 120, 8.0, 8.0, 0., 0., heightVec);
        break;

      case GroundType::STAIRS:
        heightVec.resize(200*200);
        for(int xBlock = 0; xBlock < 25; xBlock++) {
          for(int i=0; i<200*200/25; i++) {
            heightVec[xBlock*200*200/25 + i] = xBlock * 0.17 * curriculumFactor;
          }
        }
        return world->addHeightMap(200, 200, 7.25, 7.25, 0., 0., heightVec);
        break;

    }
    return nullptr;
  }

 private:
  raisim::TerrainProperties terrainProperties_;
  int heightMapSampleSize_ = 120;
  int terrain_seed_;

};

}

#endif //_RAISIM_GYM_ANYMAL_RAISIMGYM_ENV_ANYMAL_ENV_RANDOMHEIGHTMAPGENERATOR_HPP_
