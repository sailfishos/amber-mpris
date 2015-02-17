// -*- c++ -*-

/*!
 *
 * Copyright (C) 2015 Jolla Ltd.
 *
 * Contact: Valerio Valerio <valerio.valerio@jolla.com>
 * Author: Andres Gomez <andres.gomez@jolla.com>
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


#include "mprismanager.h"

#include "mpriscontroller.h"

#include <qqmlinfo.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <QtCore/QSignalMapper>

static const QString mprisNameSpace = QStringLiteral("org.mpris.MediaPlayer2.*");
static const QString dBusService = QStringLiteral("org.freedesktop.DBus");
static const QString dBusObjectPath = QStringLiteral("/org/freedesktop/DBus");
static const QString dBusInterface = QStringLiteral("org.freedesktop.DBus");
static const QString dBusNameOwnerChangedSignal = QStringLiteral("NameOwnerChanged");


MprisManager::MprisManager(QObject *parent)
    : QObject(parent)
    , m_singleService(false)
    , m_currentController()
    , m_availableControllers()
    , m_otherPlayingControllers()
    , m_playbackStatusMapper(new QSignalMapper(this))
{
    QDBusConnection connection = QDBusConnection::sessionBus();

    if (!connection.isConnected()) {
        qmlInfo(this) << "Failed attempting to connect to DBus";
        return;
    }

    connection.connect(dBusService, dBusObjectPath, dBusInterface, dBusNameOwnerChangedSignal,
                       QStringList(), QString(),
                       this, SLOT(onNameOwnerChanged(QString, QString, QString)));

    QStringList serviceNames = connection.interface()->registeredServiceNames();
    QStringList::const_iterator i = serviceNames.constBegin();
    while (i != serviceNames.constEnd()) {
        QRegExp rx(mprisNameSpace);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(*i)) {
            onServiceAppeared(*i);
        }

        ++i;
    }
}

MprisManager::~MprisManager()
{
}

// Mpris2 Root Interface
bool MprisManager::quit()
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->quit();
}

bool MprisManager::raise()
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->raise();
}

// Mpris2 Player Interface
bool MprisManager::next()
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->next();
}

bool MprisManager::openUri(const QUrl &uri)
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->openUri(uri);
}

bool MprisManager::pause()
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->pause();
}

bool MprisManager::play()
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->play();
}

bool MprisManager::playPause()
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->playPause();
}

bool MprisManager::previous()
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->previous();
}

bool MprisManager::seek(qlonglong offset)
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->seek(offset);
}

bool MprisManager::setPosition(qlonglong position)
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->setPosition(position);
}

bool MprisManager::setPosition(const QString &trackId, qlonglong position)
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->setPosition(trackId, position);
}

bool MprisManager::stop()
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->stop();
}


// Slots

bool MprisManager::singleService() const
{
    return m_singleService;
}

void MprisManager::setSingleService(bool single)
{
    if (m_singleService == single) {
        return;
    }

    m_singleService = single;
    emit singleServiceChanged();
}

QString MprisManager::currentService() const
{
    return m_currentController.isNull() ? QString() : m_currentController->service();
}

void MprisManager::setCurrentService(const QString &service)
{
    if (!m_currentController.isNull() && m_currentController->service() == service) {
        return;
    }

    QRegExp rx(mprisNameSpace);
    rx.setPatternSyntax(QRegExp::Wildcard);
    if (!rx.exactMatch(service)) {
        qmlInfo(this) << service << "is not a proper Mpris2 service";
        return;
    }

    QSharedPointer<MprisController> controller = availableController(service);
    if (controller.isNull()) {
        controller = QSharedPointer<MprisController>(new MprisController(service, QDBusConnection::sessionBus(), this));
    } else {
        m_availableControllers.move(m_availableControllers.indexOf(controller), 0);
    }

    setCurrentController(controller);
}

QStringList MprisManager::availableServices() const
{
    QStringList result;

    QList< QSharedPointer<MprisController> >::const_iterator i = m_availableControllers.constBegin();
    while (i != m_availableControllers.constEnd()) {
        const QSharedPointer<MprisController> controller = *i;
        result.append(controller->service());

        ++i;
    }

    return result;
}

// Mpris2 Root Interface
bool MprisManager::canQuit() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->canQuit();
}

bool MprisManager::canRaise() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->canRaise();
}

bool MprisManager::canSetFullscreen() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->canSetFullscreen();
}

QString MprisManager::desktopEntry() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return QString();
    }

    return m_currentController->desktopEntry();
}

bool MprisManager::fullscreen() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->fullscreen();
}

void MprisManager::setFullscreen(bool fullscreen)
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return;
    }

    m_currentController->setFullscreen(fullscreen);
}

bool MprisManager::hasTrackList() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->hasTrackList();
}

QString MprisManager::identity() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return QString();
    }

    return m_currentController->identity();
}

QStringList MprisManager::supportedUriSchemes() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return QStringList();
    }

    return m_currentController->supportedUriSchemes();
}

QStringList MprisManager::supportedMimeTypes() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return QStringList();
    }

    return m_currentController->supportedMimeTypes();
}

// Mpris2 Player Interface
bool MprisManager::canControl() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->canControl();
}

bool MprisManager::canGoNext() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->canGoNext();
}

bool MprisManager::canGoPrevious() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->canGoPrevious();
}

bool MprisManager::canPause() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->canPause();
}

bool MprisManager::canPlay() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->canPlay();
}

bool MprisManager::canSeek() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->canSeek();
}

Mpris::LoopStatus MprisManager::loopStatus() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return Mpris::None;
    }

    return m_currentController->loopStatus();
}

void MprisManager::setLoopStatus(Mpris::LoopStatus loopStatus)
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return;
    }

    m_currentController->setLoopStatus(loopStatus);
}

double MprisManager::maximumRate() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return 1;
    }

    return m_currentController->maximumRate();
}

QVariantMap MprisManager::metadata() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return QVariantMap();
    }

    return m_currentController->metadata();
}

double MprisManager::minimumRate() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return 1;
    }

    return m_currentController->minimumRate();
}

Mpris::PlaybackStatus MprisManager::playbackStatus() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return Mpris::Stopped;
    }

    return m_currentController->playbackStatus();
}

qlonglong MprisManager::position() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return 0;
    }

    return m_currentController->position();
}

double MprisManager::rate() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return 1;
    }

    return m_currentController->rate();
}

void MprisManager::setRate(double rate)
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return;
    }

    m_currentController->setRate(rate);
}

bool MprisManager::shuffle() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return false;
    }

    return m_currentController->shuffle();
}

void MprisManager::setShuffle(bool shuffle)
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return;
    }

    m_currentController->setShuffle(shuffle);
}

double MprisManager::volume() const
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return 0;
    }

    return m_currentController->volume();
}

void MprisManager::setVolume(double volume)
{
    if (m_currentController.isNull()) {
        qDebug() << Q_FUNC_INFO << "None service available/selected";
        return;
    }

    m_currentController->setVolume(volume);
}


// Private

void MprisManager::onNameOwnerChanged(const QString &service, const QString &oldOwner, const QString& newOwner)
{
    // Unfortunately, QDBus doesn't allow flexible signal watchers.
    // Would it allow them, we could pass the filter "arg0namespace='org.mpris.MediaPlayer2'"
    // Therefore, we will receive here all the name changes in the
    // bus, not just the ones for our name space of interest, and we
    // will have to filter on our own :(

    QRegExp rx(mprisNameSpace);
    rx.setPatternSyntax(QRegExp::Wildcard);
    if (!rx.exactMatch(service)) {
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

void MprisManager::onServiceAppeared(const QString &service)
{
    QSharedPointer<MprisController> controller = availableController(service);
    if (!controller.isNull()) {
        Q_ASSERT(m_availableControllers.contains(controller));
        m_availableControllers.removeOne(controller);
        m_otherPlayingControllers.removeOne(controller);
    } else {
        if (!m_currentController.isNull() && service == m_currentController->service()) {
            controller = m_currentController;
        } else {
            controller = QSharedPointer<MprisController>(new MprisController(service, QDBusConnection::sessionBus(), this));
        }

        connect(controller.data(), SIGNAL(playbackStatusChanged()), m_playbackStatusMapper, SLOT(map()));
        m_playbackStatusMapper->setMapping(controller.data(), controller->service());
        connect(m_playbackStatusMapper, SIGNAL(mapped(QString)), this, SLOT(onAvailableControllerPlaybackStatusChanged(QString)));
    }

    if (m_currentController.isNull()) {
        setCurrentController(controller);
    }

    if (controller == m_currentController) {
        m_availableControllers.prepend(controller);
        emit availableServicesChanged();
        return;
    }

    if (!m_singleService
        && m_currentController->playbackStatus() != Mpris::Playing) {
        m_availableControllers.prepend(controller);
        setCurrentController(controller);
    } else {
        m_availableControllers.insert(1, controller);
        if (controller->playbackStatus() == Mpris::Playing) {
            m_otherPlayingControllers.prepend(controller);
        }
    }

    emit availableServicesChanged();
}

void MprisManager::onServiceVanished(const QString &service)
{
    QSharedPointer<MprisController> controller = availableController(service);
    if (!controller.isNull()) {
        Q_ASSERT(m_availableControllers.contains(controller));
        m_availableControllers.removeOne(controller);
        m_otherPlayingControllers.removeOne(controller);
    }

    if (!m_currentController.isNull() && service == m_currentController->service()) {
        if (m_singleService) {
            emit availableServicesChanged();
            return;
        }

        if (!m_availableControllers.isEmpty()) {
            setCurrentController(m_availableControllers[0]);
        } else {
            setCurrentController(QSharedPointer<MprisController>());
        }
    }

    emit availableServicesChanged();
}

void MprisManager::onAvailableControllerPlaybackStatusChanged(const QString &service)
{
    QSharedPointer<MprisController> controller = availableController(service);
    Q_ASSERT(!controller.isNull());

    if (m_currentController == controller) {
        if (m_currentController->playbackStatus() == Mpris::Playing) {
            return;
        }

        if (!m_otherPlayingControllers.isEmpty()) {
            QSharedPointer<MprisController> currentController = m_otherPlayingControllers.takeFirst();
            m_availableControllers.move(m_availableControllers.indexOf(currentController), 0);
            setCurrentController(currentController);
        }
    } else {
        if (controller->playbackStatus() != Mpris::Playing) {
            m_otherPlayingControllers.removeOne(controller);
            return;
        }

        if (!m_singleService
            && m_currentController->playbackStatus() != Mpris::Playing) {
            setCurrentController(controller);
        } else {
            m_availableControllers.move(m_availableControllers.indexOf(controller), 1);
            m_otherPlayingControllers.removeOne(controller); // Just in case, shouldn't be needed
            m_otherPlayingControllers.prepend(controller);
        }
    }
}

QSharedPointer<MprisController> MprisManager::availableController(const QString &service)
{
    QList< QSharedPointer<MprisController> >::iterator i = m_availableControllers.begin();
    while (i != m_availableControllers.end()) {
        QSharedPointer<MprisController> controller = *i;
        if (!controller.isNull() && controller->service() == service) {
            return controller;
        }

        ++i;
    }

    return QSharedPointer<MprisController>();
}

void MprisManager::setCurrentController(QSharedPointer<MprisController> controller)
{
    if (controller == m_currentController) {
        return;
    }

    if (!m_currentController.isNull()) {
        // Mpris Root Interface
        disconnect(m_currentController.data(), SIGNAL(canQuitChanged()), this, SIGNAL(canQuitChanged()));
        disconnect(m_currentController.data(), SIGNAL(canRaiseChanged()), this, SIGNAL(canRaiseChanged()));
        disconnect(m_currentController.data(), SIGNAL(canSetFullscreenChanged()), this, SIGNAL(canSetFullscreenChanged()));
        disconnect(m_currentController.data(), SIGNAL(desktopEntryChanged()), this, SIGNAL(desktopEntryChanged()));
        disconnect(m_currentController.data(), SIGNAL(fullscreenChanged()), this, SIGNAL(fullscreenChanged()));
        disconnect(m_currentController.data(), SIGNAL(hasTrackListChanged()), this, SIGNAL(hasTrackListChanged()));
        disconnect(m_currentController.data(), SIGNAL(identityChanged()), this, SIGNAL(identityChanged()));
        disconnect(m_currentController.data(), SIGNAL(supportedUriSchemesChanged()), this, SIGNAL(supportedUriSchemesChanged()));
        disconnect(m_currentController.data(), SIGNAL(supportedMimeTypesChanged()), this, SIGNAL(supportedMimeTypesChanged()));

        // Mpris Player Interface
        disconnect(m_currentController.data(), SIGNAL(canControlChanged()), this, SIGNAL(canControlChanged()));
        disconnect(m_currentController.data(), SIGNAL(canGoNextChanged()), this, SIGNAL(canGoNextChanged()));
        disconnect(m_currentController.data(), SIGNAL(canGoPreviousChanged()), this, SIGNAL(canGoPreviousChanged()));
        disconnect(m_currentController.data(), SIGNAL(canPauseChanged()), this, SIGNAL(canPauseChanged()));
        disconnect(m_currentController.data(), SIGNAL(canPlayChanged()), this, SIGNAL(canPlayChanged()));
        disconnect(m_currentController.data(), SIGNAL(canSeekChanged()), this, SIGNAL(canSeekChanged()));
        disconnect(m_currentController.data(), SIGNAL(loopStatusChanged()), this, SIGNAL(loopStatusChanged()));
        disconnect(m_currentController.data(), SIGNAL(maximumRateChanged()), this, SIGNAL(maximumRateChanged()));
        disconnect(m_currentController.data(), SIGNAL(metadataChanged()), this, SIGNAL(metadataChanged()));
        disconnect(m_currentController.data(), SIGNAL(minimumRateChanged()), this, SIGNAL(minimumRateChanged()));
        disconnect(m_currentController.data(), SIGNAL(playbackStatusChanged()), this, SIGNAL(playbackStatusChanged()));
        disconnect(m_currentController.data(), SIGNAL(rateChanged()), this, SIGNAL(rateChanged()));
        disconnect(m_currentController.data(), SIGNAL(shuffleChanged()), this, SIGNAL(shuffleChanged()));
        disconnect(m_currentController.data(), SIGNAL(volumeChanged()), this, SIGNAL(volumeChanged()));
        disconnect(m_currentController.data(), SIGNAL(seeked(qlonglong)), this, SIGNAL(seeked(qlonglong)));

        if (m_currentController->playbackStatus() == Mpris::Playing) {
            m_otherPlayingControllers.prepend(m_currentController);
        }
    }

    m_currentController = controller;

    if (!m_currentController.isNull()) {
        // Mpris Root Interface
        connect(m_currentController.data(), SIGNAL(canQuitChanged()), this, SIGNAL(canQuitChanged()));
        connect(m_currentController.data(), SIGNAL(canRaiseChanged()), this, SIGNAL(canRaiseChanged()));
        connect(m_currentController.data(), SIGNAL(canSetFullscreenChanged()), this, SIGNAL(canSetFullscreenChanged()));
        connect(m_currentController.data(), SIGNAL(desktopEntryChanged()), this, SIGNAL(desktopEntryChanged()));
        connect(m_currentController.data(), SIGNAL(fullscreenChanged()), this, SIGNAL(fullscreenChanged()));
        connect(m_currentController.data(), SIGNAL(hasTrackListChanged()), this, SIGNAL(hasTrackListChanged()));
        connect(m_currentController.data(), SIGNAL(identityChanged()), this, SIGNAL(identityChanged()));
        connect(m_currentController.data(), SIGNAL(supportedUriSchemesChanged()), this, SIGNAL(supportedUriSchemesChanged()));
        connect(m_currentController.data(), SIGNAL(supportedMimeTypesChanged()), this, SIGNAL(supportedMimeTypesChanged()));
        connect(m_currentController.data(), SIGNAL(canControlChanged()), this, SIGNAL(canControlChanged()));

        // Mpris Player Interface
        connect(m_currentController.data(), SIGNAL(canGoNextChanged()), this, SIGNAL(canGoNextChanged()));
        connect(m_currentController.data(), SIGNAL(canGoPreviousChanged()), this, SIGNAL(canGoPreviousChanged()));
        connect(m_currentController.data(), SIGNAL(canPauseChanged()), this, SIGNAL(canPauseChanged()));
        connect(m_currentController.data(), SIGNAL(canPlayChanged()), this, SIGNAL(canPlayChanged()));
        connect(m_currentController.data(), SIGNAL(canSeekChanged()), this, SIGNAL(canSeekChanged()));
        connect(m_currentController.data(), SIGNAL(loopStatusChanged()), this, SIGNAL(loopStatusChanged()));
        connect(m_currentController.data(), SIGNAL(maximumRateChanged()), this, SIGNAL(maximumRateChanged()));
        connect(m_currentController.data(), SIGNAL(metadataChanged()), this, SIGNAL(metadataChanged()));
        connect(m_currentController.data(), SIGNAL(minimumRateChanged()), this, SIGNAL(minimumRateChanged()));
        connect(m_currentController.data(), SIGNAL(playbackStatusChanged()), this, SIGNAL(playbackStatusChanged()));
        connect(m_currentController.data(), SIGNAL(rateChanged()), this, SIGNAL(rateChanged()));
        connect(m_currentController.data(), SIGNAL(shuffleChanged()), this, SIGNAL(shuffleChanged()));
        connect(m_currentController.data(), SIGNAL(volumeChanged()), this, SIGNAL(volumeChanged()));
        connect(m_currentController.data(), SIGNAL(seeked(qlonglong)), this, SIGNAL(seeked(qlonglong)));

        if (m_currentController->playbackStatus() == Mpris::Playing) {
            m_otherPlayingControllers.removeOne(m_currentController);
        }
    }

    emit currentServiceChanged();
}
