cmake_minimum_required(VERSION 3.16)

if(CMAKE_PREFIX_PATH MATCHES "6\\.|Qt6|qt6")
    set(QT_VERSION_MAJOR 6)
elseif(CMAKE_PREFIX_PATH MATCHES "5\\.|Qt5|qt5")
    set(QT_VERSION_MAJOR 5)
else()
    find_package(Qt6 QUIET COMPONENTS Core Gui Qml Quick DBus NO_DEFAULT_PATH)
    if(Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
    else()
        set(QT_VERSION_MAJOR 5)
    endif()
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick DBus)
    set(MPRIS_LIB ambermpris6)

    qt_add_executable(controller
        controller.cpp
    )

    target_link_libraries(controller
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Qml
            Qt6::Quick
            Qt6::DBus
            ${MPRIS_LIB}
    )

else()
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick DBus)
    set(QT Qt5)
    set(MPRIS_LIB ambermpris)

    add_executable(controller
        controller.cpp
    )

    target_link_libraries(controller
        PRIVATE
            Qt5::Core
            Qt5::Gui
            Qt5::Qml
            Qt5::Quick
            Qt5::DBus
            ${MPRIS_LIB}
    )
endif()

target_include_directories(controller
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

install(TARGETS controller
    DESTINATION /usr/bin
)
