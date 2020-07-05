/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2019 Victor Tran
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
#include "desktopwm.h"

#include <QDebug>
#include <unistd.h>
#include <pwd.h>
#include "private/wmbackend.h"

#ifdef HAVE_X11
    #include "x11/x11backend.h"
#endif

struct DesktopWmPrivate {
    DesktopWm* dwmInstance = nullptr;
    WmBackend* instance = nullptr;
};

DesktopWmPrivate* DesktopWm::d = new DesktopWmPrivate();

DesktopWm* DesktopWm::instance() {
    if (d->dwmInstance == nullptr) d->dwmInstance = new DesktopWm();
    return d->dwmInstance;
}

DesktopAccessibility* DesktopWm::accessibility() {
    return d->instance->accessibility();
}

QList<DesktopWmWindowPtr> DesktopWm::openWindows() {
    return d->instance->openWindows();
}

DesktopWmWindowPtr DesktopWm::activeWindow() {
    return d->instance->activeWindow();
}

QStringList DesktopWm::desktops() {
    return d->instance->desktops();
}

uint DesktopWm::currentDesktop() {
    return d->instance->currentDesktop();
}

void DesktopWm::setCurrentDesktop(uint desktopNumber) {
    d->instance->setCurrentDesktop(desktopNumber);
}

void DesktopWm::setShowDesktop(bool showDesktop) {
    d->instance->setShowDesktop(showDesktop);
}

void DesktopWm::setSystemWindow(QWidget* widget) {
    d->instance->setSystemWindow(widget);
}

void DesktopWm::setSystemWindow(QWidget* widget, DesktopWm::SystemWindowType windowType) {
    d->instance->setSystemWindow(widget, windowType);
}

void DesktopWm::blurWindow(QWidget* widget) {
    d->instance->blurWindow(widget);
}

void DesktopWm::setScreenMarginForWindow(QWidget* widget, QScreen* screen, Qt::Edge edge, int width) {
    d->instance->setScreenMarginForWindow(widget, screen, edge, width);
}

void DesktopWm::setScreenOff(bool screenOff) {
    d->instance->setScreenOff(screenOff);
}

bool DesktopWm::isScreenOff() {
    return d->instance->isScreenOff();
}

quint64 DesktopWm::msecsIdle() {
    return d->instance->msecsIdle();
}

quint64 DesktopWm::grabKey(Qt::Key key, Qt::KeyboardModifiers modifiers) {
    return d->instance->grabKey(key, modifiers);
}

void DesktopWm::ungrabKey(quint64 grab) {
    d->instance->ungrabKey(grab);
}

QString DesktopWm::displayName(int uid) {
    passwd* pwEntry = getpwuid(uid);
    QStringList gecosField = QString::fromLocal8Bit(pwEntry->pw_gecos).split(",");
    if (!gecosField.at(0).isEmpty()) {
        return gecosField.at(0);
    }
    return pwEntry->pw_name;
}

QString DesktopWm::userDisplayName() {
    return displayName(getuid());
}

DesktopWm::DesktopWm() : QObject(nullptr) {
    //Figure out the best backend to use
#ifdef HAVE_X11
    if (X11Backend::isSuitable()) {
        d->instance = new X11Backend();
    }
#endif

    if (d->instance) {
        connect(d->instance, &WmBackend::windowAdded, this, &DesktopWm::windowAdded);
        connect(d->instance, &WmBackend::windowRemoved, this, &DesktopWm::windowRemoved);
        connect(d->instance, &WmBackend::activeWindowChanged, this, &DesktopWm::activeWindowChanged);
        connect(d->instance, &WmBackend::currentDesktopChanged, this, &DesktopWm::currentDesktopChanged);
        connect(d->instance, &WmBackend::desktopCountChanged, this, &DesktopWm::desktopCountChanged);
        connect(d->instance, &WmBackend::grabbedKeyPressed, this, &DesktopWm::grabbedKeyPressed);
    } else {
        //No suitable backend is available
        qWarning() << "No suitable backend for DesktopWm";
    }
}
