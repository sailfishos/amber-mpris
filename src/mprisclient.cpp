/*
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

/*!
    \qmltype MprisClient
    \inqmlmodule Amber.Mpris
    \brief Remote controls a specific media player
    \internal

    MprisClient provides ability to remotely control a certain media player
    via the MPRIS API.

    MprisClient can not be instantiated directly, but only through
    MprisController.availableClients. Note that the lifetime of a
    MprisClient is only as long as it is on the availableClients
    list.
*/

/*!
    \qmlproperty bool MprisClient::canControl
    \brief Indicates whether the player can be controlled

    When set to false, no control of the player is expected to work,
    only status can be read.

    According to the specification, this should not change value after
    the controller becomes valid, but the library does not prevent it.
*/

/*!
    \qmlproperty int MprisClient::positionInterval
    \brief Controls the emission rate of MprisClient::position notifications

    Indicates, in milliseconds, how often the MprisClient::position property
    changes are notified. Defaults to 1000 ms.

    Note that the playback rate is not taken into consideration, hence the
    change in position may not be proportional to the interval.
*/


#include "mprisclient.h"

#include "mprisclient_p.h"
#include "mprismetadata_p.h"

#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QElapsedTimer>
#include <QUrl>
#include <QMetaObject>
#include <QMetaEnum>

#include <QDebug>

namespace {
    const QString mprisObjectPath = QStringLiteral("/org/mpris/MediaPlayer2");
}

using namespace Amber;

class Amber::MprisClientPrivate : public QObject {
    Q_OBJECT

public:
    MprisClientPrivate(const QString &service, const QDBusConnection &connection, MprisClient *parent);

    ~MprisClientPrivate();

public Q_SLOTS:
    void onAsyncGetAllRootPropertiesFinished();
    void onAsyncGetAllPlayerPropertiesFinished();
    void onAsyncPropertyFinished(const QString &propertyName);
    void onCanControlChanged();
    void onMetadataChanged();
    void onPositionChanged(qlonglong aPosition);
    void onRateChanged();
    void onFinishedPendingCall(QDBusPendingCallWatcher *call);
    void onPlaybackStatusChanged();
    void onSeeked(qlonglong aPosition);
    void onPositionTimeout();

public:
    MprisRootInterface m_mprisRootInterface;
    MprisPlayerInterface m_mprisPlayerInterface;

    MprisMetaData m_metaData;
    QTimer m_positionTimer;
    MprisClient *parent() const;

    void handleCall(const QDBusPendingReply<> &reply);

    mutable bool m_initedRootInterface;
    mutable bool m_initedPlayerInterface;
    mutable bool m_requestedPosition;
    bool m_canControlReceived;
    int m_syncInterval;
    unsigned m_positionConnected;
    qlonglong m_lastPosition;
    QElapsedTimer m_positionElapsed;
};

MprisClientPrivate::MprisClientPrivate(const QString &service, const QDBusConnection &connection, MprisClient *parent)
    : QObject(parent)
    , m_mprisRootInterface(service, mprisObjectPath, connection, this)
    , m_mprisPlayerInterface(service, mprisObjectPath, connection, this)
    , m_metaData(this)
    , m_positionTimer(this)
    , m_initedRootInterface(false)
    , m_initedPlayerInterface(false)
    , m_requestedPosition(false)
    , m_canControlReceived(false)
    , m_syncInterval(5000)
    , m_positionConnected(0)
    , m_lastPosition(-1)
{
    m_positionTimer.setInterval(1000);
    m_positionTimer.setSingleShot(false);
    connect(&m_positionTimer, &QTimer::timeout, this, &MprisClientPrivate::onPositionTimeout);
    connect(&m_mprisPlayerInterface, &Private::DBusExtendedAbstractInterface::asyncPropertyFinished, this, &MprisClientPrivate::onAsyncPropertyFinished);
}

MprisClientPrivate::~MprisClientPrivate()
{
}

void MprisClientPrivate::onPositionTimeout()
{
    if (parent()->playbackStatus() == Mpris::Playing && m_positionElapsed.elapsed() > m_syncInterval) {
        parent()->requestPosition();
    } else {
        Q_EMIT parent()->positionChanged(parent()->position());
    }
}

MprisClient *MprisClientPrivate::parent() const
{
    return static_cast<MprisClient *>(QObject::parent());
}

void MprisClientPrivate::handleCall(const QDBusPendingReply<> &reply)
{
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished,
                     this, &MprisClientPrivate::onFinishedPendingCall);
}

MprisClient::MprisClient(const QString &service, const QDBusConnection &connection, QObject *parent)
    : QObject(parent)
    , priv(new MprisClientPrivate(service, connection, this))
{
    // Mpris Root Interface
    connect(&priv->m_mprisRootInterface, &MprisRootInterface::asyncGetAllPropertiesFinished, priv, &MprisClientPrivate::onAsyncGetAllRootPropertiesFinished);
    connect(&priv->m_mprisRootInterface, &MprisRootInterface::canQuitChanged, this, &MprisClient::canQuitChanged);
    connect(&priv->m_mprisRootInterface, &MprisRootInterface::canRaiseChanged, this, &MprisClient::canRaiseChanged);
    connect(&priv->m_mprisRootInterface, &MprisRootInterface::canSetFullscreenChanged, this, &MprisClient::canSetFullscreenChanged);
    connect(&priv->m_mprisRootInterface, &MprisRootInterface::desktopEntryChanged, this, &MprisClient::desktopEntryChanged);
    connect(&priv->m_mprisRootInterface, &MprisRootInterface::fullscreenChanged, this, &MprisClient::fullscreenChanged);
    connect(&priv->m_mprisRootInterface, &MprisRootInterface::hasTrackListChanged, this, &MprisClient::hasTrackListChanged);
    connect(&priv->m_mprisRootInterface, &MprisRootInterface::identityChanged, this, &MprisClient::identityChanged);
    connect(&priv->m_mprisRootInterface, &MprisRootInterface::supportedMimeTypesChanged, this, &MprisClient::supportedMimeTypesChanged);
    connect(&priv->m_mprisRootInterface, &MprisRootInterface::supportedUriSchemesChanged, this, &MprisClient::supportedUriSchemesChanged);
    priv->m_mprisRootInterface.setUseCache(true);

    // Mpris Player Interface
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::asyncGetAllPropertiesFinished, priv, &MprisClientPrivate::onAsyncGetAllPlayerPropertiesFinished);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::canControlChanged, priv, &MprisClientPrivate::onCanControlChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::canGoNextChanged, this, &MprisClient::canGoNextChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::canGoPreviousChanged, this, &MprisClient::canGoPreviousChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::canPauseChanged, this, &MprisClient::canPauseChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::canPlayChanged, this, &MprisClient::canPlayChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::canSeekChanged, this, &MprisClient::canSeekChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::canSeekChanged, this, &MprisClient::canSeekChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::loopStatusChanged, this, &MprisClient::loopStatusChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::maximumRateChanged, this, &MprisClient::maximumRateChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::metadataChanged, priv, &MprisClientPrivate::onMetadataChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::minimumRateChanged, this, &MprisClient::minimumRateChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::playbackStatusChanged, priv, &MprisClientPrivate::onPlaybackStatusChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::positionChanged, priv, &MprisClientPrivate::onPositionChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::rateChanged, priv, &MprisClientPrivate::onRateChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::shuffleChanged, this, &MprisClient::shuffleChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::volumeChanged, this, &MprisClient::volumeChanged);
    connect(&priv->m_mprisPlayerInterface, &MprisPlayerInterface::Seeked, priv, &MprisClientPrivate::onSeeked);
    priv->m_mprisPlayerInterface.setUseCache(true);

    priv->m_mprisRootInterface.getAllProperties();
    priv->m_mprisPlayerInterface.getAllProperties();
}

MprisClient::~MprisClient()
{
}

bool MprisClient::isValid() const
{
    return priv->m_initedRootInterface && priv->m_initedPlayerInterface;
}

int MprisClient::positionInterval() const
{
    return priv->m_positionTimer.interval();
}

void MprisClient::setPositionInterval(int interval)
{
    priv->m_positionTimer.setInterval(interval);
}

// Mpris2 Root Interface
bool MprisClient::quit()
{
    if (!canQuit()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    priv->handleCall(priv->m_mprisRootInterface.Quit());

    return true;
}

bool MprisClient::raise()
{
    if (!canRaise()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    priv->handleCall(priv->m_mprisRootInterface.Raise());

    return true;
}

// Mpris2 Player Interface
bool MprisClient::next()
{
    if (!canGoNext()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    priv->handleCall(priv->m_mprisPlayerInterface.Next());

    return true;
}

bool MprisClient::openUri(const QUrl &uri)
{
    if (!canControl()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    if (!uri.isValid()) {
        qDebug() << Q_FUNC_INFO << "The uri is invalid";
        return false;
    }

    priv->handleCall(priv->m_mprisPlayerInterface.OpenUri(uri.toString()));

    return true;
}

bool MprisClient::pause()
{
    if (!canPause()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    priv->handleCall(priv->m_mprisPlayerInterface.Pause());

    return true;
}

bool MprisClient::play()
{
    if (!canPlay()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    priv->handleCall(priv->m_mprisPlayerInterface.Play());

    return true;
}

bool MprisClient::playPause()
{
    if (!canPlay() && !canPause()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    priv->handleCall(priv->m_mprisPlayerInterface.PlayPause());

    return true;
}

bool MprisClient::previous()
{
    if (!canGoPrevious()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    priv->handleCall(priv->m_mprisPlayerInterface.Previous());

    return true;
}

bool MprisClient::seek(qlonglong offset)
{
    if (!canSeek()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    priv->handleCall(priv->m_mprisPlayerInterface.Seek(offset));

    return true;
}

bool MprisClient::setPosition(qlonglong position)
{
    if (!canSeek()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    QVariant trackId = metaData()->trackId();
    if (!trackId.isValid()) {
        qDebug() << Q_FUNC_INFO << "Unknown trackId in which to set the position";
        return false;
    }

    return setPosition(trackId.value<QDBusObjectPath>().path(), position);
}

bool MprisClient::setPosition(const QString &aTrackId, qlonglong position)
{
    if (!canSeek()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    QDBusObjectPath trackId(aTrackId);
    if (trackId.path().isEmpty()) {
        qDebug() << Q_FUNC_INFO << "trackId doesn't map to a valid DBus object path";
        return false;
    }

    QVariant length = metaData()->duration();
    if (length.isValid()) {
        qlonglong reportedLength = length.toLongLong();

        if (position < 0 || position > reportedLength) {
            qDebug() << Q_FUNC_INFO << "Requested position out of range";
            return false;
        }
    }

    priv->handleCall(priv->m_mprisPlayerInterface.SetPosition(trackId, position * 1000));

    return true;
}

bool MprisClient::stop()
{
    if (!canControl()) {
        qDebug() << Q_FUNC_INFO << "The method is not allowed";
        return false;
    }

    priv->handleCall(priv->m_mprisPlayerInterface.Stop());

    return true;
}


// Slots

QString MprisClient::service() const
{
    return priv->m_mprisRootInterface.service();
}

// Mpris2 Root Interface
bool MprisClient::canQuit() const
{
    return priv->m_mprisRootInterface.canQuit();
}

bool MprisClient::canRaise() const
{
    return priv->m_mprisRootInterface.canRaise();
}

bool MprisClient::canSetFullscreen() const
{
    return priv->m_mprisRootInterface.canSetFullscreen();
}

QString MprisClient::desktopEntry() const
{
    return priv->m_mprisRootInterface.desktopEntry();
}

bool MprisClient::fullscreen() const
{
    return priv->m_mprisRootInterface.fullscreen();
}

void MprisClient::setFullscreen(bool fullscreen)
{
    priv->m_mprisRootInterface.setFullscreen(fullscreen);
}

bool MprisClient::hasTrackList() const
{
    return priv->m_mprisRootInterface.hasTrackList();
}

QString MprisClient::identity() const
{
    return priv->m_mprisRootInterface.identity();
}

QStringList MprisClient::supportedUriSchemes() const
{
    return priv->m_mprisRootInterface.supportedUriSchemes();
}

QStringList MprisClient::supportedMimeTypes() const
{
    return priv->m_mprisRootInterface.supportedMimeTypes();
}

// Mpris2 Player Interface
bool MprisClient::canControl() const
{
    return priv->m_mprisPlayerInterface.canControl();
}

bool MprisClient::canGoNext() const
{
    if (canControl()) {
        return priv->m_mprisPlayerInterface.canGoNext();
    }

    return false;
}

bool MprisClient::canGoPrevious() const
{
    if (canControl()) {
        return priv->m_mprisPlayerInterface.canGoPrevious();
    }

    return false;
}

bool MprisClient::canPause() const
{
    if (canControl()) {
        return priv->m_mprisPlayerInterface.canPause();
    }

    return false;
}

bool MprisClient::canPlay() const
{
    if (canControl()) {
        return priv->m_mprisPlayerInterface.canPlay();
    }

    return false;
}

bool MprisClient::canSeek() const
{
    if (canControl()) {
        return priv->m_mprisPlayerInterface.canSeek();
    }

    return false;
}

Mpris::LoopStatus MprisClient::loopStatus() const
{
    bool ok;
    int enumVal = QMetaEnum::fromType<Mpris::LoopStatus>().keyToValue(priv->m_mprisPlayerInterface.loopStatus().toUtf8(), &ok);

    if (ok) {
        return static_cast<Mpris::LoopStatus>(enumVal);
    }

    return Mpris::LoopNone;
}

void MprisClient::setLoopStatus(Mpris::LoopStatus loopStatus)
{
    const char *strVal = QMetaEnum::fromType<Mpris::LoopStatus>().valueToKey(loopStatus);
    priv->m_mprisPlayerInterface.setLoopStatus(QString::fromLatin1(strVal));
}

double MprisClient::maximumRate() const
{
    return priv->m_mprisPlayerInterface.maximumRate();
}

MprisMetaData *MprisClient::metaData() const
{
    return &priv->m_metaData;
}

double MprisClient::minimumRate() const
{
    return priv->m_mprisPlayerInterface.minimumRate();
}

Mpris::PlaybackStatus MprisClient::playbackStatus() const
{
    bool ok;
    int enumVal = QMetaEnum::fromType<Mpris::PlaybackStatus>().keyToValue(priv->m_mprisPlayerInterface.playbackStatus().toUtf8(), &ok);

    if (ok) {
        return static_cast<Mpris::PlaybackStatus>(enumVal);
    }

    return Mpris::Stopped;
}

qlonglong MprisClient::position() const
{
    if (playbackStatus() == Mpris::Playing) {
        return priv->m_lastPosition + priv->m_positionElapsed.elapsed() * priv->m_mprisPlayerInterface.rate();
    }
    return priv->m_lastPosition;
}

void MprisClient::requestPosition() const
{
    if (priv->m_requestedPosition) {
        return;
    }

    priv->m_mprisPlayerInterface.setUseCache(false);
    priv->m_mprisPlayerInterface.position();
    priv->m_mprisPlayerInterface.setUseCache(true);
    if (priv->m_mprisPlayerInterface.lastExtendedError().isValid()) {
        qWarning() << Q_FUNC_INFO
                   << "Failed requesting the current position in the MPRIS2 Player Interface!!!";
        return;
    }
    priv->m_requestedPosition = true;
}

double MprisClient::rate() const
{
    return priv->m_mprisPlayerInterface.rate();
}

void MprisClient::setRate(double rate)
{
    priv->m_mprisPlayerInterface.setRate(rate);
}

bool MprisClient::shuffle() const
{
    return priv->m_mprisPlayerInterface.shuffle();
}

void MprisClient::setShuffle(bool shuffle)
{
    priv->m_mprisPlayerInterface.setShuffle(shuffle);
}

double MprisClient::volume() const
{
    return priv->m_mprisPlayerInterface.volume();
}

void MprisClient::setVolume(double volume)
{
    priv->m_mprisPlayerInterface.setVolume(volume);
}

void MprisClient::connectNotify(const QMetaMethod &method)
{
    if (method == QMetaMethod::fromSignal(&MprisClient::positionChanged)) {
        if (!priv->m_positionConnected++ && playbackStatus() == Mpris::Playing) {
            priv->m_positionTimer.start();
        }
    }

    QObject::connectNotify(method);
}

void MprisClient::disconnectNotify(const QMetaMethod &method)
{
    if (method == QMetaMethod::fromSignal(&MprisClient::positionChanged)) {
        if (!--priv->m_positionConnected) {
            priv->m_positionTimer.stop();
        }
    }

    QObject::disconnectNotify(method);
}

// Protected

void MprisClientPrivate::onAsyncGetAllRootPropertiesFinished()
{
    if (m_mprisRootInterface.lastExtendedError().isValid()) {
        qWarning() << Q_FUNC_INFO
                   << "Error" << m_mprisRootInterface.lastExtendedError().name()
                   << "happened:" << m_mprisRootInterface.lastExtendedError().message();
        return;
    }

    m_initedRootInterface = true;

    if (parent()->isValid()) {
            Q_EMIT parent()->isValidChanged();
    }
}

void MprisClientPrivate::onAsyncGetAllPlayerPropertiesFinished()
{
    if (m_mprisPlayerInterface.lastExtendedError().isValid()) {
        qWarning() << Q_FUNC_INFO
                   << "Error" << m_mprisPlayerInterface.lastExtendedError().name()
                   << "happened:" << m_mprisPlayerInterface.lastExtendedError().message();
        return;
    }

    m_initedPlayerInterface = true;

    if (parent()->isValid()) {
            Q_EMIT parent()->isValidChanged();
    }
}

void MprisClientPrivate::onAsyncPropertyFinished(const QString &propertyName)
{
    if (propertyName == QLatin1String("Position")) {
        m_requestedPosition = false;
    }
}

void MprisClientPrivate::onCanControlChanged()
{
    // On first reception, we are using a "GetAll" so we can skip this
    if (m_canControlReceived) {
        // I could disconnect and re-connect the signals so I avoid
        // double arriving signals but this really shouldn't happen
        // ever.
        Q_EMIT parent()->canGoNextChanged();
        Q_EMIT parent()->canGoPreviousChanged();
        Q_EMIT parent()->canPauseChanged();
        Q_EMIT parent()->canPlayChanged();
        Q_EMIT parent()->canSeekChanged();
        qWarning() << Q_FUNC_INFO
                   << "CanControl is not supposed to change its value!";
        return;
    }

    m_canControlReceived = true;
}

void MprisClientPrivate::onMetadataChanged()
{
    QString oldTrackId = m_metaData.trackId().toString();
    m_metaData.priv->setMetaData(m_mprisPlayerInterface.metadata());

    if (oldTrackId != m_metaData.trackId()) {
        m_lastPosition = 0;
        m_positionElapsed.start();
        Q_EMIT parent()->metaDataChanged();
    }
}

void MprisClientPrivate::onPositionChanged(qlonglong aPosition)
{
    m_positionElapsed.start();
    m_lastPosition = aPosition / 1000;
    Q_EMIT parent()->positionChanged(aPosition / 1000);
}

void MprisClientPrivate::onRateChanged()
{
    if (parent()->playbackStatus() == Mpris::Playing) {
        parent()->requestPosition();
    }
    Q_EMIT parent()->rateChanged();
}

// Private

void MprisClientPrivate::onPlaybackStatusChanged()
{
    switch (parent()->playbackStatus()) {
    case Mpris::Paused:
        m_lastPosition += m_positionElapsed.elapsed();
        m_positionTimer.stop();
        break;

    case Mpris::Stopped:
        m_lastPosition = 0;
        m_positionTimer.stop();
        Q_EMIT parent()->positionChanged(0);
        break;

    case Mpris::Playing:
        m_positionElapsed.start();
        if (m_positionConnected) {
            m_positionTimer.start();
        }
        break;
    }

    Q_EMIT parent()->playbackStatusChanged();
}

void MprisClientPrivate::onSeeked(qlonglong aPosition)
{
    m_lastPosition = aPosition / 1000;
    m_positionElapsed.start();
    Q_EMIT parent()->positionChanged(aPosition / 1000);
    Q_EMIT parent()->seeked(aPosition);
}

void MprisClientPrivate::onFinishedPendingCall(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<> reply = *call;
    if (reply.isError()) {
        qWarning() << Q_FUNC_INFO
                   << "Error" << reply.error().name()
                   << "happened:" << reply.error().message();
    }

    call->deleteLater();
}

#include "mprisclient.moc"
