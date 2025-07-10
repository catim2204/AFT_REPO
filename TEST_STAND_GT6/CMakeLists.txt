cmake_minimum_required(VERSION 3.16)

project(TEST_STAND_GT6 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "/home/humble/Qt/6.9.1/gcc_64")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick SerialPort Bluetooth Network)

find_package(Qt6 6 REQUIRED COMPONENTS Quick)

file(GLOB CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/Core/Src/*.cpp
)
file(GLOB CORE_HEADERS
    ${CMAKE_SOURCE_DIR}/Core/Inc/*.h
)


qt_standard_project_setup(REQUIRES 6)

qt_add_executable(TSQT6
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${CMAKE_SOURCE_DIR}/../Shared/Inc/Data_Packets.h
    ${CMAKE_SOURCE_DIR}/Core/Src/mainwindow.ui 
)
target_include_directories(TSQT6 PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/../Shared/Inc
)

qt_add_qml_module(TSQT6
    URI TEST_STAND_GT6
    VERSION 1.0
    QML_FILES
        Main.qml
)

set_target_properties(TSQT6 PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)



include(GNUInstallDirs)
install(TARGETS TSQT6
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
target_link_libraries(TSQT6 PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Quick Qt6::SerialPort Qt6::Bluetooth Qt6::Network)

