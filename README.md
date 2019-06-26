# raisimOGRE: Visualizer for raisim

# THIS LIBRARY DEPENDS ON RAISIM WHICH IS NOT PUBLICLY AVAILABLE YET. RAISIM WILL BE RELEASED VERY SOON #

## Requirements
- Linux only. support ubuntu 16.04 and 18.04 but might work on other distributions
- g++-6 or higher

## Install

Please install/save everything locally to prevent corrupting your system files. We will assume that you have a single workspace where you save all repos related to raisim. Here we introduce two variables

- WORKSPACE: workspace where you clone your git repos
- LOCAL_BUILD: build directory where you install exported cmake libraries

To link against shared libraries in LOCAL_BUILD, you have to let LDD know where the libraries are installed. This can be done adding the following line to your ```~/bashrc```

```commandline
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LOCAL_BUILD/lib
```

If you are using an IDE, ensure that it loads your bashrc. 

### Dependencies
If you have g++ >= 6.0 installed, you can switch your active compiler by
```commandline
export CXX=/usr/bin/g++-8 && export CC=/usr/bin/gcc-8
```

First, install raisimLib (https://github.com/leggedrobotics/raisimLib).

Then, install dependencies of Ogre.
```commandline
sudo apt-get install libgles2-mesa-dev libxt-dev libxaw7-dev libsdl2-dev libzzip-dev
```

Now build Ogre from source. Make sure that you install it locally since otherwise it will overwrite your local ogre installation.
```commandline
cd $WORKSPACE
git clone https://github.com/OGRECave/ogre.git
cd ogre
git checkout tags/v1.11.5 -b r1.11.5
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$LOCAL_BUILD
make install -j8
```

Copy built dependencies to local folder 
```commandline
cp -R Dependencies/* $LOCAL_BUILD
```

Next, build Assimp from source
```commandline
cd $WORKSPACE
git clone https://github.com/assimp/assimp.git
cd assimp && mkdir build && cd build
cmake .. -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX=$LOCAL_BUILD
make install -j8
```

### raisimOgre
Finally, build raisimOgre.
```commandline
cd $WORKSPACE
git clone https://github.com/leggedrobotics/raisimOgre.git
cd raisimOgre && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$LOCAL_BUILD -DCMAKE_INSTALL_PREFIX=$LOCAL_BUILD
make install -j8
```

### Optional Dependencies
- ffmpeg (video recording, for OgreVis::startRecordingVideo method. The install instruction can be found at https://tecadmin.net/install-ffmpeg-on-linux/)

## Examples

![1](img/heightmap.gif "title-2")
![1](img/laikago.gif "title-1")
![alt-text-2](img/primitives.gif "title-2")
![alt-text-1](img/newton.gif "title-1")

## OGRE Resources
1. Basic RaisimOgre resources are defined by the `RAISIM_OGRE_RESOURCE_DIR` macro.
2. Ogre resources are loaded from the resource file, whose location is defined by the `OGRE_CONFIG_DIR` macro.
3. The two aforementioned definitions are defined in the config file.

## How to contribute?
Please fork the repo, make changes and then send a pull request. Instructions can be found [here](https://help.github.com/en/articles/creating-a-pull-request-from-a-fork)

## Available materials
Check rsc/material for a few examples of pbr materials. 
Basic color materials can be found [here](https://www.rapidtables.com/web/color/RGB_Color.html). Replace spaces in the name of the color by "_", e.g., "dark red" to "dark_red"

