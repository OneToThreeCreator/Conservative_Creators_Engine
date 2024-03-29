# Conservative creator's engine
2D crossplatform game engine written in C. Very fast, but also very limited in functionality. WIP

# Building on unix-like system (Linux, probably MacOS):
Required packages: cmake, python.  
Can be built against packages: glfw (-dev or -devel on some distributions for header files), inih (-dev or -devel) (if any of this packages are not present, you need to initialize appropriate submodules instead)
```
# Change directory to project's directory
cd [Project dir]
# Clone necessary submodules
git submodule update --init external/listlib/ external/glad2/
# If inih is not present
git submodule update --init external/inih/
# If glfw is not present
git submodule update --init external/glfw/
# Build
cmake -B ./build && cmake --build ./build
```
If glfw is used as submodule, it's dependencies must be installed or it wont build.
Install:
```
cmake --install ./build
```
Run test:
```
ctest --test-dir ./build
```

# Building on Windows:
Required programs: cmake, python, git, any compiler (be it gcc, clang, msvc...)  
Windows SDK must be installed (From Windows 7 up to Windows 11). You can install it from visual studio or separately.
```
# Initialize submodules
git submodule update --init
# Build
cmake -B ./build && cmake --build ./build
```
Install:
```
cmake --install ./build
```
Run test:
```
ctest --test-dir ./build
```

# License
Conservative creator's engine is licensed under the GNU LGPL v2 or any later version, with some files under GNU All-Permissive License (check license notice at the head of individual file)
