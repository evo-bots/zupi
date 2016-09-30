cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

include(ZuPiCommon)

project(smartcam1d CXX)

find_library(COMMON_LIB zupi)
find_library(FIDO_LIB fido)
find_package(OpenCV REQUIRED)
find_library(AVDEVICE_LIB avdevice)
find_library(AVFILTER_LIB avfilter)
find_library(AVFORMAT_LIB avformat)
find_library(AVCODEC_LIB avcodec)
find_library(SWSCALE_LIB swscale)
find_library(SWRESAMPLE_LIB swresample)
find_library(AVUTIL_LIB avutil)

add_executable(smartcam1d
    src/app.cpp
    src/timeline.cpp
    src/camera1d.cpp
    src/servo.cpp
    src/capture.cpp
    src/train.cpp
    src/simulate.cpp
    src/algo_fido.cpp
    src/algo_cust1.cpp
    src/algo_simpq.cpp
    src/algo_triangle.cpp
)
target_link_libraries(smartcam1d
    ${COMMON_LIB}
    ${FIDO_LIB}
    -Wl,--start-group
        ${OpenCV_LIBS}
        ${AVDEVICE_LIB}
        ${AVFILTER_LIB}
        ${AVFORMAT_LIB}
        ${AVCODEC_LIB}
        ${SWSCALE_LIB}
        ${SWRESAMPLE_LIB}
        ${AVUTIL_LIB}
    -Wl,--end-group
    pthread
)
