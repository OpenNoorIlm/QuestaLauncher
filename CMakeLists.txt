cmake_minimum_required(VERSION 3.16)
project(QuestaLauncher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg)

qt_add_executable(QuestaLauncher
    main.cpp
    mainwindow.h
    mainwindow.cpp
    prioritydialog.h
    prioritydialog.cpp
    resources.qrc
)

target_link_libraries(QuestaLauncher PRIVATE Qt6::Core Qt6::Widgets Qt6::Svg)
