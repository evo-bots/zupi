set(BUILD_SHARED_LIBRARIES OFF)
set(CMAKE_CXX_STANDARD 14)

include_directories(
    "inc"
    "$ENV{HMAKE_PROJECT_DIR}/common/cpp/inc"
    "$ENV{HMAKE_PROJECT_DIR}/out/noarch/include"
    "$ENV{HMAKE_PROJECT_DIR}/out/${ARCH}/include"
)

link_directories(
    "$ENV{HMAKE_PROJECT_DIR}/${ARCH}/lib"
)
