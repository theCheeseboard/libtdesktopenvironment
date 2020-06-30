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
#ifndef X11WINDOW_H
#define X11WINDOW_H

#include <QPointer>
#include <QObject>
#include "../desktopwmwindow.h"

#include <X11/X.h>

struct X11WindowPrivate;
class X11Window : public DesktopWmWindow {
        Q_OBJECT
    public:
        explicit X11Window(Window wid);
        ~X11Window();

        void x11PropertyChanged(QString property);
        void configureNotify();

        QString title();
        QRect geometry();
        bool isMinimized();
        bool isMaximised();
        QIcon icon();
        quint64 pid();
        bool shouldShowInTaskbar();
        uint desktop();
        bool isOnCurrentDesktop();

        ApplicationPointer application();

    public slots:
        void activate();
        void close();

    private:
        X11WindowPrivate* d;

        void updateState();
        ApplicationPointer calculateApplication();
};
typedef QPointer<X11Window> X11WindowPtr;

#endif // X11WINDOW_H
