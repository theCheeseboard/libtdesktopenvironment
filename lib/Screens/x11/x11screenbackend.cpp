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
#include "x11screenbackend.h"

#include <QApplication>
#include <QX11Info>
#include <X11/extensions/Xrandr.h>
#include <xcb/randr.h>
#include "x11screen.h"

struct X11ScreenBackendPrivate {
    QMap<RROutput, SystemScreen*> screens;
    int randrEventBase, randrErrorBase;

    int dpi = 96; //TODO: find a sensible default DPI for the primary monitor
};

X11ScreenBackend::X11ScreenBackend() : ScreenBackend() {
    d = new X11ScreenBackendPrivate();

    if (this->isSuitable()) {
        qApp->installNativeEventFilter(this);
        XRRQueryExtension(QX11Info::display(), &d->randrEventBase, &d->randrErrorBase);

        this->updateDisplays();
    }
}

X11ScreenBackend::~X11ScreenBackend() {
    delete d;
}

bool X11ScreenBackend::isSuitable() {
    return QX11Info::isPlatformX11();
}

QList<SystemScreen*> X11ScreenBackend::screens() {
    return d->screens.values();
}

SystemScreen* X11ScreenBackend::primaryScreen() {
    for (SystemScreen* screen : d->screens.values()) {
        if (screen->isPrimary()) return screen;
    }
    return nullptr;
}

int X11ScreenBackend::dpi() const {
    return d->dpi;
}

void X11ScreenBackend::setDpi(int dpi) {
    d->dpi = dpi;

    QRect rect;
    for (SystemScreen* screen : this->screens()) {
        rect = rect.united(screen->geometry());
    }
    XRRSetScreenSize(QX11Info::display(), QX11Info::appRootWindow(), rect.width(), rect.height(), qRound((25.4 * rect.width()) / dpi), qRound((25.4 * rect.height()) / dpi));
}

void X11ScreenBackend::updateDisplays() {
    //Update all the displays
    XRRMonitorInfo* monitorInfo;
    int monitorCount;

    XRRScreenResources* resources = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    monitorInfo = XRRGetMonitors(QX11Info::display(), QX11Info::appRootWindow(), false, &monitorCount);

    for (int i = 0; i < resources->noutput; i++) {
        RROutput output = resources->outputs[i];
        XRROutputInfo* newOutputInfo = XRRGetOutputInfo(QX11Info::display(), resources, resources->outputs[i]);
        if (newOutputInfo->connection == RR_Connected) {
            //Make sure this output is available and update the information on it
            if (!d->screens.contains(output)) d->screens.insert(output, new X11Screen(output));
            static_cast<X11Screen*>(d->screens.value(output))->updateScreen();
        } else {
            //Make sure this output is unavailable
            if (d->screens.contains(output)) {
                SystemScreen* screen = d->screens.value(output);
                d->screens.remove(output);
                screen->deleteLater();
            }
        }
        XRRFreeOutputInfo(newOutputInfo);
    }

    XRRFreeScreenResources(resources);
    XRRFreeMonitors(monitorInfo);
}

bool X11ScreenBackend::nativeEventFilter(const QByteArray& eventType, void* message, long* result) {
    Q_UNUSED(result)

    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
        if (event->response_type == d->randrEventBase + XCB_RANDR_NOTIFY) {
            //RandR has changed, update all the displays
            this->updateDisplays();
        }
    }
    return false;
}
