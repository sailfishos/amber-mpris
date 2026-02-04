cmake_minimum_required(VERSION 3.16)

option(BUILD_DOCUMENTATION "Build Sailfish QDoc documentation" ON)

if(BUILD_DOCUMENTATION)

    find_package(Qt6 QUIET COMPONENTS Core)
    if(Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
    else()
        find_package(Qt5 REQUIRED COMPONENTS Core)
        set(QT_VERSION_MAJOR 5)
    endif()

    find_program(QDOC_EXECUTABLE qdoc REQUIRED)

    set(QDOC_CONF ${CMAKE_CURRENT_SOURCE_DIR}/ambermpris.qdocconf)
    set(QDOC_INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(QDOC_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/qdoc-html)
    set(DOC_INSTALL_PATH /usr/share/doc/amber-mpris)

    if(DEFINED ENV{SAILFISH_QDOC_TEMPLATES})
        set(ENV{QDOC_TEMPLATE_PATH} $ENV{SAILFISH_QDOC_TEMPLATES})
    endif()

    add_custom_command(
        OUTPUT ${QDOC_OUTPUT_DIR}/index.html
        COMMAND ${QDOC_EXECUTABLE}
                ${QDOC_CONF}
                -outputdir ${QDOC_OUTPUT_DIR}
        WORKING_DIRECTORY ${QDOC_INPUT_DIR}
        DEPENDS
            ${QDOC_CONF}
            src/index.qdoc
        COMMENT "Generating Sailfish offline QDoc for ambermpris"
        VERBATIM
    )

    add_custom_target(ambermpris-docs ALL
        DEPENDS ${QDOC_OUTPUT_DIR}/index.html
    )

    install(
        DIRECTORY ${QDOC_OUTPUT_DIR}/
        DESTINATION ${DOC_INSTALL_PATH}
    )

endif()
