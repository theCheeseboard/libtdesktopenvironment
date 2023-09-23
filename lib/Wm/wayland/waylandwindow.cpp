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
#include "waylandwindow.h"

#include "waylandbackend.h"
#include <QIcon>
#include <QRect>

struct WaylandWindowPrivate {
        WaylandBackend* backend;
        WaylandWindowEventListener* listener;

        QString title;
        ApplicationPointer application;

        enum WindowState {
            NoState = 0,
            Activated = 1,
            Maximised = 2,
            Minimised = 4
        };
        typedef QFlags<WindowState> WindowStateFlags;

        WindowStateFlags state = NoState;
};

struct WaylandWindowEventListener {
        WaylandWindow* parent;

        WaylandWindowEventListener(WaylandWindow* parentWindow) {
            this->parent = parentWindow;
        }
};

WaylandWindow::WaylandWindow(::zwlr_foreign_toplevel_handle_v1* handle, WaylandBackend* backend) :
    DesktopWmWindow(), QtWayland::zwlr_foreign_toplevel_handle_v1(handle) {
    d = new WaylandWindowPrivate();
    d->backend = backend;

    d->listener = new WaylandWindowEventListener(this);
    wl_display_roundtrip(backend->display());
}

WaylandWindow::~WaylandWindow() {
    delete d->listener;
    delete d;
}

bool WaylandWindow::isActive() {
    return d->state & WaylandWindowPrivate::Activated;
}

QString WaylandWindow::title() {
    return d->title;
}

QRect WaylandWindow::geometry() {
    return QRect();
}

QIcon WaylandWindow::icon() {
    if (d->application) return d->application->icon();
    return QIcon();
}

bool WaylandWindow::isMinimized() {
    return d->state & WaylandWindowPrivate::Minimised;
}

bool WaylandWindow::isMaximised() {
    return d->state & WaylandWindowPrivate::Maximised;
}

bool WaylandWindow::isFullScreen() {
    return false;
}

bool WaylandWindow::shouldShowInTaskbar() {
    return true;
}

quint64 WaylandWindow::pid() {
    return 0;
}

uint WaylandWindow::desktop() {
    return 0;
}

bool WaylandWindow::isOnCurrentDesktop() {
    return true;
}

void WaylandWindow::moveToDesktop(uint desktop) {
}

ApplicationPointer WaylandWindow::application() {
    return d->application;
}

void WaylandWindow::activate() {
    wl_seat* seat = d->backend->seat();
    this->QtWayland::zwlr_foreign_toplevel_handle_v1::activate(seat);
}

void WaylandWindow::kill() {
}

void WaylandWindow::zwlr_foreign_toplevel_handle_v1_title(const QString& title) {
    d->title = title;
    emit titleChanged();
}

void WaylandWindow::zwlr_foreign_toplevel_handle_v1_app_id(const QString& app_id) {
    ApplicationPointer newApp(new Application(app_id));
    if (newApp->isValid()) {
        d->application = newApp;
    } else {
        d->application.clear();
    }

    emit applicationChanged();
    emit iconChanged();
}

void WaylandWindow::zwlr_foreign_toplevel_handle_v1_state(wl_array* state) {
    WaylandWindowPrivate::WindowStateFlags windowState = WaylandWindowPrivate::NoState;
    for (quint32* flag = static_cast<quint32*>(state->data); reinterpret_cast<char*>(flag) < (static_cast<char*>(state->data) + state->size); flag++) {
        if (*flag == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED) {
            windowState |= WaylandWindowPrivate::Activated;
        }
        if (*flag == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED) {
            windowState |= WaylandWindowPrivate::Maximised;
        }
        if (*flag == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED) {
            windowState |= WaylandWindowPrivate::Minimised;
        }
    }

    d->state = windowState;
    emit windowStateChanged();
    emit d->backend->activeWindowChanged();
}

void WaylandWindow::zwlr_foreign_toplevel_handle_v1_closed() {
    d->backend->signalToplevelClosed(this->object());
    d->backend->activeWindowChanged();
}

void WaylandWindow::close() {
    this->QtWayland::zwlr_foreign_toplevel_handle_v1::close();
}

bool WaylandWindow::isOnDesktop(uint desktop) {
    return desktop == 0;
}
