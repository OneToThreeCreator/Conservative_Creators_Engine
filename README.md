# Conservative creator's engine
2D crossplatform game engine. WIP.
# Building on unix-like system:
Required packages: glfw, cmake.
```
# Change directory to project's directory
cd [Project dir]
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
