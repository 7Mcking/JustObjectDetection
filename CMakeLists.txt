cmake_minimum_required(VERSION 3.27)
project(JustObjectDetection)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        Multimedia
        REQUIRED)

find_package(OpenCV REQUIRED)
message(STATUS "OpenCV is at location: ${OpenCV_DIR}")

add_executable(JustObjectDetection main.cpp
        MainWindow.h
        MainWindow.cpp
        utilities.h
        utilities.cpp
        captureThread.h
        captureThread.cpp)
target_link_libraries(JustObjectDetection
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Multimedia
        ${OpenCV_LIBS}
)

