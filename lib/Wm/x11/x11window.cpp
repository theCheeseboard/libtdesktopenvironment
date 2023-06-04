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

#include <QDebug>
#include <QRect>

#include "../desktopwm.h"
#include <QIcon>
#include <QTimer>
#include <QtEndian>

#include "x11functions.h"
#include "x11window.h"
#include <X11/Xutil.h>
#include <signal.h>

#undef Above
#undef Below

struct X11WindowPrivate {
        Window wid;
        QMap<QString, std::function<void()>> propertyChangeEvents;

        QIcon windowIcon;
        bool iconNeedsUpdate = true;

        ApplicationPointer application;

        enum State : uint {
            NoState = 0,
            Modal = 1 << 0,
            Sticky = 1 << 1,
            MaximizedVert = 1 << 2,
            MaximizedHorz = 1 << 3,
            Shaded = 1 << 4,
            SkipTaskbar = 1 << 5,
            SkipPager = 1 << 6,
            Hidden = 1 << 7,
            Fullscreen = 1 << 8,
            Above = 1 << 9,
            Below = 1 << 10,
            DemandsAttention = 1 << 11
        };

        State windowState = NoState;
};

X11WindowPrivate::State& operator|=(X11WindowPrivate::State& a, X11WindowPrivate::State b) {
    X11WindowPrivate::State newState = static_cast<X11WindowPrivate::State>(static_cast<uint>(a) | static_cast<uint>(b));
    a = newState;
    return a;
}

X11Window::X11Window(Window wid) :
    DesktopWmWindow() {
    d = new X11WindowPrivate();
    d->wid = wid;

    XWindowAttributes attrs;
    XGetWindowAttributes(tX11Info::display(), d->wid, &attrs);
    XSelectInput(tX11Info::display(), d->wid, attrs.your_event_mask | PropertyChangeMask | StructureNotifyMask | SubstructureNotifyMask);

    d->propertyChangeEvents.insert("_NET_WM_NAME", [this] {
        emit titleChanged();
    });
    d->propertyChangeEvents.insert("_NET_WM_ICON", [this] {
        d->iconNeedsUpdate = true;
        emit iconChanged();
    });
    d->propertyChangeEvents.insert("_NET_WM_STATE", [this] {
        this->updateState();
        emit windowStateChanged();
    });
    d->propertyChangeEvents.insert("_NET_WM_WINDOW_TYPE", [this] {
        emit windowStateChanged();
    });

    d->propertyChangeEvents.insert("_NET_WM_DESKTOP", [this] {
        emit desktopChanged();
    });
    this->updateState();
}

X11Window::~X11Window() {
    delete d;
}

void X11Window::x11PropertyChanged(QString property) {
    if (d->propertyChangeEvents.contains(property)) d->propertyChangeEvents.value(property)();
}

void X11Window::configureNotify() {
    emit geometryChanged();
}

QString X11Window::title() {
    TX11::WindowPropertyPtr<char> title = TX11::getWindowProperty<char>("_NET_WM_NAME", d->wid, "UTF8_STRING");
    return QString::fromUtf8(title->data, static_cast<int>(title->nItems));
}

QRect X11Window::geometry() {
    XWindowAttributes attrs;
    XGetWindowAttributes(tX11Info::display(), d->wid, &attrs);

    Window childReturn;
    XTranslateCoordinates(tX11Info::display(), d->wid, tX11Info::appRootWindow(), 0, 0, &attrs.x, &attrs.y, &childReturn);

    QRect geometry(attrs.x, attrs.y, attrs.width, attrs.height);
    return geometry;
}

bool X11Window::isMinimized() {
    return d->windowState & X11WindowPrivate::Hidden;
}

bool X11Window::isMaximised() {
    return d->windowState & (X11WindowPrivate::MaximizedHorz | X11WindowPrivate::MaximizedVert);
}

bool X11Window::isFullScreen() {
    return d->windowState & (X11WindowPrivate::Fullscreen);
}

QIcon X11Window::icon() {
    if (d->iconNeedsUpdate) {
        d->windowIcon = QIcon();

        TX11::WindowPropertyPtr<long> icons = TX11::getWindowProperty<long>("_NET_WM_ICON", d->wid, XA_CARDINAL);

        long offset = 0;
        while (offset < static_cast<long>(icons->nItems)) {
            long width = icons->at(offset++);
            long height = icons->at(offset++);

            if (width <= 0 || height <= 0) {
                // Bail
                return d->windowIcon;
            }

            QImage image(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32);
            for (long y = 0; y < height; y++) {
                QRgb* scanLine = reinterpret_cast<QRgb*>(image.scanLine(y));
                if (scanLine) {
                    for (long x = 0; x < height; x++) {
                        if (offset < static_cast<long>(icons->nItems)) {
                            scanLine[x] = static_cast<QRgb>(icons->at(offset++));
                        }
                    }
                }
            }
            d->windowIcon.addPixmap(QPixmap::fromImage(image));
        }

        d->iconNeedsUpdate = false;
    }
    return d->windowIcon;
}

void X11Window::activate() {
    TX11::WindowPropertyPtr<long> userTime = TX11::getWindowProperty<long>("_NET_WM_USER_TIME", d->wid, XA_CARDINAL);

    long userTimeValue;
    if (userTime->nItems > 0) {
        userTimeValue = userTime->first();
    } else {
        userTimeValue = CurrentTime;
    }

    long activeWindow = 0;
    if (DesktopWm::activeWindow()) {
        activeWindow = static_cast<long>(static_cast<X11Window*>(DesktopWm::activeWindow().data())->d->wid);
    }
    TX11::sendMessageToRootWindow("_NET_ACTIVE_WINDOW", d->wid, 2, userTimeValue, activeWindow);
}

quint64 X11Window::pid() {
    TX11::WindowPropertyPtr<long> pid = TX11::getWindowProperty<long>("_NET_WM_PID", d->wid, XA_CARDINAL);
    if (pid->nItems == 0) return 0;

    return static_cast<quint64>(pid->first());
}

void X11Window::close() {
    TX11::sendMessageToRootWindow("_NET_CLOSE_WINDOW", d->wid, CurrentTime, 2);
}

void X11Window::kill() {
    TX11::WindowPropertyPtr<unsigned long> pid = TX11::getWindowProperty<unsigned long>("_NET_WM_PID", d->wid, XA_CARDINAL);
    XKillClient(tX11Info::display(), d->wid);
    if (pid->nItems != 0) {
        // Also kill this process by its PID for good measure
        ::kill(pid->first(), SIGKILL);
    }
}

void X11Window::updateState() {
    const QMap<QString, X11WindowPrivate::State> windowState = {
        {"_NET_WM_STATE_MODAL",             X11WindowPrivate::Modal           },
        {"_NET_WM_STATE_STICKY",            X11WindowPrivate::Sticky          },
        {"_NET_WM_STATE_MAXIMIZED_VERT",    X11WindowPrivate::MaximizedVert   },
        {"_NET_WM_STATE_MAXIMIZED_HORZ",    X11WindowPrivate::MaximizedHorz   },
        {"_NET_WM_STATE_SHADED",            X11WindowPrivate::Shaded          },
        {"_NET_WM_STATE_SKIP_TASKBAR",      X11WindowPrivate::SkipTaskbar     },
        {"_NET_WM_STATE_SKIP_PAGER",        X11WindowPrivate::SkipPager       },
        {"_NET_WM_STATE_HIDDEN",            X11WindowPrivate::Hidden          },
        {"_NET_WM_STATE_FULLSCREEN",        X11WindowPrivate::Fullscreen      },
        {"_NET_WM_STATE_ABOVE",             X11WindowPrivate::Above           },
        {"_NET_WM_STATE_BELOW",             X11WindowPrivate::Below           },
        {"_NET_WM_STATE_DEMANDS_ATTENTION", X11WindowPrivate::DemandsAttention}
    };

    X11WindowPrivate::State newState = X11WindowPrivate::NoState;
    TX11::WindowPropertyPtr<Atom> states = TX11::getWindowProperty<Atom>("_NET_WM_STATE", d->wid, XA_ATOM);
    for (Atom a : *states) {
        QString atom = TX11::atomName(a);
        if (windowState.contains(atom)) {
            newState |= windowState.value(atom);
        }
    }
    d->windowState = newState;
}

ApplicationPointer X11Window::calculateApplication() {
    // First, see if we can figure out the application using the GTK application ID
    TX11::WindowPropertyPtr<char> gtkAppId = TX11::getWindowProperty<char>("_GTK_APPLICATION_ID", d->wid, "UTF8_STRING");
    if (gtkAppId->type == XInternAtom(tX11Info::display(), "UTF8_STRING", false)) {
        ApplicationPointer app(new Application(QString::fromUtf8(gtkAppId->data, gtkAppId->nItems)));
        if (app->isValid()) return app;
    }

    // See if we can figure out the application using the class hint
    XClassHint classHint;
    if (XGetClassHint(tX11Info::display(), d->wid, &classHint) == 0) return nullptr;

    QStringList classHints = {
        classHint.res_name,
        classHint.res_class};

    // Remove empty class hints
    classHints.removeAll("");

    QStringList applications = Application::allApplications();
    for (const QString& desktopEntry : qAsConst(applications)) {
        ApplicationPointer app(new Application(desktopEntry));
        for (const QString& classHint : classHints) {
            if (classHint == app->getProperty("StartupWMClass").toString()) return app;
            if (classHint == desktopEntry) return app;
            if (classHint.toLower() == desktopEntry.toLower()) return app;
        }
    }

    return nullptr;
}

bool X11Window::shouldShowInTaskbar() {
    if (d->windowState & X11WindowPrivate::SkipTaskbar) return false;

    TX11::WindowPropertyPtr<Atom> windowType = TX11::getWindowProperty<Atom>("_NET_WM_WINDOW_TYPE", d->wid, XA_ATOM);
    if (windowType->nItems != 0) {
        QString atomName = TX11::atomName(windowType->first());
        if (QStringList({"_NET_WM_WINDOW_TYPE_DESKTOP",
                            "_NET_WM_WINDOW_TYPE_DOCK"})
                .contains(atomName)) {
            return false;
        }
    }

    TX11::WindowPropertyPtr<unsigned long> skipTaskbar = TX11::getWindowProperty<unsigned long>("_THESHELL_SKIP_TASKBAR", d->wid, XA_CARDINAL);
    if (skipTaskbar->nItems != 0 && skipTaskbar->first()) {
        return false;
    }

    return true;
}

uint X11Window::desktop() {
    TX11::WindowPropertyPtr<uint> desktop = TX11::getWindowProperty<uint>("_NET_WM_DESKTOP", d->wid, XA_CARDINAL);
    if (desktop->nItems == 0) return 0;
    return desktop->first();
}

bool X11Window::isOnDesktop(uint desktop) {
    auto thisDesktop = this->desktop();
    return thisDesktop == desktop || thisDesktop == UINT_MAX;
}

bool X11Window::isOnCurrentDesktop() {
    uint desktop = this->desktop();
    if (desktop == UINT_MAX) return true;
    return DesktopWm::currentDesktop() == desktop;
}

void X11Window::moveToDesktop(uint desktop) {
    TX11::sendMessageToRootWindow("_NET_WM_DESKTOP", d->wid, desktop, 2);
}

ApplicationPointer X11Window::application() {
    if (d->application) return d->application;

    d->application = calculateApplication();
    return d->application;
}
