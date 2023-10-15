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
#ifndef WAYLANDACCESSIBILITY_H
#define WAYLANDACCESSIBILITY_H

#include "Wm/desktopaccessibility.h"
#include "qwayland-tdesktopenvironment-accessibility-v1.h"
#include "waylandbackend.h"

#include <QObject>

struct WaylandAccessibilityPrivate;
class WaylandAccessibility : public DesktopAccessibility,
                             public QtWayland::tdesktopenvironment_accessibility_sticky_keys_v1,
                             public QtWayland::tdesktopenvironment_accessibility_mouse_keys_v1 {
        Q_OBJECT
    public:
        explicit WaylandAccessibility(WaylandBackend* parent);

    signals:

    private:
        WaylandAccessibilityPrivate* d;

        // DesktopAccessibility interface
    public:
        bool isAccessibilityOptionEnabled(AccessibilityOption option);
        void setAccessibilityOptionEnabled(AccessibilityOption option, bool enabled);

        // tdesktopenvironment_accessibility_sticky_keys_v1 interface
    protected:
        void tdesktopenvironment_accessibility_sticky_keys_v1_sticky_keys_enabled(uint32_t enabled);
        void tdesktopenvironment_accessibility_sticky_keys_v1_sticky_keys_held(uint32_t keys);
        void tdesktopenvironment_accessibility_sticky_keys_v1_sticky_keys_latched(uint32_t keys);

        // tdesktopenvironment_accessibility_mouse_keys_v1 interface
    protected:
        void tdesktopenvironment_accessibility_mouse_keys_v1_mouse_keys_enabled(uint32_t enabled);
};

#endif // WAYLANDACCESSIBILITY_H
