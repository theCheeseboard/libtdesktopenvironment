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

#include <QRect>
#include <QIcon>
#include "waylandbackend.h"
#include "wlr-foreign-toplevel-management-unstable-v1-proto.h"

struct WaylandWindowPrivate {
    zwlr_foreign_toplevel_handle_v1* handle;
    WaylandBackend* backend;
    WaylandWindowEventListener* listener;

    QString title;
    ApplicationPointer application;
};

struct WaylandWindowEventListener {
    WaylandWindow* parent;

    WaylandWindowEventListener(WaylandWindow* parentWindow) {
        this->parent = parentWindow;
    }

    void titleChanged(QString title) {
        this->parent->d->title = title;
        emit this->parent->titleChanged();
    }

    void applicationChanged(QString application) {
        ApplicationPointer newApp(new Application(application));
        if (newApp->isValid()) {
            this->parent->d->application = newApp;
        } else {
            this->parent->d->application.clear();
        }

        emit this->parent->applicationChanged();
        emit this->parent->iconChanged();
    }

    void closed() {
        this->parent->d->backend->signalToplevelClosed(this->parent->d->handle);
    }
};

WaylandWindow::WaylandWindow(zwlr_foreign_toplevel_handle_v1* handle, WaylandBackend* backend) : DesktopWmWindow() {
    d = new WaylandWindowPrivate();
    d->handle = handle;
    d->backend = backend;

    d->listener = new WaylandWindowEventListener(this);

    zwlr_foreign_toplevel_handle_v1_listener* listener = new zwlr_foreign_toplevel_handle_v1_listener();
    listener->title = [](void* data, struct zwlr_foreign_toplevel_handle_v1 * zwlr_foreign_toplevel_handle_v1, const char* title) {
        Q_UNUSED(zwlr_foreign_toplevel_handle_v1)
        static_cast<WaylandWindowEventListener*>(data)->titleChanged(QString::fromLocal8Bit(title));
    };
    listener->app_id = [](void* data, struct zwlr_foreign_toplevel_handle_v1 * zwlr_foreign_toplevel_handle_v1, const char* app_id) {
        Q_UNUSED(zwlr_foreign_toplevel_handle_v1)
        static_cast<WaylandWindowEventListener*>(data)->applicationChanged(QString::fromLocal8Bit(app_id));
    };
    listener->output_enter = [](void* data, struct zwlr_foreign_toplevel_handle_v1 * zwlr_foreign_toplevel_handle_v1, struct wl_output * output) {

    };
    listener->output_leave = [](void* data, struct zwlr_foreign_toplevel_handle_v1 * zwlr_foreign_toplevel_handle_v1, struct wl_output * output) {

    };
    listener->state = [](void* data, struct zwlr_foreign_toplevel_handle_v1 * zwlr_foreign_toplevel_handle_v1, struct wl_array * state) {

    };
    listener->done = [](void* data, struct zwlr_foreign_toplevel_handle_v1 * zwlr_foreign_toplevel_handle_v1) {

    };
    listener->closed = [](void* data, struct zwlr_foreign_toplevel_handle_v1 * zwlr_foreign_toplevel_handle_v1) {
        Q_UNUSED(zwlr_foreign_toplevel_handle_v1)
        static_cast<WaylandWindowEventListener*>(data)->closed();
    };
    listener->parent = [](void* data, struct zwlr_foreign_toplevel_handle_v1 * zwlr_foreign_toplevel_handle_v1, struct zwlr_foreign_toplevel_handle_v1 * parent) {

    };
    zwlr_foreign_toplevel_handle_v1_add_listener(handle, listener, d->listener);
    wl_display_roundtrip(backend->display());
}

WaylandWindow::~WaylandWindow() {
    delete d->listener;
    delete d;
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
    return false;
}

bool WaylandWindow::isMaximised() {
    return false;
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
    zwlr_foreign_toplevel_handle_v1_activate(d->handle, seat);
}

void WaylandWindow::close() {
    zwlr_foreign_toplevel_handle_v1_close(d->handle);
}

void WaylandWindow::kill() {
}
