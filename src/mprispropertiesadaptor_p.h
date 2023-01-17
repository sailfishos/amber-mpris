/*!
 * Copyright (C) 2015-2023 Jolla Ltd.
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
 */

#ifndef MPRISPROPERTIESADAPTOR_P_H
#define MPRISPROPERTIESADAPTOR_P_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

namespace Amber {

class MprisPlayerPrivate;

/*
 * Adaptor class for interface org.freedesktop.DBus.Properties
 *
 * This is needed because the standard QDBusAbstractAdaptor
 * Properties implementation doesn't allow errors to be
 * returned for the org.freedesktop.DBus.Properties.Set method,
 * but these are needed to fulfil the MPRIS spec.
 *
 */
class MprisPropertiesAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.Properties")
    // Introspection of Get, Set and GetAll happens automatically
    // so avoid adding them twice
    Q_CLASSINFO("D-Bus Introspection", "")

public:
    MprisPropertiesAdaptor(MprisPlayerPrivate *parent);
    virtual ~MprisPropertiesAdaptor();

public Q_SLOTS: // METHODS
    QDBusVariant Get(const QString &interface_name, const QString &property_name);
    void Set(const QString &interface_name, const QString &property_name, const QDBusVariant &value);
    QVariantMap GetAll(const QString &interface_name);

private:
    void replyPropertyNotFoundError(const QString &interface_name, const QString &property_name);
    void replyPropertyReadOnlyError(const QString &interface_name, const QString &property_name);
    void replyInternalError();

    QVariant get(const QString &getter);
    void set(const QString &setter, const QDBusVariant &value);

    MprisPlayerPrivate *m_playerPrivate;
};
}

#endif
