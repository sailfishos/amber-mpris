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
    id: mainItem

    anchors.fill: parent

    Loader {
        id: controlsLoader

        active: mprisController.availableServices.length > 0

        Component.onCompleted: setSource("MprisControls.qml", { "mprisController": mprisController, "parent": mainItem })

        MprisController {
            id: mprisController

            onIdentityChanged: console.log(identity)
            onCurrentServiceChanged: console.log(currentService)
        }
    }
}
