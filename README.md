# Conservative creator's engine
2D crossplatform game engine written in C. Very fast, but also very restricted in functionality. WIP

# Building on unix-like system (Linux, probably MacOS):
Required packages: cmake, python.  
Can be built against packages: glfw (-dev or -devel on some distributions for header files), inih (-dev or -devel) (if any of this packages are not present, you need to initialize appropriate submodules instead)
```
# Change directory to project's directory
cd [Project dir]
# Clone necessary submodules
git submodule update --init external/listlib/ external/glad2/
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

# Building on Windows:
Required programs: cmake, python, git, any compiler (be it gcc, clang, msvc...)
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
