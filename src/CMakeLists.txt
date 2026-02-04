cmake_minimum_required(VERSION 3.16)

if(QT_VERSION_MAJOR EQUAL 6)
    set(QT_PACKAGE Qt6)
    set(MPRISQTLIB ambermpris6)
else()
    set(QT_PACKAGE Qt5)
    set(MPRISQTLIB ambermpris)
endif()    

option(USE_SYSTEM_DBUS "Use system D-Bus" OFF)

add_library(${MPRISQTLIB} SHARED)

target_sources(${MPRISQTLIB}
    PRIVATE
        mpris.cpp
        mprisclient.cpp
        mpriscontroller.cpp
        mprisintrospectableadaptor.cpp
        mprismetadata.cpp
        mprismetadataproxy.cpp
        mprisplayer.cpp
        mprisplayeradaptor.cpp
        mprisplayerinterface.cpp
        mprispropertiesadaptor.cpp
        mprisrootinterface.cpp
        mprisserviceadaptor.cpp

        mpris.h
        mpris_p.h
        mprisclient.h
        mprisclient_p.h
        mpriscontroller.h
        mprisintrospectableadaptor_p.h
        mprismetadata.h
        mprismetadata_p.h
        mprismetadataproxy.h
        mprisplayeradaptor_p.h
        mprisplayer.h
        mprisplayer_p.h
        ambermpris.h
        ambermpris_p.h
        mprispropertiesadaptor_p.h
        mprisserviceadaptor_p.h     
        org.mpris.MediaPlayer2.xml
        org.mpris.MediaPlayer2.Player.xml
 
)

target_include_directories(${MPRISQTLIB}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/AmberMpris>
        ${CMAKE_SOURCE_DIR}/qtdbusextended
)

target_compile_definitions(${MPRISQTLIB}
    PRIVATE
        MPRIS_QT_LIBRARY
)

if (USE_SYSTEM_DBUS)
    target_compile_definitions(${MPRISQTLIB} PRIVATE USE_SYSTEM_DBUS)
endif()

target_link_libraries(${MPRISQTLIB}
    PRIVATE
        ${QT_PACKAGE}::Core
        ${QT_PACKAGE}::DBus
        dbusextended-qt${QT_VERSION_MAJOR}
)

set_target_properties(${MPRISQTLIB} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME ${MPRISQTLIB}
)

include(GNUInstallDirs)

install(TARGETS ${MPRISQTLIB}
    EXPORT AmberMprisTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
    mpris.h
    mprisclient.h
    mpriscontroller.h
    mprisplayer.h
    mprismetadata.h
    ambermpris.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/AmberMpris
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/AmberMpris.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/AmberMpris.pc
    @ONLY
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AmberMpris.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)