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
#include "waylandwmconstants.h"
#include <QCoroDBus>
#include <QDBusInterface>
#include <QIcon>
#include <QRect>
#include <signal.h>

struct WaylandWindowPrivate {
        WaylandBackend* backend;
        WaylandWindowEventListener* listener;

        QString title;
        ApplicationPointer application;
        quint64 pid;
        bool active;
        bool maximised;
        bool minimised;
        bool fullScreen;

        uint viewId;

        QDBusInterface* interface;
};

struct WaylandWindowEventListener {
        WaylandWindow* parent;

        WaylandWindowEventListener(WaylandWindow* parentWindow) {
            this->parent = parentWindow;
        }
};

WaylandWindow::WaylandWindow(uint viewId, WaylandBackend* backend) :
    DesktopWmWindow() {
    d = new WaylandWindowPrivate();
    d->backend = backend;
    d->viewId = viewId;

    d->listener = new WaylandWindowEventListener(this);
    d->interface = new QDBusInterface(WaylandWmConstants::serviceName, WaylandWmConstants::servicePath, "wayland.compositor.views", QDBusConnection::sessionBus(), this);

    // clang-format off
    QDBusConnection::sessionBus().connect(WaylandWmConstants::serviceName, WaylandWmConstants::servicePath, "wayland.compositor", "ViewTitleChanged", this, SLOT(viewTitleChanged(uint,QString)));
    QDBusConnection::sessionBus().connect(WaylandWmConstants::serviceName, WaylandWmConstants::servicePath, "wayland.compositor", "ViewAppIdChanged", this, SLOT(viewAppIdChanged(uint,QString)));
    QDBusConnection::sessionBus().connect(WaylandWmConstants::serviceName, WaylandWmConstants::servicePath, "wayland.compositor", "ViewFocusChanged", this, SLOT(viewFocusChanged(uint,bool)));
    QDBusConnection::sessionBus().connect(WaylandWmConstants::serviceName, WaylandWmConstants::servicePath, "wayland.compositor", "ViewMaximizedChanged", this, SLOT(viewMaximizedChanged(uint,bool)));
    QDBusConnection::sessionBus().connect(WaylandWmConstants::serviceName, WaylandWmConstants::servicePath, "wayland.compositor", "ViewMinimizedChanged", this, SLOT(viewMinimizedChanged(uint,bool)));
    this->updateWindow();
    // clang-format on
}

WaylandWindow::~WaylandWindow() {
    delete d->listener;
    delete d;
}

bool WaylandWindow::isActive() {
    return d->active;
}

void WaylandWindow::viewTitleChanged(uint viewId, QString title) {
    if (d->viewId == viewId) {
        d->title = title;
        emit titleChanged();
    }
}

void WaylandWindow::viewAppIdChanged(uint viewId, QString appId) {
    if (d->viewId == viewId) {
        ApplicationPointer app(new Application(appId));
        if (app->isValid()) {
            d->application = app;
            emit applicationChanged();
            return;
        }

        QStringList applications = Application::allApplications();
        for (const QString& desktopEntry : qAsConst(applications)) {
            ApplicationPointer app(new Application(desktopEntry));
            if (appId == app->getProperty("StartupWMClass").toString() || appId == desktopEntry || appId.toLower() == desktopEntry.toLower()) {
                d->application = app;
                emit applicationChanged();
                return;
            }
        }

        d->application.clear();
        emit applicationChanged();
    }
}

void WaylandWindow::viewFocusChanged(uint viewId, bool haveFocus) {
    if (d->viewId == viewId) {
        d->active = true;
    } else {
        d->active = false;
    }
    emit windowStateChanged();
}

void WaylandWindow::viewMaximizedChanged(uint viewId, bool maximized) {
    if (d->viewId == viewId) {
        d->maximised = maximized;
        emit windowStateChanged();
    }
}

void WaylandWindow::viewMinimizedChanged(uint viewId, bool minimized) {
    if (d->viewId == viewId) {
        d->minimised = minimized;
        emit windowStateChanged();
    }
}

QCoro::Task<> WaylandWindow::updateWindow() {
    // This one needs to be synchronous because we need to have this set before we return
    auto application = d->interface->call("QueryViewAppId", d->viewId);
    this->viewAppIdChanged(d->viewId, application.arguments().constFirst().toString());

    auto title = co_await d->interface->asyncCall("QueryViewTitle", d->viewId);
    this->viewTitleChanged(d->viewId, title.arguments().constFirst().toString());

    auto pid = co_await d->interface->asyncCall("QueryViewPid", d->viewId);
    d->pid = pid.arguments().constFirst().toUInt();

    auto active = co_await d->interface->asyncCall("QueryViewActive", d->viewId);
    this->viewFocusChanged(d->viewId, active.arguments().constFirst().toBool());

    auto maximised = co_await d->interface->asyncCall("QueryViewMaximized", d->viewId);
    this->viewMaximizedChanged(d->viewId, maximised.arguments().constFirst().toBool());

    auto minimised = co_await d->interface->asyncCall("QueryViewMinimized", d->viewId);
    this->viewMinimizedChanged(d->viewId, minimised.arguments().constFirst().toBool());
    co_return;
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
    return d->minimised;
}

bool WaylandWindow::isMaximised() {
    return d->maximised;
}

bool WaylandWindow::isFullScreen() {
    return d->fullScreen;
}

bool WaylandWindow::shouldShowInTaskbar() {
    return true;
}

quint64 WaylandWindow::pid() {
    return d->pid;
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
    d->interface->asyncCall("FocusView", d->viewId);
}

void WaylandWindow::kill() {
    ::kill(d->pid, SIGKILL);
}

void WaylandWindow::close() {
    d->interface->asyncCall("CloseView", d->viewId);
}

bool WaylandWindow::isOnDesktop(uint desktop) {
    return desktop == 0;
}
