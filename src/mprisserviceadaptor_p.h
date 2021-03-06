/*!
 *
 * Copyright (C) 2021 Jolla Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This file was initially generated by qdbusxml2cpp version 0.8
 *
 * qdbusxml2cpp is Copyright (C) 2020 The Qt Company Ltd.
 *
 */

#ifndef MPRISSERVICEADAPTOR_P_H
#define MPRISSERVICEADAPTOR_P_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>


QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.mpris.MediaPlayer2
 */
namespace Amber {
class MprisPlayerPrivate;
class MprisServiceAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.mpris.MediaPlayer2\">\n"
"    <annotation value=\"true\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    <method name=\"Raise\">\n"
"    </method>\n"
"    <method name=\"Quit\">\n"
"    </method>\n"
"    <property access=\"read\" type=\"b\" name=\"CanQuit\">\n"
"      <annotation value=\"canQuit\" name=\"org.qtproject.QtDBus.PropertyGetter\"/>\n"
"    </property>\n"
"    <property access=\"readwrite\" type=\"b\" name=\"Fullscreen\">\n"
"      <annotation value=\"true\" name=\"org.mpris.MediaPlayer2.property.optional\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"b\" name=\"CanSetFullscreen\">\n"
"      <annotation value=\"true\" name=\"org.mpris.MediaPlayer2.property.optional\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"b\" name=\"CanRaise\">\n"
"    </property>\n"
"    <property access=\"read\" type=\"b\" name=\"HasTrackList\">\n"
"    </property>\n"
"    <property access=\"read\" type=\"s\" name=\"Identity\">\n"
"    </property>\n"
"    <property access=\"read\" type=\"s\" name=\"DesktopEntry\">\n"
"      <annotation value=\"true\" name=\"org.mpris.MediaPlayer2.property.optional\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"as\" name=\"SupportedUriSchemes\">\n"
"    </property>\n"
"    <property access=\"read\" type=\"as\" name=\"SupportedMimeTypes\">\n"
"    </property>\n"
"  </interface>\n"
        "")
public:
    MprisServiceAdaptor(MprisPlayerPrivate *parent);
    virtual ~MprisServiceAdaptor();

    MprisPlayerPrivate *parent() const;

public: // PROPERTIES
    Q_PROPERTY(bool CanQuit READ canQuit)
    bool canQuit() const;

    Q_PROPERTY(bool CanRaise READ canRaise)
    bool canRaise() const;

    Q_PROPERTY(bool CanSetFullscreen READ canSetFullscreen)
    bool canSetFullscreen() const;

    Q_PROPERTY(QString DesktopEntry READ desktopEntry)
    QString desktopEntry() const;

    Q_PROPERTY(bool Fullscreen READ fullscreen WRITE setFullscreen)
    bool fullscreen() const;
    void setFullscreen(bool value);

    Q_PROPERTY(bool HasTrackList READ hasTrackList)
    bool hasTrackList() const;

    Q_PROPERTY(QString Identity READ identity)
    QString identity() const;

    Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes)
    QStringList supportedMimeTypes() const;

    Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes)
    QStringList supportedUriSchemes() const;

public Q_SLOTS: // METHODS
    void Quit();
    void Raise();
Q_SIGNALS: // SIGNALS
};
}

#endif
