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
#include "x11accessibility.h"

#include <QDebug>

#include <X11/extensions/XKB.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <QX11Info>

// xkb.h expects "explicit" to not be a keyword
#define explicit _explicit
#include <xcb/xkb.h>

struct X11AccessibilityPrivate {
    int xkbEventBase;

    bool stickyKeysEnabled = false;
    bool mouseKeysEnabled = false;
};

X11Accessibility::X11Accessibility(X11Backend* parent) : DesktopAccessibility(parent) {
    d = new X11AccessibilityPrivate();
    XkbQueryExtension(QX11Info::display(), nullptr, &d->xkbEventBase, nullptr, nullptr, nullptr);
    XkbSelectEvents(QX11Info::display(), XkbUseCoreKbd, XkbStateNotifyMask | XkbBellNotifyMask | XkbControlsNotifyMask, XkbStateNotifyMask | XkbBellNotifyMask | XkbControlsNotifyMask);
}

X11Accessibility::~X11Accessibility() {
    delete d;
}

void X11Accessibility::postEvent(xcb_generic_event_t* event) {
    if (event->response_type == d->xkbEventBase) {
        qDebug() << "XKB event of some sort";

        //pad0 contains the event subtype
        if (event->pad0 == XkbStateNotify) {
            xcb_xkb_state_notify_event_t* notifyEvent = reinterpret_cast<xcb_xkb_state_notify_event_t*>(event);
            qDebug() << "State Notify";

            Qt::KeyboardModifiers latched, locked;
            if (notifyEvent->latchedMods & ControlMask) latched |= Qt::ControlModifier;
            if (notifyEvent->latchedMods & Mod1Mask) latched |= Qt::AltModifier;
            if (notifyEvent->latchedMods & ShiftMask) latched |= Qt::ShiftModifier;
            if (notifyEvent->latchedMods & Mod4Mask) latched |= Qt::MetaModifier;
            if (notifyEvent->lockedMods & ControlMask) locked |= Qt::ControlModifier;
            if (notifyEvent->lockedMods & Mod1Mask) locked |= Qt::AltModifier;
            if (notifyEvent->lockedMods & ShiftMask) locked |= Qt::ShiftModifier;
            if (notifyEvent->lockedMods & Mod4Mask) locked |= Qt::MetaModifier;

            emit stickyKeysStateChanged(latched, locked);
        } else if (event->pad0 == XkbBellNotify) {
            xcb_xkb_bell_notify_event_t* notifyEvent = reinterpret_cast<xcb_xkb_bell_notify_event_t*>(event);
            qDebug() << "Bell Notify";
            if (notifyEvent->name != None) {
                char* atomName = XGetAtomName(QX11Info::display(), notifyEvent->name);
                qDebug() << atomName;
            }
        } else if (event->pad0 == XkbControlsNotify) {
            xcb_xkb_controls_notify_event_t* notifyEvent = reinterpret_cast<xcb_xkb_controls_notify_event_t*>(event);
            qDebug() << "Controls Notify";

            if (notifyEvent->enabledControlChanges & XkbStickyKeysMask) {
                d->stickyKeysEnabled = notifyEvent->enabledControls & XkbStickyKeysMask;
                emit accessibilityOptionEnabledChanged(StickyKeys, d->stickyKeysEnabled);
            }
            if (notifyEvent->enabledControlChanges & XkbMouseKeysMask) {
                d->mouseKeysEnabled = notifyEvent->enabledControls & XkbMouseKeysMask;
                emit accessibilityOptionEnabledChanged(MouseKeys, d->mouseKeysEnabled);
            }
        }
    }
}

bool X11Accessibility::isAccessibilityOptionEnabled(AccessibilityOption option) {
    switch (option) {
        case DesktopAccessibility::StickyKeys:
            return d->stickyKeysEnabled;
        case DesktopAccessibility::MouseKeys:
            return d->mouseKeysEnabled;
        case DesktopAccessibility::LastAccessibilityOption:
            break;
    }

    return false;
}

void X11Accessibility::setAccessibilityOptionEnabled(AccessibilityOption option, bool enabled) {
    XkbChangeEnabledControls(QX11Info::display(), XkbUseCoreKbd, XkbAccessXFeedbackMask, XkbAccessXFeedbackMask);

    switch (option) {
        case DesktopAccessibility::StickyKeys: {
            XkbChangeEnabledControls(QX11Info::display(), XkbUseCoreKbd, XkbStickyKeysMask, XkbStickyKeysMask * enabled);
            d->stickyKeysEnabled = enabled;
            break;
        }
        case DesktopAccessibility::MouseKeys: {
            XkbChangeEnabledControls(QX11Info::display(), XkbUseCoreKbd, XkbMouseKeysMask, XkbMouseKeysMask * enabled);
            d->mouseKeysEnabled = enabled;
            break;
        }
        case DesktopAccessibility::LastAccessibilityOption:
            break;
    }

    emit accessibilityOptionEnabledChanged(option, enabled);
}
