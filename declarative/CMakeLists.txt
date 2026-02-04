cmake_minimum_required(VERSION 3.16)

if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core DBus Qml)
    set(MPRIS_LIB ambermpris6)

    qt_add_qml_module(ambermprisplugin
        URI Amber.Mpris
        VERSION 1.0
        SOURCES
            declarativemprisplayer.cpp
            declarativemprisplayer_p.h
        DEPENDENCIES
            QtCore
            QtQml
    )

    target_link_libraries(ambermprisplugin
        PUBLIC
            ${MPRIS_LIB}
        PRIVATE
            Qt6::Core
            Qt6::DBus
            Qt6::Qml
    )

else()
    find_package(Qt5 REQUIRED COMPONENTS Core DBus Qml)
    set(QT Qt5)
    set(MPRIS_LIB ambermpris)

    if(NOT DEFINED QT5_INSTALL_QML)
        get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
        execute_process(
            COMMAND "${_qmake_executable}" -query QT_INSTALL_QML
            RESULT_VARIABLE _qmake_result
            OUTPUT_VARIABLE QT5_INSTALL_QML
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(NOT _qmake_result EQUAL 0)
        endif()
    endif()

    add_library(ambermprisplugin SHARED
        mprisplugin.cpp
        mprisplugin.h
        declarativemprisplayer.cpp
        declarativemprisplayer_p.h
    )

    target_link_libraries(ambermprisplugin
        PRIVATE
            Qt5::Core
            Qt5::DBus
            Qt5::Qml
            ${MPRIS_LIB}
    )

    install(TARGETS ambermprisplugin
        LIBRARY DESTINATION ${QT5_INSTALL_QML}/Amber/Mpris
    )

    install(FILES
        qmldir
        plugins.qmltypes
        MprisAudio.qml
        DESTINATION ${QT5_INSTALL_QML}/Amber/Mpris
    )
endif()
