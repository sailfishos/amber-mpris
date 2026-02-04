cmake_minimum_required(VERSION 3.16)

enable_language(CXX)

if(CMAKE_PREFIX_PATH MATCHES "6\\.|Qt6|qt6")
    set(QT_VERSION_MAJOR 6)
elseif(CMAKE_PREFIX_PATH MATCHES "5\\.|Qt5|qt5")
    set(QT_VERSION_MAJOR 5)
else()
    find_package(Qt6 QUIET COMPONENTS Core DBus NO_DEFAULT_PATH)
    if(Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
    else()
        set(QT_VERSION_MAJOR 5)
    endif()
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    set(QML_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/example/qml-qt6")
else()
    set(QML_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/example/qml-qt5")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core DBus)
    project(amber-mpris-qt6
        VERSION 1.0.0
        DESCRIPTION "MPRIS D-Bus implementation"
        LANGUAGES CXX
    )
else()
    find_package(Qt5 REQUIRED COMPONENTS Core DBus)
    project(amber-mpris-qt5
        VERSION 1.0.0
        DESCRIPTION "MPRIS D-Bus implementation"
        LANGUAGES CXX
    )
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(NO_QML "Building without QML dependency" OFF)
if(NO_QML)
else()
endif()

file(COPY "${QML_SOURCE_DIR}/" DESTINATION "${CMAKE_BINARY_DIR}/example/qml")

add_subdirectory(qtdbusextended)
add_subdirectory(src)
add_subdirectory(doc)

if(NOT NO_QML)
    add_subdirectory(declarative)
endif()
