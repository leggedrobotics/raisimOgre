cmake_minimum_required(VERSION 3.5)
project(raisimOgre_examples VERSION 0.2.0)

find_package(raisimOgre CONFIG REQUIRED)
find_package(Eigen3 REQUIRED HINTS ${Eigen3_HINT})

#==
# Build Targets
#==

function(create_executable app_name file_name)
  add_executable(${app_name} ${file_name})
  target_link_libraries(${app_name} PUBLIC raisim::raisimOgre)
  target_include_directories(${app_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${EIGEN3_INCLUDE_DIRS})
  target_compile_definitions(${app_name} PRIVATE "-DEXAMPLE_ROBOT_RESOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rsc/")
endfunction()

# robots
create_executable(anymal src/robots/anymal.cpp)
create_executable(laikago src/robots/laikago.cpp)
create_executable(kinova src/robots/kinova.cpp)
create_executable(atlas src/robots/atlas.cpp)
create_executable(anymalOnHeightMap src/robots/heightMap.cpp)
create_executable(chain src/robots/chain.cpp)
create_executable(aliengo src/robots/aliengo.cpp)
create_executable(shunk src/robots/shunk.cpp)
create_executable(creatingRobotUsingCPP src/robots/creatingRobotUsingCPP.cpp)

# remote
create_executable(remoteVisualzation_server src/remote/server.cpp)
create_executable(remoteVisualzation_client src/remote/client.cpp)

# primitives
create_executable(zurichHeightMap src/primitives/heightMapFromPng.cpp)
create_executable(newtonsCradle src/primitives/newtonsCradle.cpp)
create_executable(primitives src/primitives/primitives.cpp)

# meshes
create_executable(meshes src/mesh/meshes.cpp)

# visual objects
create_executable(visualObjects src/visualObjects/visualObjects.cpp)

#==
# Install example binaries
#==

set(EXAMPLE_BINS
  anymal
  laikago
  anymalOnHeightMap
  primitives
  kinova
  atlas
  remoteVisualzation_server
  remoteVisualzation_client
  newtonsCradle
)

install(
  TARGETS ${EXAMPLE_BINS}
  RUNTIME DESTINATION bin
)

# EOF
