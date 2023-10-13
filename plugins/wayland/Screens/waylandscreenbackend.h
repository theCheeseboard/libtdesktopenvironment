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
#ifndef WAYLANDSCREENBACKEND_H
#define WAYLANDSCREENBACKEND_H

#include "Screens/private/screenbackend.h"
#include "qwayland-wlr-gamma-control-unstable-v1.h"
#include "qwayland-wlr-output-management-unstable-v1.h"
#include <QObject>

struct WaylandScreenBackendPrivate;
class WaylandScreenBackend : public ScreenBackend,
                             public QtWayland::zwlr_output_manager_v1,
                             public QtWayland::zwlr_gamma_control_manager_v1 {
        Q_OBJECT
    public:
        explicit WaylandScreenBackend();
        ~WaylandScreenBackend();

        static bool isSuitable();

        uint32_t serial();

    signals:

    private:
        WaylandScreenBackendPrivate* d;

        // ScreenBackend interface
    public:
        QList<SystemScreen*> screens();
        SystemScreen* primaryScreen();
        int dpi() const;
        void setDpi(int dpi);

        // zwlr_output_manager_v1 interface
    protected:
        void zwlr_output_manager_v1_head(zwlr_output_head_v1* head);
        void zwlr_output_manager_v1_done(uint32_t serial);
        void zwlr_output_manager_v1_finished();
};

#endif // WAYLANDSCREENBACKEND_H
