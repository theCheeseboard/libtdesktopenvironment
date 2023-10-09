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

#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <tlogger.h>

struct WaylandAccessibilityPrivate {
        wl_display* display;
        WaylandAccessibility* parent;

        Qt::KeyboardModifiers latchedKeys;
        Qt::KeyboardModifiers lockedKeys;

        bool isStickyKeysEnabled = false;
};

WaylandAccessibility::WaylandAccessibility(WaylandBackend* parent) :
    DesktopAccessibility(parent) {
    d = new WaylandAccessibilityPrivate();
    d->display = reinterpret_cast<wl_display*>(qApp->platformNativeInterface()->nativeResourceForIntegration("display"));
    d->parent = this;

    wl_registry_listener listener = {
        [](void* data, wl_registry* registry, quint32 name, const char* interface, quint32 version) {
        WaylandAccessibilityPrivate* backend = static_cast<WaylandAccessibilityPrivate*>(data);
        if (strcmp(interface, tdesktopenvironment_accessibility_sticky_keys_v1_interface.name) == 0) {
            backend->parent->QtWayland::tdesktopenvironment_accessibility_sticky_keys_v1::init(registry, name, 1);
        }
        },
        [](void* data, wl_registry* registry, quint32 name) {
        Q_UNUSED(data)
        Q_UNUSED(registry)
        Q_UNUSED(name)
    }};

    wl_registry* registry = wl_display_get_registry(d->display);
    wl_registry_add_listener(registry, &listener, d);
    wl_display_roundtrip(d->display);

    if (!this->QtWayland::tdesktopenvironment_accessibility_sticky_keys_v1::isInitialized()) {
        tWarn("WaylandBackend") << "The compositor doesn't support the tdesktopenvironment_accessibility_sticky_keys_v1 protocol";
    }
    wl_registry_destroy(registry);
}

bool WaylandAccessibility::isAccessibilityOptionEnabled(AccessibilityOption option) {
    // TODO: Implement
    switch (option) {
        case DesktopAccessibility::StickyKeys:
            return d->isStickyKeysEnabled;
            break;
        case DesktopAccessibility::MouseKeys:
        case DesktopAccessibility::LastAccessibilityOption:
        default:
            return false;
    }
}

void WaylandAccessibility::setAccessibilityOptionEnabled(AccessibilityOption option, bool enabled) {
    switch (option) {
        case DesktopAccessibility::StickyKeys:
            this->tdesktopenvironment_accessibility_sticky_keys_v1_sticky_keys_enabled(enabled);
            break;
        case DesktopAccessibility::MouseKeys:
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
