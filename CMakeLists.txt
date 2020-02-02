cmake_minimum_required(VERSION 3.14)
project(game_of_life)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -std=c++11 -march=native")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -Wall -std=c++11 -fsanitize=address -fsanitize=undefined")

find_package(OpenMP REQUIRED)
find_package(FLTK REQUIRED)

add_executable(game_of_life src/main.cpp src/fields/GameField.cpp src/fields/GameField.h src/Presets.h src/fields/SimpleMatrix.h src/GameWidget.h src/FieldBenchmark.cpp src/FieldBenchmark.h)

target_link_libraries(game_of_life PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(game_of_life PUBLIC fltk)