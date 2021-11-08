// -*- c++ -*-

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
 */

#ifndef MPRISMETADATA_H
#define MPRISMETADATA_H

#include <QObject>
#include <QVariant>
#include <mpris.h>
#include <ambermpris.h>

namespace Amber {
class MprisMetaDataPrivate;

class AMBER_MPRIS_EXPORT MprisMetaData : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant trackId READ trackId WRITE setTrackId NOTIFY metaDataChanged)

    Q_PROPERTY(QVariant duration READ duration WRITE setDuration NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant artUrl READ artUrl WRITE setArtUrl NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant albumTitle READ albumTitle WRITE setAlbumTitle NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant albumArtist READ albumArtist WRITE setAlbumArtist NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant contributingArtist READ contributingArtist WRITE setContributingArtist NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant lyrics READ lyrics WRITE setLyrics NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant audioBpm READ audioBpm WRITE setAudioBpm NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant autoRating READ autoRating WRITE setAutoRating NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant comment READ comment WRITE setComment NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant composer READ composer WRITE setComposer NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant year READ year WRITE setYear NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant date READ date WRITE setDate NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant discNumber READ discNumber WRITE setDiscNumber NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant firstUsed READ firstUsed WRITE setFirstUsed NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant genre READ genre WRITE setGenre NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant lastUsed READ lastUsed WRITE setLastUsed NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant writer READ writer WRITE setWriter NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant title READ title WRITE setTitle NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant trackNumber READ trackNumber WRITE setTrackNumber NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant url READ url WRITE setUrl NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant useCount READ useCount WRITE setUseCount NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant userRating READ userRating WRITE setUserRating NOTIFY metaDataChanged)

    Q_PROPERTY(QVariantMap extraFields READ extraFields WRITE setExtraFields NOTIFY metaDataChanged)
    Q_PROPERTY(QVariant fillFrom READ fillFrom WRITE setFillFrom NOTIFY fillFromChanged)

public:
    MprisMetaData(QObject *parent = 0);
    MprisMetaData();

    QVariant trackId() const;
    void setTrackId(const QVariant &trackId);

    QVariant duration() const;
    void setDuration(const QVariant &duration);
    QVariant artUrl() const;
    void setArtUrl(const QVariant &url);
    QVariant albumTitle() const;
    void setAlbumTitle(const QVariant &title);
    QVariant albumArtist() const;
    void setAlbumArtist(const QVariant &artist);
    QVariant contributingArtist() const;
    void setContributingArtist(const QVariant &artist);
    QVariant lyrics() const;
    void setLyrics(const QVariant &lyrics);

    QVariant comment() const;
    void setComment(const QVariant &comment);
    QVariant composer() const;
    void setComposer(const QVariant &composer);
    QVariant year() const;
    void setYear(const QVariant &year);
    QVariant date() const;
    void setDate(const QVariant &date);
    QVariant discNumber() const;
    void setDiscNumber(const QVariant &disc);

    QVariant genre() const;
    void setGenre(const QVariant &genre);

    QVariant writer() const;
    void setWriter(const QVariant &writer);

    QVariant title() const;
    void setTitle(const QVariant &title);
    QVariant trackNumber() const;
    void setTrackNumber(const QVariant &track);

    QVariant userRating() const;
    void setUserRating(const QVariant &rating);

    QVariant audioBpm() const;
    void setAudioBpm(const QVariant &bpm);
    QVariant autoRating() const;
    void setAutoRating(const QVariant &rating);
    QVariant firstUsed() const;
    void setFirstUsed(const QVariant &used);
    QVariant lastUsed() const;
    void setLastUsed(const QVariant &used);
    QVariant url() const;
    void setUrl(const QVariant &url);
    QVariant useCount() const;
    void setUseCount(const QVariant &count);

    QVariantMap extraFields() const;
    void setExtraFields(const QVariantMap &fields);

    QVariant fillFrom() const;
    void setFillFrom(const QVariant &fillFrom);

Q_SIGNALS:
    void metaDataChanged();
    void fillFromChanged();

private:
    MprisMetaDataPrivate *priv;
    friend class MprisPlayerPrivate;
    friend class MprisClientPrivate;
};
}
#endif
