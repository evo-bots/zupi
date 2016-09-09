cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

include(ZuPiCommon)

project(libzupi CXX)

add_library(zupi STATIC src/app.cpp)
