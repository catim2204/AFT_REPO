cmake_minimum_required(VERSION 3.22)
project(AFT_Refactored)

add_library(Shared
)

target_include_directories(Shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Inc)

