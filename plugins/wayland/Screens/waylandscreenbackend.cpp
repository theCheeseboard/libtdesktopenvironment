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

#include "twaylandregistry.h"
#include "waylandscreen.h"
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <tlogger.h>

struct WaylandScreenBackendPrivate {
        quint32 serial;

        QMap<zwlr_output_head_v1*, WaylandScreen*> heads;
};

WaylandScreenBackend::WaylandScreenBackend() :
    ScreenBackend() {
    d = new WaylandScreenBackendPrivate();

    tWaylandRegistry registry;
    if (!registry.init<QtWayland::zwlr_output_manager_v1>(this)) {
        tWarn("WaylandScreenBackend") << "The compositor doesn't support the wlr-output-management protocol";
    }

    auto display = reinterpret_cast<wl_display*>(qApp->platformNativeInterface()->nativeResourceForIntegration("display"));
    wl_display_roundtrip(display);
}

WaylandScreenBackend::~WaylandScreenBackend() {
    delete d;
}

bool WaylandScreenBackend::isSuitable() {
    return QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive);
}

uint32_t WaylandScreenBackend::serial() {
    return d->serial;
}

QList<SystemScreen*> WaylandScreenBackend::screens() {
    QList<SystemScreen*> screens;
    for (WaylandScreen* screen : d->heads.values()) {
        screens.append(screen);
    }
    return screens;
}

SystemScreen* WaylandScreenBackend::primaryScreen() {
    // TODO: Implement
    if (d->heads.isEmpty()) return nullptr;
    return d->heads.first();
}

int WaylandScreenBackend::dpi() const {
    // TODO: Implement
    return 96;
}

void WaylandScreenBackend::setDpi(int dpi) {
    // TODO: Implement
}

void WaylandScreenBackend::zwlr_output_manager_v1_head(zwlr_output_head_v1* head) {
    d->heads.insert(head, new WaylandScreen(head, this));
}

void WaylandScreenBackend::zwlr_output_manager_v1_done(uint32_t serial) {
    d->serial = serial;
}

void WaylandScreenBackend::zwlr_output_manager_v1_finished() {
}
