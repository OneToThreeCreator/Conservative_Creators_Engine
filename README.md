# CoffeeChain
2D crossplatform game engine. WIP.
# Building on unix-like system:
Required packages: glfw, cmake.

cd [Project dir]

cmake -B ./build && cmake --build ./build

Install:

cmake --install ./build

Run test:

ctest --test-dir ./build
