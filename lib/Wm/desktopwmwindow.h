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
#ifndef DESKTOPWMWINDOW_H
#define DESKTOPWMWINDOW_H

#include <QObject>
#include <QPointer>
#include "Applications/application.h"

class DesktopWmWindow : public QObject {
        Q_OBJECT
    public:
        virtual QString title() = 0;
        virtual QRect geometry() = 0;
        virtual QIcon icon() = 0;

        virtual bool isMinimized() = 0;
        virtual bool isMaximised() = 0;
        virtual bool isFullScreen() = 0;
        virtual bool shouldShowInTaskbar() = 0;
        virtual quint64 pid() = 0;
        virtual uint desktop() = 0;
        virtual bool isOnCurrentDesktop() = 0;

        bool isActive();

        virtual ApplicationPointer application() = 0;

    signals:
        void titleChanged();
        void iconChanged();
        void windowStateChanged();
        void geometryChanged();
        void applicationChanged();

    public slots:
        virtual void activate() = 0;
        virtual void close() = 0;
        virtual void kill() = 0;

    protected:
        explicit DesktopWmWindow();
};
typedef QPointer<DesktopWmWindow> DesktopWmWindowPtr;

#endif // DESKTOPWMWINDOW_H
