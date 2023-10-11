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
#ifndef WAYLANDBACKEND_H
#define WAYLANDBACKEND_H

#include "Wm/private/wmbackend.h"
#include "qwayland-tdesktopenvironment-keygrab-v1.h"
#include "qwayland-wlr-foreign-toplevel-management-unstable-v1.h"
#include <QObject>

struct zwlr_foreign_toplevel_handle_v1;
struct WaylandBackendPrivate;
struct WaylandWindow;
struct wl_display;
struct wl_seat;
class WaylandBackend : public WmBackend,
                       public QtWayland::tdesktopenvironment_keygrab_manager_v1 {
        Q_OBJECT
    public:
        explicit WaylandBackend();

        static bool isSuitable();
        QString windowSystemName();
        wl_display* display();
        wl_seat* seat();

    signals:

    private slots:
        void viewAdded(uint viewId);
        void viewRemoved(uint viewId);

    private:
        friend WaylandWindow;
        WaylandBackendPrivate* d;

        // WmBackend interface
    public:
        DesktopAccessibility* accessibility();
        QList<DesktopWmWindowPtr> openWindows();
        DesktopWmWindowPtr activeWindow();
        QStringList desktops();
        uint currentDesktop();
        void setCurrentDesktop(uint desktopNumber);
        void setNumDesktops(uint numDesktops);
        void setShowDesktop(bool showDesktop);
        bool supportsSetNumDesktops();
        void setSystemWindow(QWidget* widget);
        void setSystemWindow(QWidget* widget, DesktopWm::SystemWindowType windowType);
        void blurWindow(QWidget* widget);
        void setScreenMarginForWindow(QWidget* widget, QScreen* screen, Qt::Edge edge, int width);
        void setScreenOff(bool screenOff);
        bool isScreenOff();
        quint64 msecsIdle();
        quint64 grabKey(Qt::Key key, Qt::KeyboardModifiers modifiers);
        void ungrabKey(quint64 grab);
        QStringList availableKeyboardLayouts();
        QString currentKeyboardLayout();
        QString keyboardLayoutDescription(QString layout);
        void setCurrentKeyboardLayout(QString layout);
        void registerAsPrimaryProvider();

        // tdesktopenvironment_keygrab_manager_v1 interface
    protected:
        void tdesktopenvironment_keygrab_manager_v1_activated(uint32_t mod, uint32_t key, uint32_t type);
};

#endif // WAYLANDBACKEND_H
