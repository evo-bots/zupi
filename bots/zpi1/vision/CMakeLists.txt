cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

project(zpi1-vision CXX)

set(BUILD_SHARED_LIBRARIES OFF)

find_package(OpenCV REQUIRED)
find_library(AVDEVICE_LIB avdevice)
find_library(AVFILTER_LIB avfilter)
find_library(AVFORMAT_LIB avformat)
find_library(AVCODEC_LIB avcodec)
find_library(SWSCALE_LIB swscale)
find_library(SWRESAMPLE_LIB swresample)
find_library(AVUTIL_LIB avutil)

include_directories("inc" "$ENV{HMAKE_PROJECT_DIR}/_build/include")
add_executable(zpi1-vision src/analyzer.cpp src/app.cpp)
set_property(TARGET zpi1-vision PROPERTY CXX_STANDARD 11)
set_property(TARGET zpi1-vision PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(zpi1-vision -Wl,--start-group
    ${OpenCV_LIBS}
    ${AVDEVICE_LIB}
    ${AVFILTER_LIB}
    ${AVFORMAT_LIB}
    ${AVCODEC_LIB}
    ${SWSCALE_LIB}
    ${SWRESAMPLE_LIB}
    ${AVUTIL_LIB}
    -Wl,--end-group)
