// -*- c++ -*-

/*!
 *
 * Copyright (C) 2015-2021 Jolla Ltd.
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
 */


#include "mpriscontroller.h"

#include "mprisclient.h"
#include "ambermpris_p.h"

#include <algorithm>
#include <QMetaMethod>
#include <QTimer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QSharedPointer>

#include <QDebug>

using namespace Amber;

namespace {
    const QString mprisNameSpace = QStringLiteral("org.mpris.MediaPlayer2.");
    const QString dBusService = QStringLiteral("org.freedesktop.DBus");
    const QString dBusObjectPath = QStringLiteral("/org/freedesktop/DBus");
    const QString dBusInterface = QStringLiteral("org.freedesktop.DBus");
    const QString dBusNameOwnerChangedSignal = QStringLiteral("NameOwnerChanged");
}

namespace Amber {
class MprisControllerPrivate : public QObject
{
    Q_OBJECT

public:
    MprisControllerPrivate(MprisController *parent);
    ~MprisControllerPrivate();

    MprisController *parent() const;


public Q_SLOTS:
    void onNameOwnerChanged(const QString &service, const QString &oldOwner, const QString& newOwner);
    void onServiceAppeared(const QString &service);
    void onServiceVanished(const QString &service);
    void onAvailableClientPlaybackStatusChanged(MprisClient *client);

public:
    MprisClient *availableClient(const QString &service) const;
    MprisClient *pendingClient(const QString &service) const;
    void setCurrentClient(MprisClient *client);
    bool checkClient(const char *callerName) const;

    bool m_singleService;
    QString m_singleServiceName;
    MprisClient *m_currentClient;
    QDBusConnection m_connection;
    MprisMetaData m_dummyMetaData;
    QList<MprisClient *> m_pendingClients;
    QList<MprisClient *> m_availableClients;
    QList<MprisClient *> m_otherPlayingClients;
    unsigned m_positionConnected;
};
}

MprisControllerPrivate::MprisControllerPrivate(MprisController *parent)
    : QObject(parent)
    , m_singleService(false)
    , m_currentClient(nullptr)
    , m_connection(getDBusConnection())
    , m_dummyMetaData(this)
    , m_positionConnected(0)
{
    if (!m_connection.isConnected()) {
        qWarning() << "Mpris: Failed attempting to connect to DBus";
        return;
    }

    m_connection.connect(dBusService, dBusObjectPath, dBusInterface, dBusNameOwnerChangedSignal,
                         QStringList(), QString(),
                         this, SLOT(onNameOwnerChanged(QString, QString, QString)));

    QTimer::singleShot(0, [this]() {
        QStringList serviceNames = m_connection.interface()->registeredServiceNames();
        for (auto i = serviceNames.constBegin();
             i != serviceNames.constEnd();
             ++i) {
            if (i->startsWith(mprisNameSpace)) {
                onServiceAppeared(*i);
            }
        }
    });
}

MprisControllerPrivate::~MprisControllerPrivate()
{
}

MprisController *MprisControllerPrivate::parent() const
{
    return static_cast<MprisController *>(QObject::parent());
}

MprisController::MprisController(QObject *parent)
    : QObject(parent)
    , priv(new MprisControllerPrivate(this))
{
}

MprisController::~MprisController()
{
}

// Mpris2 Root Interface
bool MprisController::quit() const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->quit();
}

bool MprisController::raise() const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->raise();
}

// Mpris2 Player Interface
bool MprisController::next() const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->next();
}

bool MprisController::openUri(const QUrl &uri) const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->openUri(uri);
}

bool MprisController::pause() const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->pause();
}

bool MprisController::play() const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->play();
}

bool MprisController::playPause() const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->playPause();
}

bool MprisController::previous() const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->previous();
}

bool MprisController::seek(qlonglong offset) const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->seek(offset);
}

bool MprisController::setPosition(qlonglong position) const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->setPosition(position);
}

bool MprisController::setPosition(const QString &trackId, qlonglong position) const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->setPosition(trackId, position);
}

bool MprisController::stop() const
{
    return priv->checkClient(Q_FUNC_INFO) && priv->m_currentClient->stop();
}


// Slots

bool MprisController::singleService() const
{
    return priv->m_singleService;
}

void MprisController::setSingleService(bool single)
{
    if (priv->m_singleService == single) {
        return;
    }

    if (single) {
        if (priv->m_currentClient) {
            priv->m_singleServiceName = priv->m_currentClient->service();
        }
    } else if (!priv->m_currentClient
               || (priv->m_currentClient->playbackStatus() != Mpris::Playing
                   && !priv->m_otherPlayingClients.isEmpty())) {
        priv->setCurrentClient(priv->m_otherPlayingClients[0]);
    }

    priv->m_singleService = single;
    Q_EMIT singleServiceChanged();
}

QString MprisController::currentService() const
{
    return priv->m_currentClient ? priv->m_currentClient->service() : QString();
}

void MprisController::setCurrentService(const QString &service)
{
    if (!service.startsWith(mprisNameSpace)) {
        qWarning() << "Mpris:" << service << "is not a proper Mpris2 service";
        return;
    }

    if (!service.isEmpty()) {
        priv->m_singleServiceName = service;
    }

    if (priv->m_currentClient && priv->m_currentClient->service() == service) {
        return;
    }

    MprisClient *client = priv->availableClient(service);
    priv->setCurrentClient(client);
}

QStringList MprisController::availableServices() const
{
    QStringList result;

    for (auto i = priv->m_availableClients.constBegin();
         i != priv->m_availableClients.constEnd();
         ++i) {
        result.append((*i)->service());
    }

    return result;
}

QList<QObject *> MprisController::availableClients() const
{
    QList<QObject *> result;

    for (auto i = priv->m_availableClients.constBegin();
         i != priv->m_availableClients.constEnd();
         ++i) {
        result << *i;
    }

    return result;
}

// Mpris2 Root Interface
bool MprisController::canQuit() const
{
    return priv->m_currentClient && priv->m_currentClient->canQuit();
}

bool MprisController::canRaise() const
{
    return priv->m_currentClient && priv->m_currentClient->canRaise();
}

bool MprisController::canSetFullscreen() const
{
    return priv->m_currentClient && priv->m_currentClient->canSetFullscreen();
}

QString MprisController::desktopEntry() const
{
    return priv->m_currentClient ? priv->m_currentClient->desktopEntry() : QString();
}

bool MprisController::fullscreen() const
{
    return priv->m_currentClient && priv->m_currentClient->fullscreen();
}

void MprisController::setFullscreen(bool fullscreen)
{
    if (priv->checkClient(Q_FUNC_INFO)) {
        priv->m_currentClient->setFullscreen(fullscreen);
    }
}

bool MprisController::hasTrackList() const
{
    return priv->m_currentClient && priv->m_currentClient->hasTrackList();
}

QString MprisController::identity() const
{
    return priv->m_currentClient ? priv->m_currentClient->identity() : QString();
}

QStringList MprisController::supportedUriSchemes() const
{
    return priv->m_currentClient ? priv->m_currentClient->supportedUriSchemes() : QStringList();
}

QStringList MprisController::supportedMimeTypes() const
{
    return priv->m_currentClient ? priv->m_currentClient->supportedMimeTypes() : QStringList();
}

// Mpris2 Player Interface
bool MprisController::canControl() const
{
    return priv->m_currentClient && priv->m_currentClient->canControl();
}

bool MprisController::canGoNext() const
{
    return priv->m_currentClient && priv->m_currentClient->canGoNext();
}

bool MprisController::canGoPrevious() const
{
    return priv->m_currentClient && priv->m_currentClient->canGoPrevious();
}

bool MprisController::canPause() const
{
    return priv->m_currentClient && priv->m_currentClient->canPause();
}

bool MprisController::canPlay() const
{
    return priv->m_currentClient && priv->m_currentClient->canPlay();
}

bool MprisController::canSeek() const
{
    return priv->m_currentClient && priv->m_currentClient->canSeek();
}

Mpris::LoopStatus MprisController::loopStatus() const
{
    return priv->m_currentClient ? priv->m_currentClient->loopStatus() : Mpris::None;
}

void MprisController::setLoopStatus(Mpris::LoopStatus loopStatus)
{
    if (priv->checkClient(Q_FUNC_INFO)) {
        priv->m_currentClient->setLoopStatus(loopStatus);
    }
}

double MprisController::maximumRate() const
{
    return priv->m_currentClient ? priv->m_currentClient->maximumRate() : 1;
}

MprisMetaData *MprisController::metaData() const
{
    return priv->m_currentClient ? priv->m_currentClient->metaData() : &priv->m_dummyMetaData;
}

double MprisController::minimumRate() const
{
    return priv->m_currentClient ? priv->m_currentClient->minimumRate() : 1;
}

Mpris::PlaybackStatus MprisController::playbackStatus() const
{
    return priv->m_currentClient ? priv->m_currentClient->playbackStatus() : Mpris::Stopped;
}

qlonglong MprisController::position() const
{
    return priv->m_currentClient ? priv->m_currentClient->position() : 0;
}

void MprisController::requestPosition() const
{
    if (priv->checkClient(Q_FUNC_INFO)) {
        priv->m_currentClient->requestPosition();
    }
}

double MprisController::rate() const
{
    return priv->m_currentClient ? priv->m_currentClient->rate() : 1;
}

void MprisController::setRate(double rate)
{
    if (priv->checkClient(Q_FUNC_INFO)) {
        priv->m_currentClient->setRate(rate);
    }
}

bool MprisController::shuffle() const
{
    return priv->m_currentClient && priv->m_currentClient->shuffle();
}

void MprisController::setShuffle(bool shuffle)
{
    if (priv->checkClient(Q_FUNC_INFO)) {
        priv->m_currentClient->setShuffle(shuffle);
    }
}

double MprisController::volume() const
{
    return priv->m_currentClient ? priv->m_currentClient->volume() : 0;
}

void MprisController::setVolume(double volume)
{
    if (priv->checkClient(Q_FUNC_INFO)) {
        priv->m_currentClient->setVolume(volume);
    }
}

void MprisController::connectNotify(const QMetaMethod &method)
{
    if (method == QMetaMethod::fromSignal(&MprisController::positionChanged)) {
        if (!priv->m_positionConnected++ && priv->m_currentClient) {
            connect(priv->m_currentClient, &MprisClient::positionChanged, this, &MprisController::positionChanged);
        }
    }
}

void MprisController::disconnectNotify(const QMetaMethod &method)
{
    if (method == QMetaMethod::fromSignal(&MprisController::positionChanged)) {
        if (!--priv->m_positionConnected && priv->m_currentClient) {
            disconnect(priv->m_currentClient, &MprisClient::positionChanged, this, &MprisController::positionChanged);
        }
    }
}

// Private

void MprisControllerPrivate::onNameOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    // Unfortunately, QDBus doesn't allow flexible signal watchers.
    // Would it allow them, we could pass the filter "arg0namespace='org.mpris.MediaPlayer2'"
    // Therefore, we will receive here all the name changes in the
    // bus, not just the ones for our name space of interest, and we
    // will have to filter on our own :(

    if (!service.startsWith(mprisNameSpace)) {
        return;
    }

    if (oldOwner.isEmpty()) {
        onServiceAppeared(service);
        return;
    }

    if (newOwner.isEmpty()) {
        onServiceVanished(service);
        return;
    }

    // Service changed owner. Nothing to do ...
}

void MprisControllerPrivate::onServiceAppeared(const QString &service)
{
    MprisClient *client = availableClient(service);
    if (client) {
        Q_ASSERT(m_availableClients.contains(client));
        m_availableClients.removeOne(client);
        m_otherPlayingClients.removeOne(client);

        if (client == m_currentClient) {
            if (m_availableClients.isEmpty()) {
                m_currentClient = 0;
            } else {
                m_currentClient = m_availableClients[0];
            }
        }

        Q_EMIT parent()->availableServicesChanged();
        client->deleteLater();
    } else {
        client = pendingClient(service);

        if (client) {
            m_pendingClients.removeOne(client);
            client->deleteLater();
        }
    }

    client = new MprisClient(service, getDBusConnection(), this);

    auto validHandler = [this, client] {
        bool emitted = false;
        QMetaObject::Connection connection = connect(parent(), &MprisController::availableServicesChanged, this, [&emitted] { emitted = true; }, Qt::DirectConnection);
        m_pendingClients.removeOne(client);
        m_availableClients.prepend(client);
        if ((m_singleService && client->service() == m_singleServiceName)
            || (!m_singleService && !m_currentClient)) {
            setCurrentClient(client);
        } else {
            m_otherPlayingClients.prepend(client);
        }
        connect(client, &MprisClient::playbackStatusChanged, this, [this, client] { onAvailableClientPlaybackStatusChanged(client); });
        onAvailableClientPlaybackStatusChanged(client);

        if (!emitted) {
            Q_EMIT parent()->availableServicesChanged();
        }
        parent()->disconnect(connection);
    };

    if (!client->isValid()) {
        QSharedPointer<QMetaObject::Connection> connection(new QMetaObject::Connection);
        *connection = connect(client, &MprisClient::isValidChanged, this, [client, connection, validHandler] {
            if (client->isValid()) {
                client->disconnect(*connection);
                validHandler();
            }
        });
        m_pendingClients << client;
        return;
    }

    validHandler();
}

void MprisControllerPrivate::onServiceVanished(const QString &service)
{
    MprisClient *client = pendingClient(service);

    if (client) {
        m_pendingClients.removeOne(client);
        client->deleteLater();

        return;
    }

    client = availableClient(service);

    if (!client) {
        return;
    }

    m_availableClients.removeOne(client);
    m_otherPlayingClients.removeOne(client);
    Q_EMIT parent()->availableServicesChanged();

    if (m_currentClient == client) {
        if (m_singleService || m_availableClients.isEmpty()) {
            setCurrentClient(nullptr);
        } else {
            setCurrentClient(m_availableClients[0]);
        }
    }

    if (client) {
        client->deleteLater();
    }
}

void MprisControllerPrivate::onAvailableClientPlaybackStatusChanged(MprisClient *client)
{
    if (m_currentClient == client) {
        if (m_currentClient->playbackStatus() == Mpris::Playing) {
            if (m_availableClients[0] != m_currentClient) {
                m_availableClients.move(m_availableClients.indexOf(m_currentClient), 0);
                Q_EMIT parent()->availableServicesChanged();
            }
            return;
        }

        if (!m_otherPlayingClients.isEmpty()) {
            m_availableClients.move(m_availableClients.indexOf(m_currentClient), m_otherPlayingClients.length());
            MprisClient *currentClient = m_otherPlayingClients.takeFirst();
            m_availableClients.move(m_availableClients.indexOf(currentClient), 0);
            Q_EMIT parent()->availableServicesChanged();

            if (!m_singleService) {
                setCurrentClient(currentClient);
            }
        }
    } else {
        if (client->playbackStatus() != Mpris::Playing) {
            if (m_otherPlayingClients.removeOne(client)) {
                m_availableClients.move(m_availableClients.indexOf(client), m_otherPlayingClients.length());
                Q_EMIT parent()->availableServicesChanged();
            }
            return;
        }

        m_availableClients.move(m_availableClients.indexOf(client), 0);
        Q_EMIT parent()->availableServicesChanged();

        if (!m_singleService && (!m_currentClient
            || m_currentClient->playbackStatus() != Mpris::Playing)) {
            setCurrentClient(client);
        } else {
            m_otherPlayingClients.removeOne(client); // Just in case, shouldn't be needed
            m_otherPlayingClients.prepend(client);
        }
    }
}

MprisClient *MprisControllerPrivate::pendingClient(const QString &service) const
{
    auto result = std::find_if(m_pendingClients.cbegin(),
                               m_pendingClients.cend(),
                               [service](MprisClient *c) { return c->service() == service; });

    if (result != m_pendingClients.cend()) {
        return *result;
    }

    return nullptr;
}

MprisClient *MprisControllerPrivate::availableClient(const QString &service) const
{
    auto result = std::find_if(m_availableClients.cbegin(),
                               m_availableClients.cend(),
                               [service](MprisClient *c) { return c->service() == service; });

    if (result != m_availableClients.cend()) {
        return *result;
    }

    return nullptr;
}

void MprisControllerPrivate::setCurrentClient(MprisClient *client)
{
    if (client == m_currentClient) {
        return;
    }

    if (m_currentClient) {
        // Mpris Root Interface
        disconnect(m_currentClient, &MprisClient::canQuitChanged, parent(), &MprisController::canQuitChanged);
        disconnect(m_currentClient, &MprisClient::canRaiseChanged, parent(), &MprisController::canRaiseChanged);
        disconnect(m_currentClient, &MprisClient::canSetFullscreenChanged, parent(), &MprisController::canSetFullscreenChanged);
        disconnect(m_currentClient, &MprisClient::desktopEntryChanged, parent(), &MprisController::desktopEntryChanged);
        disconnect(m_currentClient, &MprisClient::fullscreenChanged, parent(), &MprisController::fullscreenChanged);
        disconnect(m_currentClient, &MprisClient::hasTrackListChanged, parent(), &MprisController::hasTrackListChanged);
        disconnect(m_currentClient, &MprisClient::identityChanged, parent(), &MprisController::identityChanged);
        disconnect(m_currentClient, &MprisClient::supportedUriSchemesChanged, parent(), &MprisController::supportedUriSchemesChanged);
        disconnect(m_currentClient, &MprisClient::supportedMimeTypesChanged, parent(), &MprisController::supportedMimeTypesChanged);

        // Mpris Player Interface
        disconnect(m_currentClient, &MprisClient::canControlChanged, parent(), &MprisController::canControlChanged);
        disconnect(m_currentClient, &MprisClient::canGoNextChanged, parent(), &MprisController::canGoNextChanged);
        disconnect(m_currentClient, &MprisClient::canGoPreviousChanged, parent(), &MprisController::canGoPreviousChanged);
        disconnect(m_currentClient, &MprisClient::canPauseChanged, parent(), &MprisController::canPauseChanged);
        disconnect(m_currentClient, &MprisClient::canPlayChanged, parent(), &MprisController::canPlayChanged);
        disconnect(m_currentClient, &MprisClient::canSeekChanged, parent(), &MprisController::canSeekChanged);
        disconnect(m_currentClient, &MprisClient::loopStatusChanged, parent(), &MprisController::loopStatusChanged);
        disconnect(m_currentClient, &MprisClient::maximumRateChanged, parent(), &MprisController::maximumRateChanged);
        disconnect(m_currentClient, &MprisClient::minimumRateChanged, parent(), &MprisController::minimumRateChanged);
        disconnect(m_currentClient, &MprisClient::playbackStatusChanged, parent(), &MprisController::playbackStatusChanged);
        disconnect(m_currentClient, &MprisClient::positionChanged, parent(), &MprisController::positionChanged);
        disconnect(m_currentClient, &MprisClient::rateChanged, parent(), &MprisController::rateChanged);
        disconnect(m_currentClient, &MprisClient::shuffleChanged, parent(), &MprisController::shuffleChanged);
        disconnect(m_currentClient, &MprisClient::volumeChanged, parent(), &MprisController::volumeChanged);
        disconnect(m_currentClient, &MprisClient::seeked, parent(), &MprisController::seeked);

        if (m_currentClient->playbackStatus() == Mpris::Playing) {
            m_otherPlayingClients.prepend(m_currentClient);
        }
    }

    bool oldCanQuit = parent()->canQuit();
    bool oldCanRaise = parent()->canRaise();
    bool oldCanSetFullscreen = parent()->canSetFullscreen();
    QString oldDesktopEntry = parent()->desktopEntry();
    bool oldFullscreen = parent()->fullscreen();
    bool oldHasTrackList = parent()->hasTrackList();
    QString oldIdentity = parent()->identity();
    QStringList oldSupportedUriSchemes = parent()->supportedUriSchemes();
    QStringList oldSupportedMimeTypes = parent()->supportedMimeTypes();
    bool oldCanControl = parent()->canControl();
    bool oldCanGoNext = parent()->canGoNext();
    bool oldCanGoPrevious = parent()->canGoPrevious();
    bool oldCanPause = parent()->canPause();
    bool oldCanPlay = parent()->canPlay();
    bool oldCanSeek = parent()->canSeek();
    Mpris::LoopStatus oldLoopStatus = parent()->loopStatus();
    double oldMaximumRate = parent()->maximumRate();
    const MprisMetaData *oldMetaData = parent()->metaData();
    double oldMinimumRate = parent()->minimumRate();
    Mpris::PlaybackStatus oldPlaybackStatus = parent()->playbackStatus();
    double oldRate = parent()->rate();
    bool oldShuffle = parent()->shuffle();
    double oldVolume = parent()->volume();

    m_currentClient = client;

    if (oldCanQuit != parent()->canQuit()) {
        Q_EMIT parent()->canQuitChanged();
    }
    if (oldCanRaise != parent()->canRaise()) {
        Q_EMIT parent()->canRaiseChanged();
    }
    if (oldCanSetFullscreen != parent()->canSetFullscreen()) {
        Q_EMIT parent()->canSetFullscreenChanged();
    }
    if (oldDesktopEntry != parent()->desktopEntry()) {
        Q_EMIT parent()->desktopEntryChanged();
    }
    if (oldFullscreen != parent()->fullscreen()) {
        Q_EMIT parent()->fullscreenChanged();
    }
    if (oldHasTrackList != parent()->hasTrackList()) {
        Q_EMIT parent()->hasTrackListChanged();
    }
    if (oldIdentity != parent()->identity()) {
        Q_EMIT parent()->identityChanged();
    }
    if (oldSupportedUriSchemes != parent()->supportedUriSchemes()) {
        Q_EMIT parent()->supportedUriSchemesChanged();
    }
    if (oldSupportedMimeTypes != parent()->supportedMimeTypes()) {
        Q_EMIT parent()->supportedMimeTypesChanged();
    }

    if (oldCanControl != parent()->canControl()) {
        Q_EMIT parent()->canControlChanged();
    }
    if (oldCanGoNext != parent()->canGoNext()) {
        Q_EMIT parent()->canGoNextChanged();
    }
    if (oldCanGoPrevious != parent()->canGoPrevious()) {
        Q_EMIT parent()->canGoPreviousChanged();
    }
    if (oldCanPause != parent()->canPause()) {
        Q_EMIT parent()->canPauseChanged();
    }
    if (oldCanPlay != parent()->canPlay()) {
        Q_EMIT parent()->canPlayChanged();
    }
    if (oldCanSeek != parent()->canSeek()) {
        Q_EMIT parent()->canSeekChanged();
    }
    if (oldLoopStatus != parent()->loopStatus()) {
        Q_EMIT parent()->loopStatusChanged();
    }
    if (oldMaximumRate != parent()->maximumRate()) {
        Q_EMIT parent()->maximumRateChanged();
    }
    if (oldMetaData != parent()->metaData()) {
        Q_EMIT parent()->metaDataChanged();
    }
    if (oldMinimumRate != parent()->minimumRate()) {
        Q_EMIT parent()->minimumRateChanged();
    }
    if (oldPlaybackStatus != parent()->playbackStatus()) {
        Q_EMIT parent()->playbackStatusChanged();
    }
    if (oldRate != parent()->rate()) {
        Q_EMIT parent()->rateChanged();
    }
    if (oldShuffle != parent()->shuffle()) {
        Q_EMIT parent()->shuffleChanged();
    }
    if (oldVolume != parent()->volume()) {
        Q_EMIT parent()->volumeChanged();
    }

    if (m_currentClient) {
        // Mpris Root Interface
        connect(m_currentClient, &MprisClient::canQuitChanged, parent(), &MprisController::canQuitChanged);
        connect(m_currentClient, &MprisClient::canRaiseChanged, parent(), &MprisController::canRaiseChanged);
        connect(m_currentClient, &MprisClient::canSetFullscreenChanged, parent(), &MprisController::canSetFullscreenChanged);
        connect(m_currentClient, &MprisClient::desktopEntryChanged, parent(), &MprisController::desktopEntryChanged);
        connect(m_currentClient, &MprisClient::fullscreenChanged, parent(), &MprisController::fullscreenChanged);
        connect(m_currentClient, &MprisClient::hasTrackListChanged, parent(), &MprisController::hasTrackListChanged);
        connect(m_currentClient, &MprisClient::identityChanged, parent(), &MprisController::identityChanged);
        connect(m_currentClient, &MprisClient::supportedUriSchemesChanged, parent(), &MprisController::supportedUriSchemesChanged);
        connect(m_currentClient, &MprisClient::supportedMimeTypesChanged, parent(), &MprisController::supportedMimeTypesChanged);
        connect(m_currentClient, &MprisClient::canControlChanged, parent(), &MprisController::canControlChanged);

        // Mpris Player Interface
        connect(m_currentClient, &MprisClient::canGoNextChanged, parent(), &MprisController::canGoNextChanged);
        connect(m_currentClient, &MprisClient::canGoPreviousChanged, parent(), &MprisController::canGoPreviousChanged);
        connect(m_currentClient, &MprisClient::canPauseChanged, parent(), &MprisController::canPauseChanged);
        connect(m_currentClient, &MprisClient::canPlayChanged, parent(), &MprisController::canPlayChanged);
        connect(m_currentClient, &MprisClient::canSeekChanged, parent(), &MprisController::canSeekChanged);
        connect(m_currentClient, &MprisClient::loopStatusChanged, parent(), &MprisController::loopStatusChanged);
        connect(m_currentClient, &MprisClient::maximumRateChanged, parent(), &MprisController::maximumRateChanged);
        connect(m_currentClient, &MprisClient::minimumRateChanged, parent(), &MprisController::minimumRateChanged);
        connect(m_currentClient, &MprisClient::playbackStatusChanged, parent(), &MprisController::playbackStatusChanged);
        if (m_positionConnected) {
            connect(m_currentClient, &MprisClient::positionChanged, parent(), &MprisController::positionChanged);
        }
        connect(m_currentClient, &MprisClient::rateChanged, parent(), &MprisController::rateChanged);
        connect(m_currentClient, &MprisClient::shuffleChanged, parent(), &MprisController::shuffleChanged);
        connect(m_currentClient, &MprisClient::volumeChanged, parent(), &MprisController::volumeChanged);
        connect(m_currentClient, &MprisClient::seeked, parent(), &MprisController::seeked);

        if (m_currentClient->playbackStatus() == Mpris::Playing) {
            m_otherPlayingClients.removeOne(m_currentClient);
        }
    }

    Q_EMIT parent()->currentServiceChanged();
    Q_EMIT parent()->metaDataChanged();
    Q_EMIT parent()->positionChanged(parent()->position());
}

bool MprisControllerPrivate::checkClient(const char *callerName) const
{
    if (!m_currentClient) {
        qWarning() << callerName << "None service available/selected";
        return false;
    }

    return true;
}

#include "mpriscontroller.moc"
