include(../common.pri)

TEMPLATE = lib
CONFIG += qt link_pkgconfig no_keywords

QT = core dbus

TARGET = $${MPRISQTLIB}

DEFINES += MPRIS_QT_LIBRARY

use_system_dbus {
    DEFINES += USE_SYSTEM_DBUS
}

DEPENDPATH += ../qtdbusextended
INCLUDEPATH += ../qtdbusextended
LIBS += -L../qtdbusextended -ldbusextended-qt5

# Generate pkg-config support by default
# Note that we HAVE TO also create prl config as QMake implementation
# mixes both of them together.
CONFIG += create_pc create_prl no_install_prl

SOURCES += \
    mpris.cpp \
    mprisclient.cpp \
    mpriscontroller.cpp \
    mprisintrospectableadaptor.cpp \
    mprismetadata.cpp \
    mprismetadataproxy.cpp \
    mprisplayer.cpp \
    mprisplayeradaptor.cpp \
    mprisplayerinterface.cpp \
    mprispropertiesadaptor.cpp \
    mprisrootinterface.cpp \
    mprisserviceadaptor.cpp

HEADERS += \
    mpris.h \
    mpris_p.h \
    mprisclient.h \
    mprisclient_p.h \
    mpriscontroller.h \
    mprisintrospectableadaptor_p.h \
    mprismetadata.h \
    mprismetadata_p.h \
    mprismetadataproxy.h \
    mprisplayeradaptor_p.h \
    mprisplayer.h \
    mprisplayer_p.h \
    ambermpris.h \
    ambermpris_p.h \
    mprispropertiesadaptor_p.h \
    mprisserviceadaptor_p.h

INSTALL_HEADERS = \
    Mpris \
    MprisPlayer \
    MprisClient \
    MprisController \
    MprisMetaData \
    mpris.h \
    mprisclient.h \
    mpriscontroller.h \
    mprisplayer.h \
    mprismetadata.h \
    ambermpris.h

OTHER_FILES += \
    org.mpris.MediaPlayer2.xml \
    org.mpris.MediaPlayer2.Player.xml

isEmpty(PREFIX) {
    target.path = $$[QT_INSTALL_LIBS]
    headers.path = /usr/include/AmberMpris
    prf.path = $$[QMAKE_MKSPECS]/features
} else {
    target.path = $${PREFIX}/lib
    headers.path = $${PREFIX}/include/AmberMpris
    prf.path = $${PREFIX}/share/qt5/mkspecs/features
}
headers.files = $$INSTALL_HEADERS

INSTALLS += target headers

equals(QT_MAJOR_VERSION, 5) {
prf.files = $${TARGET}.prf
INSTALLS += prf
QMAKE_PKGCONFIG_REQUIRES = Qt5Core Qt5DBus
}

equals(QT_MAJOR_VERSION, 6) {
QMAKE_PKGCONFIG_REQUIRES = Qt6Core Qt6DBus
}

QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_NAME = AmberMpris
