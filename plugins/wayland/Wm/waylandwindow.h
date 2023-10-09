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
#ifndef WAYLANDWINDOW_H
#define WAYLANDWINDOW_H

#include "Wm/desktopwmwindow.h"
#include <QCoroTask>
#include <QPointer>

struct zwlr_foreign_toplevel_handle_v1;
struct WaylandWindowPrivate;
struct WaylandWindowEventListener;
class WaylandBackend;
class WaylandWindow : public DesktopWmWindow {
        Q_OBJECT
    public:
        explicit WaylandWindow(uint viewId, WaylandBackend* backend);
        ~WaylandWindow();

        bool isActive();

    signals:

    private slots:
        void viewTitleChanged(uint viewId, QString title);
        void viewAppIdChanged(uint viewId, QString appId);
        void viewFocusChanged(uint viewId, bool haveFocus);
        void viewMaximizedChanged(uint viewId, bool maximized);
        void viewMinimizedChanged(uint viewId, bool minimized);

    private:
        friend WaylandWindowEventListener;
        WaylandWindowPrivate* d;

        QCoro::Task<> updateWindow();

        // DesktopWmWindow interface
    public:
        QString title();
        QRect geometry();
        QIcon icon();
        bool isMinimized();
        bool isMaximised();
        bool isFullScreen();
        bool shouldShowInTaskbar();
        quint64 pid();
        uint desktop();
        bool isOnCurrentDesktop();
        void moveToDesktop(uint desktop);
        ApplicationPointer application();
        bool isOnDesktop(uint desktop);

    public slots:
        void activate();
        void close();
        void kill();
};
typedef QPointer<WaylandWindow> WaylandWindowPtr;

#endif // WAYLANDWINDOW_H
