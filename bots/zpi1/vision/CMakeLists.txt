cmake_minimum_required(VERSION 3.0)
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.14.26.tar.gz"
    SHA1 "45ac03944db2bc7fc55dfceca42d89ec7fa1dca2"
    LOCAL
)

project(zpi1-vision)

hunter_add_package(OpenCV)
find_package(OpenCV REQUIRED)

include_directories("inc")
add_executable(zpi1-vision src/analyzer.cpp)
target_link_libraries(zpi1-vision PRIVATE ${OpenCV_LIBS})
