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

import QtQuick 2.0
import QtMultimedia 5.6
import Amber.Mpris 1.0

MprisPlayer {
    property Audio player: parent
    property Playlist playlist: player ? player.playlist : null
    property bool seekDetection: true
    property int _oldLoopStatus: Playlist.Sequential
    property var _oldPosition: 0
    property var _oldStamp

    playbackStatus: {
        switch (player.playbackState) {
        case Audio.PlayingState:
            return Mpris.Playing
        case Audio.PausedState:
            return Mpris.Paused
        default:
            return Mpris.Stopped
        }
    }

    canControl: true

    canGoNext: playlist && playlist.currentIndex < playlist.itemCount - 1 || loopStatus != Mpris.LoopNone
    canGoPrevious: playlist && playlist.currentIndex > 0 || loopStatus != Mpris.LoopNone

    canPlay: player.playbackState != Mpris.Playing && (player.source || playlist.itemCount)
    canPause: player.playbackState == Mpris.Playing

    canSeek: player.seekable
    metaData {
        url: player.source
        trackId: playlist && playlist.currentIndex >= 0 ? playlist.currentIndex : null
        duration: player.metaData.duration || player.duration
        coverArtUrlSmall: player.metaData.coverArtUrlSmall
        coverArtUrlLarge: player.metaData.coverArtUrlLarge
        albumTitle: player.metaData.albumTitle
        albumArtist: player.metaData.albumArtist
        contributingArtist: player.metaData.contributingArtist
        lyrics: player.metaData.lyrics
        comment: player.metaData.comment
        composer: player.metaData.composer
        year: player.metaData.year
        date: player.metaData.date
        chapterNumber: player.metaData.chapterNumber
        genre: player.metaData.genre
        writer: player.metaData.writer
        title: player.metaData.title
        trackNumber: player.metaData.trackNumber
        userRating: player.metaData.userRating
    }

    loopStatus: {
        if (!playlist) {
            return Mpris.LoopNone
        } else if (playlist.playbackMode == Playlist.Random) {
            return Mpris.LoopPlaylist
        }

        _oldLoopStatus = playlist.playbackMode

        if (playlist.playbackMode == Playlist.CurrentItemInLoop) {
            return Mpris.LoopTrack
        } else if (playlist.playbackMode == Playlist.Loop) {
            return Mpris.LoopPlaylist
        } else {
            return Mpris.LoopNone
        }
    }

    shuffle: playlist ? playlist.playbackMode == Playlist.Random : false
    rate: player.playbackRate
    volume: player.muted ? 0 : player.volume
    onPlayRequested: player.play()
    onStopRequested: player.stop()
    onPauseRequested: player.pause()
    onPlayPauseRequested: if (player.playbackState == Audio.PlayingState) { player.pause() } else { player.play() }
    onNextRequested: if (playlist) { playlist.next() }
    onPreviousRequested: if (playlist) { playlist.previous() }
    onShuffleRequested: if (playlist) { playlist.playbackMode = (shuffle ? Playlist.Random : _oldLoopStatus) }
    onLoopStatusRequested: {
        if (!playlist) {
            return
        }
        if (loopStatus == Mpris.LoopTrack) {
            playlist.playbackMode = Playlist.CurrentItemInLoop
        } else if (loopStatus == Mpris.LoopPlaylist) {
            playlist.playbackMode = Playlist.Loop
        } else {
            playlist.playbackMode = Playlist.Sequential
        }
    }

    onSetPositionRequested: {
        if (!playlist || playlist.currentIndex == trackId) {
            player.seek(position)
            if (!seekDetection) {
                seeked(player.position)
            }
        }
    }

    onSeekRequested: {
        if (playlist && player.metaData.duration && player.position + offset >= player.metaData.duration) {
            playlist.next()
        } else {
            player.seek(player.position + offset)
            if (!seekDetection) {
                seeked(player.position)
            }
        }
    }

    onPlaybackStatusChanged: {
        if (!seekDetection) {
            return;
        }

        _oldPosition = player.position
        _oldStamp = Date.now()
    }

    property Connections _connections: Connections {
        target: player
        onPositionChanged: {
            if (!seekDetection) {
                return;
            }
            var stamp = Date.now();
            if ((playbackStatus != Mpris.Playing && position != _oldPosition)
                || (_oldStamp
                    && Math.abs((player.position - _oldPosition) - (stamp - _oldStamp) * rate) > 100)) {
                seeked(player.position)
            }

            _oldStamp = stamp
            _oldPosition = player.position
        }

        onSourceChanged: {
            if (!seekDetection) {
                return;
            }
            _oldPosition = player.position
            _oldStamp = Date.now()
        }
    }

    onPositionRequested: position = player.position
}
