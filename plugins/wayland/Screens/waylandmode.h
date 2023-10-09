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
#ifndef WAYLANDMODE_H
#define WAYLANDMODE_H

#include <QObject>
#include "qwayland-wlr-output-management-unstable-v1.h"
#include "Screens/systemscreen.h"

struct WaylandModePrivate;
class WaylandMode : public QObject, public QtWayland::zwlr_output_mode_v1 {
        Q_OBJECT
    public:
        explicit WaylandMode(::zwlr_output_mode_v1* mode, QObject* parent = nullptr);
        ~WaylandMode();

        QSize size();
        SystemScreen::Mode mode(int id);

    signals:

    private:
        WaylandModePrivate* d;

        // zwlr_output_mode_v1 interface
    protected:
        void zwlr_output_mode_v1_size(int32_t width, int32_t height);
        void zwlr_output_mode_v1_refresh(int32_t refresh);
        void zwlr_output_mode_v1_preferred();
        void zwlr_output_mode_v1_finished();
};

#endif // WAYLANDMODE_H
