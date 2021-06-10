/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "waylandscreenbackend.h"

#include <QGuiApplication>

WaylandScreenBackend::WaylandScreenBackend() : ScreenBackend() {

}

bool WaylandScreenBackend::isSuitable() {
    return QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive);
}

QList<SystemScreen*> WaylandScreenBackend::screens() {
    //TODO: Implement
    return {};
}

SystemScreen* WaylandScreenBackend::primaryScreen() {
    //TODO: Implement
    return nullptr;
}

int WaylandScreenBackend::dpi() const {
    //TODO: Implement
    return 96;
}

void WaylandScreenBackend::setDpi(int dpi) {
    //TODO: Implement
}
