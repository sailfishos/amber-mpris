cmake_minimum_required(VERSION 3.16)

if(QT_VERSION_MAJOR EQUAL 6)
    set(QT_PACKAGE Qt6)
    set(DBUSEXT_LIB dbusextended-qt6)
else()
    set(QT_PACKAGE Qt5)
    set(DBUSEXT_LIB dbusextended-qt5)
endif()

add_library(${DBUSEXT_LIB} STATIC)

set_target_properties(${DBUSEXT_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_sources(${DBUSEXT_LIB}
    PRIVATE
        dbusextendedabstractinterface.cpp
        dbusextended.h
        dbusextendedabstractinterface.h
)

target_compile_definitions(${DBUSEXT_LIB}
    PRIVATE
        QT_DBUS_EXTENDED_LIBRARY
)

target_link_libraries(${DBUSEXT_LIB}
    PUBLIC
        ${QT_PACKAGE}::Core
        ${QT_PACKAGE}::DBus
)

target_include_directories(${DBUSEXT_LIB}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
