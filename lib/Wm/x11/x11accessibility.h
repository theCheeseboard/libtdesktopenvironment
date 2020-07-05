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
#ifndef X11ACCESSIBILITY_H
#define X11ACCESSIBILITY_H

#include "../desktopaccessibility.h"
#include "x11backend.h"

#include <xcb/xcb.h>

struct X11AccessibilityPrivate;
class X11Accessibility : public DesktopAccessibility {
        Q_OBJECT
    public:
        explicit X11Accessibility(X11Backend* parent);
        ~X11Accessibility();

        void postEvent(xcb_generic_event_t* event);

    signals:

    private:
        X11AccessibilityPrivate* d;

        // DesktopAccessibility interface
    public:
        bool isAccessibilityOptionEnabled(AccessibilityOption option);
        void setAccessibilityOptionEnabled(AccessibilityOption option, bool enabled);
};

#endif // X11ACCESSIBILITY_H
