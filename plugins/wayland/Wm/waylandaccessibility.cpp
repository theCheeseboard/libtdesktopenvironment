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
#include "waylandaccessibility.h"

#include "twaylandregistry.h"
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <tlogger.h>

struct WaylandAccessibilityPrivate {
        tWaylandRegistry registry;
        Qt::KeyboardModifiers latchedKeys;
        Qt::KeyboardModifiers lockedKeys;

        bool isStickyKeysEnabled = false;
        bool isMouseKeysEnabled = false;
};

WaylandAccessibility::WaylandAccessibility(WaylandBackend* parent) :
    DesktopAccessibility(parent) {
    d = new WaylandAccessibilityPrivate();

    if (!d->registry.init<QtWayland::tdesktopenvironment_accessibility_sticky_keys_v1>(this)) {
        tWarn("WaylandBackend") << "The compositor doesn't support the tdesktopenvironment_accessibility_sticky_keys_v1 protocol";
    }
    if (!d->registry.init<QtWayland::tdesktopenvironment_accessibility_mouse_keys_v1>(this)) {
        tWarn("WaylandBackend") << "The compositor doesn't support the tdesktopenvironment_accessibility_mouse_keys_v1 protocol";
    }
}

bool WaylandAccessibility::isAccessibilityOptionEnabled(AccessibilityOption option) {
    switch (option) {
        case DesktopAccessibility::StickyKeys:
            return d->isStickyKeysEnabled;
        case DesktopAccessibility::MouseKeys:
            return d->isMouseKeysEnabled;
        case DesktopAccessibility::LastAccessibilityOption:
        default:
            return false;
    }
}

void WaylandAccessibility::setAccessibilityOptionEnabled(AccessibilityOption option, bool enabled) {
    switch (option) {
        case DesktopAccessibility::StickyKeys:
            if (this->QtWayland::tdesktopenvironment_accessibility_sticky_keys_v1::isInitialized()) {
                this->QtWayland::tdesktopenvironment_accessibility_sticky_keys_v1::set_enabled(enabled);
            }
            break;
        case DesktopAccessibility::MouseKeys:
            if (this->QtWayland::tdesktopenvironment_accessibility_mouse_keys_v1::isInitialized()) {
                this->QtWayland::tdesktopenvironment_accessibility_mouse_keys_v1::set_enabled(enabled);
            }
            break;
        case DesktopAccessibility::LastAccessibilityOption:
        default:
            return;
    }
}

void WaylandAccessibility::tdesktopenvironment_accessibility_sticky_keys_v1_sticky_keys_enabled(uint32_t enabled) {
    d->isStickyKeysEnabled = enabled;
    emit accessibilityOptionEnabledChanged(StickyKeys, enabled);
}

void WaylandAccessibility::tdesktopenvironment_accessibility_sticky_keys_v1_sticky_keys_held(uint32_t keys) {
    d->latchedKeys = d->latchedKeys.setFlag(Qt::ControlModifier, keys & TDESKTOPENVIRONMENT_ACCESSIBILITY_STICKY_KEYS_V1_MODIFIER_CONTROL)
                         .setFlag(Qt::AltModifier, keys & TDESKTOPENVIRONMENT_ACCESSIBILITY_STICKY_KEYS_V1_MODIFIER_ALT)
                         .setFlag(Qt::ShiftModifier, keys & TDESKTOPENVIRONMENT_ACCESSIBILITY_STICKY_KEYS_V1_MODIFIER_SHIFT)
                         .setFlag(Qt::MetaModifier, keys & TDESKTOPENVIRONMENT_ACCESSIBILITY_STICKY_KEYS_V1_MODIFIER_SUPER);
    emit stickyKeysStateChanged(d->latchedKeys, d->lockedKeys);
}

void WaylandAccessibility::tdesktopenvironment_accessibility_sticky_keys_v1_sticky_keys_latched(uint32_t keys) {
    d->lockedKeys = d->lockedKeys.setFlag(Qt::ControlModifier, keys & TDESKTOPENVIRONMENT_ACCESSIBILITY_STICKY_KEYS_V1_MODIFIER_CONTROL)
                        .setFlag(Qt::AltModifier, keys & TDESKTOPENVIRONMENT_ACCESSIBILITY_STICKY_KEYS_V1_MODIFIER_ALT)
                        .setFlag(Qt::ShiftModifier, keys & TDESKTOPENVIRONMENT_ACCESSIBILITY_STICKY_KEYS_V1_MODIFIER_SHIFT)
                        .setFlag(Qt::MetaModifier, keys & TDESKTOPENVIRONMENT_ACCESSIBILITY_STICKY_KEYS_V1_MODIFIER_SUPER);
    emit stickyKeysStateChanged(d->latchedKeys, d->lockedKeys);
}

void WaylandAccessibility::tdesktopenvironment_accessibility_mouse_keys_v1_mouse_keys_enabled(uint32_t enabled) {
    d->isMouseKeysEnabled = enabled;
    emit accessibilityOptionEnabledChanged(MouseKeys, enabled);
}
