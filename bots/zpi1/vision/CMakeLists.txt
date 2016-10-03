cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

include(ZuPiCommon)

project(zpi1-vision CXX)

find_library(COMMON_LIB zupi)
find_package(OpenCV REQUIRED)
find_library(AVDEVICE_LIB avdevice)
find_library(AVFILTER_LIB avfilter)
find_library(AVFORMAT_LIB avformat)
find_library(AVCODEC_LIB avcodec)
find_library(SWSCALE_LIB swscale)
find_library(SWRESAMPLE_LIB swresample)
find_library(AVUTIL_LIB avutil)

add_executable(zpi1-vision
    src/model.cpp
    src/vision.cpp
)

target_link_libraries(zpi1-vision -Wl,--start-group
    ${COMMON_LIB}
    ${OpenCV_LIBS}
    ${AVDEVICE_LIB}
    ${AVFILTER_LIB}
    ${AVFORMAT_LIB}
    ${AVCODEC_LIB}
    ${SWSCALE_LIB}
    ${SWRESAMPLE_LIB}
    ${AVUTIL_LIB}
    -Wl,--end-group)
