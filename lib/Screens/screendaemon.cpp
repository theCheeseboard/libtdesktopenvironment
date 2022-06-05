/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
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
#include "screendaemon.h"

#include "private/screenbackend.h"
#include <QDebug>

#ifdef HAVE_X11
    #include "x11/x11screenbackend.h"
#endif

#ifdef HAVE_WAYLAND
    #include "wayland/waylandscreenbackend.h"
#endif

struct ScreenDaemonPrivate {
        ScreenDaemon* instance = nullptr;
        ScreenBackend* backend = nullptr;
};

ScreenDaemonPrivate* ScreenDaemon::d = new ScreenDaemonPrivate();

ScreenDaemon* ScreenDaemon::instance() {
    if (!d->instance) d->instance = new ScreenDaemon();
    return d->instance;
}

QList<SystemScreen*> ScreenDaemon::screens() {
    return d->backend->screens();
}

SystemScreen* ScreenDaemon::primayScreen() {
    return d->backend->primaryScreen();
}

int ScreenDaemon::dpi() const {
    return d->backend->dpi();
}

void ScreenDaemon::setDpi(int dpi) {
    d->backend->setDpi(dpi);
}

ScreenDaemon::ScreenDaemon() :
    QObject(nullptr) {
    // Figure out the best backend to use
#ifdef HAVE_X11
    if (X11ScreenBackend::isSuitable()) {
        d->backend = new X11ScreenBackend();
    }
#endif

#ifdef HAVE_WAYLAND
    if (WaylandScreenBackend::isSuitable()) {
        d->backend = new WaylandScreenBackend();
    }
#endif

    if (d->backend) {
        connect(d->backend, &ScreenBackend::screensUpdated, this, &ScreenDaemon::screensUpdated);
    } else {
        // No suitable backend is available
        qWarning() << "No suitable backend for ScreenDaemon";
    }
}
