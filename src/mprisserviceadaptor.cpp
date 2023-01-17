/*
 *
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
 * This file was initially generated by qdbusxml2cpp version 0.8
 *
 * qdbusxml2cpp is Copyright (C) 2020 The Qt Company Ltd.
 *
 */

#include "mprisserviceadaptor_p.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include "mprisplayer_p.h"

using namespace Amber;

/*
 * Implementation of adaptor class MprisServiceAdaptor
 */

MprisServiceAdaptor::MprisServiceAdaptor(MprisPlayerPrivate *parent)
    : QDBusAbstractAdaptor(parent)
    , m_playerPrivate(parent)
{
    setAutoRelaySignals(true);
}

MprisServiceAdaptor::~MprisServiceAdaptor()
{
}

void MprisServiceAdaptor::Quit()
{
    // handle method call org.mpris.MediaPlayer2.Quit
    m_playerPrivate->quit();
}

void MprisServiceAdaptor::Raise()
{
    // handle method call org.mpris.MediaPlayer2.Raise
    m_playerPrivate->raise();
}

