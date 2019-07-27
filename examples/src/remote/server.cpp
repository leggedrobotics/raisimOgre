//
// Created by Jemin Hwangbo on 3/3/19.
// MIT License
//
// Copyright (c) 2019-2019 Robotic Systems Lab, ETH Zurich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "raisim/World.hpp"
#include "raisim/RaisimServer.hpp"
#include <thread>
#include "helper.hpp"

int main() {

  raisim::World world;
  world.setTimeStep(0.002);
  world.setERP(world.getTimeStep() * .1, world.getTimeStep() * .1);

  auto anymal = world.addArticulatedSystem(raisim::loadResource("anymal/anymal.urdf"));
  anymal->setName("anymal");

//  auto ground = world.addGround(0);
//  ground->setName("ground");

  auto box = world.addBox(1, 2, 1, 1);
  box->setPosition(0, 5, 20);

  auto sph = world.addSphere(1, 2);
  sph->setPosition(2, 5, 50);

  auto cyl = world.addCylinder(1, 2, 1);
  cyl->setPosition(3, 5, 60);

  /// create heightmap
  raisim::TerrainProperties terrainProperties;
  terrainProperties.frequency = 0.2;
  terrainProperties.zScale = 3.0;
  terrainProperties.xSize = 20.0;
  terrainProperties.ySize = 20.0;
  terrainProperties.xSamples = 50;
  terrainProperties.ySamples = 50;
  terrainProperties.fractalOctaves = 3;
  terrainProperties.fractalLacunarity = 2.0;
  terrainProperties.fractalGain = 0.25;
  auto hm = world.addHeightMap(0.0, 0.0, terrainProperties);

  anymal->setGeneralizedCoordinate({0, 1, 30.54, 1.0, 0.0, 0.0, 0.0, 0.03, 0.4, -0.8, -0.03, 0.4, -0.8, 0.03, -0.4, 0.8,
                                    -0.03, -0.4, 0.8});
  anymal->setControlMode(raisim::ControlMode::PD_PLUS_FEEDFORWARD_TORQUE);
  /// ANYmal joint PD controller
  Eigen::VectorXd jointPgain(18), jointDgain(18);
  jointPgain.setZero();
  jointDgain.setZero();
  jointPgain.tail(12).setConstant(200.0);
  jointDgain.tail(12).setConstant(10.0);
  anymal->setPdGains(jointPgain, jointDgain);
  
  raisim::RaisimServer server(&world);
  server.launchServer();

  static size_t controlDecimation = 0;

  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0.0, 0.2);
  std::srand(std::time(nullptr));

  /// ANYmal joint PD controller
  Eigen::VectorXd jointNominalConfig(19), jointVelocityTarget(18);
  jointVelocityTarget.setZero();

  jointNominalConfig << 0, 0, 0, 1, 0, 0, 0, 0.03, 0.7, -1.4, -0.03, 0.7, -1.4, 0.03, -0.7, 1.4, -0.03, -0.7, 1.4;

  anymal->setPdTarget(jointNominalConfig, jointVelocityTarget);

  for(int i=0; i<20000; i++) {
    raisim::MSLEEP(2);
    server.integrateWorldThreadSafe();
    if(i % 50 != 0)
      continue;
    
    /// ANYmal joint PD controller
    Eigen::VectorXd jointNominalConfig(19), jointVelocityTarget(18);
    jointVelocityTarget.setZero();

    jointNominalConfig << 0, 0, 0, 1, 0, 0, 0, 0.03, 0.7, -1.4, -0.03, 0.7, -1.4, 0.03, -0.7, 1.4, -0.03, -0.7, 1.4;

    for (size_t k = 0; k < anymal->getGeneralizedCoordinateDim(); k++)
      jointNominalConfig(k) += distribution(generator);

    anymal->setPdTarget(jointNominalConfig, jointVelocityTarget);
  }

//  server.killServer();
}
