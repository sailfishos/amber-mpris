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


import QtQuick 2.0
import Amber.Mpris 1.0

Item {
    id: controls

    property MprisController mprisController
    property bool isPlaying: mprisController.playbackStatus == Mpris.Playing

    height: parent.height
    width: column.width

    Column {
        id: column

        Text {
            id: artistLabel

            text: (mprisController.metaData.contributingArtist || []).join(', ')
            width: parent.width
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            id: songLabel

            text: mprisController.metaData.title || ''
            width: parent.width
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
        }

        Row {

            MouseArea {

                width: controls.parent.width * 0.25
                height: width

                onClicked: if (mprisController.canGoPrevious) mprisController.previous()

                Text {
                    anchors.centerIn: parent
                    text: "⏮"
                }
            }
            MouseArea {

                width: controls.parent.width * 0.25
                height: width

                onClicked: if (mprisController.canPlay || mprisController.canPause) mprisController.playPause()

                Text {
                    anchors.centerIn: parent
                    text: controls.isPlaying ? "⏸" : "⏵"
                }
            }
            MouseArea {

                width: controls.parent.width * 0.25
                height: width

                onClicked: if (mprisController.canGoNext) mprisController.next()

                Text {
                    anchors.centerIn: parent
                    text: "⏭"
                }
            }
        }
    }
}

