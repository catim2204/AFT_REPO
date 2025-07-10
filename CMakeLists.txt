cmake_minimum_required(VERSION 3.22)
project(AFT_Refactored)


# Add individual projects
add_subdirectory(TEST_STAND_GT6)
# Add Shared library
add_subdirectory(Shared)

# Only add STM32 project if toolchain is available
if(EXISTS "${CMAKE_SOURCE_DIR}/fw_STM32")
    add_subdirectory(fw_STM32)
endif()
