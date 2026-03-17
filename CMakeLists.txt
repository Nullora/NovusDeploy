cmake_minimum_required(VERSION 3.10)
project(NovusDeploy)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(ndep
    src/main.cpp
)