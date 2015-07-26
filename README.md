# MAXR

## Introduction

Mechanized Assault & eXploration Reloaded

A turn based strategy game based on M.A.X. created by fans as a community to make it's unique multiplayer experience possible on modern networks and systems. It is licensed under the GPL v2 and CC BY-SA 3.0 and is as such a free open source game written for the operating systems Linux, Mac and Windows.

## Licence

MAXR is dual licensed. While the game itself, including source and most assets, are released under the GPL v2 some files are not.

For GPL v2 see `data/COPYING`

For files unter CC BY-SA 3.0 see `data/COPYING.README `

# Building MAXR

## Supported Platforms

* Mac OS X
* MS Windows
* Linux

Other UNIX-like operating systems may work too out of the box. Chances are huge if there's a build of SDL2 and CMake for the system.

## Building MAXR with CMake

MAXR is distributed with a `CMakeLists.txt` file for CMake.
Consequently, if you are building from source, you will have to
set up your Makefiles with `cmake`, before you can run `make` 
from the projects root folder:

```shell
mkdir build
cd build
cmake ..
make
```

### Change between Debug and Release build:

Use the options `-DCMAKE_BUILD_TYPE=Debug` or `-DCMAKE_BUILD_TYPE=Release` when running `cmake`:

`cmake -DCMAKE_BUILD_TYPE=Release ..`

### Build the dedicated server of maxr:

Use the options `-DMAXR_BUILD_DEDICATED_SERVER=ON` in another
cmake build folder, because the binary will overwrite any
existing maxr binary. From the projects root folder:

```shell
mkdir build-dedicated
cd build-dedicated
cmake --DMAXR_BUILD_DEDICATED_SERVER=ON ..
```

### Build and install MAXR on Linux

Compiles the maxr binary and installs it along with the game data to the default prefix which is probably /usr/local.

```shell
mkdir build
cd build
cmake ..
make install
```

After installing you can start maxr executing 'maxr'

Use `make DESTDIR=/foo/bar install` to make use of destdir.

If you just want to start maxr without installing simply
override the data path MAXRDATA after running `make` in your
build dir: `MAXRDATA=../cp data/ ./maxr`

## Additional important files

Please find additional files in `./data`:

ABOUT `data/ABOUT`

AUTHORS `data/AUTHORS`

INSTALL `data/INSTALL`

MANUAL `data/MANUAL`
