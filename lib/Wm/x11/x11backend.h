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
#ifndef X11BACKEND_H
#define X11BACKEND_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include "../private/wmbackend.h"

#include <X11/X.h>
#include <X11/Xdefs.h>

#undef CursorShape

struct X11BackendPrivate;
class X11Backend : public WmBackend, public QAbstractNativeEventFilter {
        Q_OBJECT
    public:
        explicit X11Backend();

        bool nativeEventFilter(const QByteArray& eventType, void* message, long* result);

        static bool isSuitable();

        QList<DesktopWmWindowPtr> openWindows();
        DesktopWmWindowPtr activeWindow();

        QStringList desktops();
        uint currentDesktop();
        void setCurrentDesktop(uint desktopNumber);
        void setShowDesktop(bool showDesktop);

        void setSystemWindow(QWidget* widget);
        void setSystemWindow(QWidget* widget, DesktopWm::SystemWindowType type);
        void blurWindow(QWidget* widget);

        void setScreenMarginForWindow(QWidget* widget, QScreen* screen, Qt::Edge edge, int width);

        void setScreenOff(bool screenOff);
        bool isScreenOff();
        quint64 msecsIdle();

        quint64 grabKey(Qt::Key key, Qt::KeyboardModifiers modifiers);
        void ungrabKey(quint64 grab);

    signals:

    public slots:

    private:
        X11BackendPrivate* d;

        void addWindow(Window window);

};

#endif // X11BACKEND_H
