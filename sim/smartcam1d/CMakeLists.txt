cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

include(ZuPiCommon)

project(smartcam1d CXX)

find_library(COMMON_LIB zupi)
find_library(FIDO_LIB fido)

add_executable(smartcam1d
    src/app.cpp
    src/timeline.cpp
    src/camera1d.cpp
    src/servo.cpp
    src/capture.cpp
    src/train.cpp
    src/simulate.cpp
    src/algo_fido.cpp
)
target_link_libraries(smartcam1d ${COMMON_LIB} ${FIDO_LIB} pthread)
