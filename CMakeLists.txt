cmake_minimum_required(VERSION 3.14)

project(plasma_adaptive_accent_color LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui REQUIRED)
find_package(KF5Config)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
add_executable(plasma_adaptive_accent_color
  ${SRC_FILES}
)

target_include_directories(plasma_adaptive_accent_color PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(plasma_adaptive_accent_color PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(plasma_adaptive_accent_color PRIVATE Qt${QT_VERSION_MAJOR}::Gui)
target_link_libraries(plasma_adaptive_accent_color PRIVATE Qt${QT_VERSION_MAJOR}::DBus)
target_link_libraries(plasma_adaptive_accent_color PRIVATE KF5::ConfigCore)
