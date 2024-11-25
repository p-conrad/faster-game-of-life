cmake_minimum_required(VERSION 3.14)
project(game_of_life)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Ofast -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Og -fsanitize=address -fsanitize=undefined")

find_package(OpenMP REQUIRED)
find_package(FLTK REQUIRED)

add_executable(game_of_life src/main.cpp src/GameField.cpp src/GameField.h src/Presets.h src/SimpleMatrix.h src/GameWidget.h src/FieldBenchmark.cpp src/FieldBenchmark.h src/accessors.h)

target_link_libraries(game_of_life PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(game_of_life PUBLIC fltk)
