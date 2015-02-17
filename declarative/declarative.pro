include(../common.pri)

TEMPLATE = lib
CONFIG += qt plugin link_pkgconfig
DEPENDPATH += ../src
INCLUDEPATH += ../src

QT = core dbus qml

LIBS += -L../src -l$${MPRISQTLIB}
PKGCONFIG = dbusextended-qt5

TARGET = $${MPRISQTLIB}-qml-plugin
PLUGIN_IMPORT_PATH = org/nemomobile/mpris

QMAKE_SUBSTITUTES = qmldir.in

SOURCES += \
    mprisplugin.cpp

HEADERS += \
    mprisplugin.h

target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH

qml.files = qmldir
qml.path = $$target.path
INSTALLS += target qml
