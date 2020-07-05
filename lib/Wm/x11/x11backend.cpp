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

#include <QApplication>
#include <QDebug>
#include <QX11Info>
#include <QWidget>
#include <QScopedPointer>
#include <QScreen>

#include "x11backend.h"
#include "x11window.h"
#include "x11functions.h"
#include "x11accessibility.h"

#include <X11/XKBlib.h>

#ifdef HAVE_XSCRNSAVER
    #include <X11/extensions/scrnsaver.h>
#endif

#ifdef HAVE_XEXT
    #include <X11/extensions/dpms.h>
#endif

struct X11BackendPrivate {
    struct X11KeyGrab {
        KeyCode keycode;
        uint keymods;

        bool operator==(const X11KeyGrab& other) const;
    };

    QMap<Window, X11WindowPtr> windows;
    QMap<QString, std::function<void()>> propertyChangeEvents;

    bool haveScrnsaver = false;
    bool haveDpms = false;
    bool heardSuper = false;

    QHash<quint64, X11KeyGrab> grabs;
    quint64 nextGrab = 1;

    X11Accessibility* accessibility;
};

X11Backend::X11Backend() : WmBackend() {
    d = new X11BackendPrivate();

    QApplication::instance()->installNativeEventFilter(this);
    XSelectInput(QX11Info::display(), QX11Info::appRootWindow(), PropertyChangeMask);

    d->accessibility = new X11Accessibility(this);

    TX11::WindowPropertyPtr<Window> windowList = TX11::getRootWindowProperty<Window>("_NET_CLIENT_LIST", AnyPropertyType, 0, ~0L);
    for (Window window : *windowList) {
        addWindow(window);
    }

    d->propertyChangeEvents.insert("_NET_CLIENT_LIST", [ = ] {
        TX11::WindowPropertyPtr<Window> newWindowList = TX11::getRootWindowProperty<Window>("_NET_CLIENT_LIST", XA_WINDOW);

        //Find out which windows no longer exist
        for (int i = 0; i < d->windows.count(); i++) {
            Window win = d->windows.keys().at(i);
            if (!newWindowList->contains(win)) {
                //This window no longer exists
                X11WindowPtr window = d->windows.value(win);
                emit windowRemoved(window.data());
                window->deleteLater();
                d->windows.remove(win);
                i--;
            }
        }

        //Find out which windows are new
        for (Window win : *newWindowList) {
            if (!d->windows.contains(win)) {
                //This window is new
                addWindow(win);
            }
        }
    });
    d->propertyChangeEvents.insert("_NET_ACTIVE_WINDOW", [ = ] {
        emit activeWindowChanged();
    });
    d->propertyChangeEvents.insert("_NET_NUMBER_OF_DESKTOPS", [ = ] {
        emit desktopCountChanged();
    });
    d->propertyChangeEvents.insert("_NET_DESKTOP_NAMES", [ = ] {
        emit desktopCountChanged();
    });
    d->propertyChangeEvents.insert("_NET_CURRENT_DESKTOP", [ = ] {
        emit currentDesktopChanged();
    });

    int eventBase, errorBase;
#ifdef HAVE_XSCRNSAVER
    if (XScreenSaverQueryExtension(QX11Info::display(), &eventBase, &errorBase)) d->haveScrnsaver = true;
#endif
#ifdef HAVE_XEXT
    if (DPMSQueryExtension(QX11Info::display(), &eventBase, &errorBase) && DPMSCapable(QX11Info::display())) d->haveDpms = true;
#endif
}

bool X11Backend::isSuitable() {
    return QX11Info::isPlatformX11();
}

DesktopAccessibility* X11Backend::accessibility() {
    return d->accessibility;
}

QList<DesktopWmWindowPtr> X11Backend::openWindows() {
    QList<DesktopWmWindowPtr> windows;
    for (X11WindowPtr ptr : d->windows) {
        windows.append(ptr.data());
    }
    return windows;
}

void X11Backend::addWindow(Window window) {
    X11WindowPtr w(new X11Window(window));
    emit windowAdded(w.data());
    connect(w, &X11Window::destroyed, this, [ = ] {
        d->windows.remove(window);
    });
    d->windows.insert(window, w);
}


bool X11Backend::nativeEventFilter(const QByteArray& eventType, void* message, long* result) {
    xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
    if (event->response_type == XCB_PROPERTY_NOTIFY) {
        xcb_property_notify_event_t* propertyNotify = reinterpret_cast<xcb_property_notify_event_t*>(event);
        QString property = TX11::atomName(propertyNotify->atom);
        if (d->windows.contains(propertyNotify->window)) {
            X11WindowPtr window = d->windows.value(propertyNotify->window);
            window->x11PropertyChanged(property);
        } else if (propertyNotify->window == QX11Info::appRootWindow()) {
            if (d->propertyChangeEvents.contains(property)) d->propertyChangeEvents.value(property)();
        }
    } else if (event->response_type == XCB_CONFIGURE_NOTIFY) {
        xcb_configure_notify_event_t* configureNotify = reinterpret_cast<xcb_configure_notify_event_t*>(event);
        if (d->windows.contains(configureNotify->event)) {
            X11WindowPtr window = d->windows.value(configureNotify->event);
            window->configureNotify();
        }
    } else if (event->response_type == XCB_KEY_PRESS) { //Key Press Event
        xcb_key_release_event_t* button = static_cast<xcb_key_release_event_t*>(message);
        ulong keyState = 0;
        if (button->state & XCB_MOD_MASK_1) keyState |= Mod1Mask;
        if (button->state & XCB_MOD_MASK_CONTROL) keyState |= ControlMask;
        if (button->state & XCB_MOD_MASK_4) {
            keyState |= Mod4Mask;
            d->heardSuper = true;
        }
        if (button->state & XCB_MOD_MASK_SHIFT) keyState |= ShiftMask;

        for (KeySym keysym : {
                XK_Control_L, XK_Control_R, XK_Alt_L, XK_Alt_R, XK_Shift_L, XK_Shift_R, XK_Super_L, XK_Super_R, XK_Meta_L, XK_Meta_R, XK_Hyper_L, XK_Hyper_R
            }) {
            if (button->detail == XKeysymToKeycode(QX11Info::display(), keysym)) return false; //Do nothing; this is a modifier key
        }

        //Go through all the keys and find the appropriate grab ID
        for (auto i = d->grabs.begin(); i != d->grabs.end(); i++) {
            if (i.value().keycode == button->detail && i.value().keymods == keyState) {
                emit grabbedKeyPressed(i.key());
            }
        }
    } else if (event->response_type == XCB_KEY_RELEASE) { //Key Release Event
        xcb_key_release_event_t* button = static_cast<xcb_key_release_event_t*>(message);
        if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_Super_L)) {
            if (d->heardSuper) {
                d->heardSuper = false;
            } else {
                quint64 grabId = d->grabs.key({
                    XKeysymToKeycode(QX11Info::display(), XK_Super_L),
                    0
                }, 0);
                if (grabId != 0) emit grabbedKeyPressed(grabId);
            }
        }
    } else {
        d->accessibility->postEvent(event);
    }
    return false;
}


DesktopWmWindowPtr X11Backend::activeWindow() {
    TX11::WindowPropertyPtr<Window> activeWindow = TX11::getRootWindowProperty<Window>("_NET_ACTIVE_WINDOW", "WINDOW");
    return d->windows.value(activeWindow->first(), nullptr).data();
}


QStringList X11Backend::desktops() {
    QStringList desktops;

    TX11::WindowPropertyPtr<quint32> desktopCountMessage = TX11::getRootWindowProperty<quint32>("_NET_NUMBER_OF_DESKTOPS", XA_CARDINAL);
    TX11::WindowPropertyPtr<char> desktopNames = TX11::getRootWindowProperty<char>("_NET_DESKTOP_NAMES", "UTF8_STRING");

    QByteArray desktopNamesBytes(desktopNames->data, static_cast<int>(desktopNames->nItems));
    QList<QByteArray> desktopNamesList = desktopNamesBytes.split('\0');
    desktopNamesList.takeLast(); //Remove the trailing null character

    if (desktopCountMessage->nItems > 0) {
        for (int i = 0; static_cast<uint>(i) < desktopCountMessage->first(); i++) {
            if (i < desktopNamesList.count()) {
                QString desktopName = desktopNamesList.at(i);
                desktops.append(desktopName);
            } else {
                desktops.append(tr("Desktop %1").arg(i));
            }
        }
    }

    return desktops;
}

uint X11Backend::currentDesktop() {
    TX11::WindowPropertyPtr<quint32> currentDesktop = TX11::getRootWindowProperty<quint32>("_NET_CURRENT_DESKTOP", XA_CARDINAL);
    if (currentDesktop->nItems > 0) {
        return currentDesktop->first();
    } else {
        return 0;
    }
}


void X11Backend::setCurrentDesktop(uint desktopNumber) {
    TX11::sendMessageToRootWindow("_NET_CURRENT_DESKTOP", QX11Info::appRootWindow(), desktopNumber, CurrentTime);
}

void X11Backend::setSystemWindow(QWidget* widget) {
    this->setSystemWindow(widget, DesktopWm::SystemWindowTypeSkipTaskbarOnly);
}

void X11Backend::setSystemWindow(QWidget* widget, DesktopWm::SystemWindowType type) {
    //Skip the taskbar
    unsigned long skipTaskbar = 1;
    XChangeProperty(QX11Info::display(), widget->winId(), XInternAtom(QX11Info::display(), "_THESHELL_SKIP_TASKBAR", False),
        XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&skipTaskbar), 1);

    //Set visible on all desktops
    unsigned long desktop = 0xFFFFFFFF;
    XChangeProperty(QX11Info::display(), widget->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
        XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&desktop), 1);

    switch (type) {
        case DesktopWm::SystemWindowTypeSkipTaskbarOnly: {
            //Change the window type to a _NET_WM_WINDOW_TYPE_NORMAL
            Atom DesktopWindowTypeAtom;
            DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_NORMAL", False);
            XChangeProperty(QX11Info::display(), widget->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&DesktopWindowTypeAtom), 1);
            break;
        }
        case DesktopWm::SystemWindowTypeDesktop: {
            //Change the window type to a _NET_WM_WINDOW_TYPE_DESKTOP
            Atom DesktopWindowTypeAtom;
            DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_DESKTOP", False);
            XChangeProperty(QX11Info::display(), widget->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&DesktopWindowTypeAtom), 1);
            break;
        }
        case DesktopWm::SystemWindowTypeTaskbar: {
            //Change the window type to a _NET_WM_WINDOW_TYPE_DOCK
            Atom DesktopWindowTypeAtom;
            DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_DOCK", False);
            XChangeProperty(QX11Info::display(), widget->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&DesktopWindowTypeAtom), 1);
            break;
        }
        case DesktopWm::SystemWindowTypeNotification: {
            //Change the window type to a _NET_WM_WINDOW_TYPE_NOTIFICATION
            //And also a _KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY specifically for KWin :)

            Atom DesktopWindowTypeAtom[2];
            DesktopWindowTypeAtom[0] = XInternAtom(QX11Info::display(), "_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY", False);
            DesktopWindowTypeAtom[1] = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
            XChangeProperty(QX11Info::display(), widget->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&DesktopWindowTypeAtom), 2);

        }
    }
}

void X11Backend::blurWindow(QWidget* widget) {
    unsigned char value = 0;
    XChangeProperty(QX11Info::display(), widget->winId(), XInternAtom(QX11Info::display(), "_KDE_NET_WM_BLUR_BEHIND_REGION", False),
        XA_CARDINAL, 32, PropModeReplace, &value, 1);
}

void X11Backend::setScreenMarginForWindow(QWidget* widget, QScreen* screen, Qt::Edge edge, int width) {
    QRect rootGeometry;
    for (QScreen* screen : qApp->screens()) {
        rootGeometry = rootGeometry.united(screen->geometry());
    }

    long struts[12];
    std::fill(struts, struts + 12, 0);
    QRect screenGeometry = screen->geometry();
    switch (edge) {
        case Qt::TopEdge:
            struts[2] = screenGeometry.y() + width; //top
            struts[8] = screenGeometry.x(); //top_start_x
            struts[9] = screenGeometry.right(); //top_end_x
            break;
        case Qt::LeftEdge:
            struts[0] = screenGeometry.x() + width; //left
            struts[4] = screenGeometry.y(); //left_start_y
            struts[5] = screenGeometry.bottom(); //left_end_y
            break;
        case Qt::RightEdge:
            struts[1] = rootGeometry.width() - screenGeometry.right() + width; //right
            struts[6] = screenGeometry.y(); //right_start_y
            struts[7] = screenGeometry.bottom(); //right_end_y
            break;
        case Qt::BottomEdge:
            struts[3] = rootGeometry.height() - screenGeometry.bottom() + width; //bottom
            struts[10] = screenGeometry.x(); //bottom_start_x
            struts[11] = screenGeometry.right(); //bottom_end_x
            break;

    }

    XChangeProperty(QX11Info::display(), widget->winId(), XInternAtom(QX11Info::display(), "_NET_WM_STRUT_PARTIAL", False),
        XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<unsigned char*>(struts), 12);
}

void X11Backend::setShowDesktop(bool showDesktop) {
    TX11::sendMessageToRootWindow("_NET_SHOWING_DESKTOP", QX11Info::appRootWindow(), showDesktop ? 1 : 0);
}


quint64 X11Backend::msecsIdle() {
#ifdef HAVE_XSCRNSAVER
    if (d->haveScrnsaver) {
        QScopedPointer<XScreenSaverInfo, TX11::XDeleter> info(XScreenSaverAllocInfo());
        if (info.isNull()) return 0;
        if (!XScreenSaverQueryInfo(QX11Info::display(), QX11Info::appRootWindow(), info.data())) return 0;

        return info->idle;
    }
#endif

    return 0;
}

quint64 X11Backend::grabKey(Qt::Key key, Qt::KeyboardModifiers modifiers) {
    quint64 grabId = d->nextGrab;
    d->nextGrab++;

    uint keymods = TX11::toNativeModifiers(modifiers);
    KeySym keysym = TX11::toKeySym(key);
    KeyCode keycode = XKeysymToKeycode(QX11Info::display(), keysym);

    if (XGrabKey(QX11Info::display(), keycode, keymods, QX11Info::appRootWindow(), true, GrabModeAsync, GrabModeAsync) == 0) {
        qDebug() << "Failed grabbing key" << key << modifiers;
    }

    d->grabs.insert(grabId, {keycode, keymods});

    return grabId;
}

void X11Backend::ungrabKey(quint64 grab) {
    X11BackendPrivate::X11KeyGrab kg = d->grabs.value(grab);
    XUngrabKey(QX11Info::display(), kg.keycode, kg.keymods, QX11Info::appRootWindow());
    d->grabs.remove(grab);
}


void X11Backend::setScreenOff(bool screenOff) {
#ifdef HAVE_XEXT
    if (d->haveDpms) {
        if (screenOff) {
            DPMSForceLevel(QX11Info::display(), DPMSModeOff);
        } else {
            DPMSForceLevel(QX11Info::display(), DPMSModeOff);
        }
    }
#endif
}

bool X11Backend::isScreenOff() {
#ifdef HAVE_XEXT
    if (d->haveDpms) {
        BOOL state;
        CARD16 powerLevel;
        DPMSInfo(QX11Info::display(), &powerLevel, &state);

        if (powerLevel == DPMSModeOn) {
            return true;
        } else {
            return false;
        }
    }
#endif
    return false;
}

bool X11BackendPrivate::X11KeyGrab::operator==(const X11BackendPrivate::X11KeyGrab& other) const {
    return other.keycode == this->keycode && other.keymods == this->keymods;
}
