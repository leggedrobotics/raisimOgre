# raisimOGRE: Visualizer for raisim

# THIS LIBRARY DEPENDS ON RAISIM WHICH IS NOT PUBLICLY AVAILABLE YET. RAISIM WILL BE RELEASED VERY SOON #

## Requirements
- Linux only. support ubuntu 16.04 and 18.04 but might work on other distributions
- g++-6 or higher

## Install

Please install/save everything locally to prevent corrupting your system files. We will assume that you have a single workspace where you save all repos related to raisim. Here we introduce two variables

- WORKSPACE: workspace where you clone your git repos
- LOCAL_BUILD: build directory where you install exported cmake libraries

### Dependencies
If you have g++ >= 6.0 installed, You can switch your active compiler by
```commandline
export CXX=/usr/bin/g++-8 && export CC=/usr/bin/gcc-8
```

First, install raisimLib (https://github.com/leggedrobotics/raisimLib).

Then, install dependencies of Ogre. Make sure that you install it locally since otherwise it will overwrite your local ogre installation.
```commandline
sudo apt-get install libgles2-mesa-dev libxt-dev libxaw7-dev libsdl2-dev libzzip-dev
```

Now build Ogre from source.
```commandline
cd WORKSPACE
git clone https://github.com/OGRECave/ogre.git
cd ogre
git checkout tags/v1.11.5 -b r1.11.5
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=LOCAL_BUILD
make install -j
```

Copy built dependencies to local folder 
```commandline
cp -R Dependencies/* LOCAL_BUILD
```

Next, build Assimp from source
```commandline
cd WORKSPACE
git clone https://github.com/assimp/assimp.git
cd assimp && mkdir build && cd build
cmake .. -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX=LOCAL_BUILD
make install -j
```

### raisimOgre
Finally, build raisimOgre.
```commandline
cd WORKSPACE/raisimOgre
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=LOCAL_BUILD -DCMAKE_INSTALL_PREFIX=LOCAL_BUILD
make install -j
```

### Optional Dependencies
- ffmpeg (video recording, for OgreVis::startRecordingVideo method. The install struction can be found at https://tecadmin.net/install-ffmpeg-on-linux/)

## Examples
ANYmal | ANYmal on a heightmap  
:-----------------------------------:|:------------------------------------:
![alt-text-1](img/anymal.gif "title-1") | ![alt-text-2](img/heightmap.gif "title-2")

Laikago   | Primitive shapes  
:-----------------------------------:|:------------------------------------:
![alt-text-1](img/laikago.gif "title-1") | ![alt-text-2](img/primitives.gif "title-2")

## OGRE Resources
1. Basic RaisimOgre resources are defined by the `RAISIM_OGRE_RESOURCE_DIR` macro.
2. Ogre resources are loaded from the resource file, whose location is defined by the `OGRE_CONFIG_DIR` macro.
3. The two aforementioned definitions are defined in the config file.
