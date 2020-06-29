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
#ifndef DESKTOPWM_H
#define DESKTOPWM_H

#include <QObject>
#include "desktopwmwindow.h"

class QScreen;
struct DesktopWmPrivate;
class DesktopWm : public QObject {
        Q_OBJECT
    public:
        enum SystemWindowType {
            SystemWindowTypeSkipTaskbarOnly,
            SystemWindowTypeDesktop,
            SystemWindowTypeTaskbar,
            SystemWindowTypeNotification
        };

        static DesktopWm* instance();

        static QList<DesktopWmWindowPtr> openWindows();
        static DesktopWmWindowPtr activeWindow();

        static QStringList desktops();
        static uint currentDesktop();
        static void setCurrentDesktop(uint desktopNumber);

        static void setShowDesktop(bool showDesktop);

        static void setSystemWindow(QWidget* widget);
        static void setSystemWindow(QWidget* widget, SystemWindowType windowType);
        static void blurWindow(QWidget* widget);

        static void setScreenMarginForWindow(QWidget* widget, QScreen* screen, Qt::Edge edge, int width);

        static void setScreenOff(bool screenOff);
        static bool isScreenOff();
        static quint64 msecsIdle();

        static quint64 grabKey(Qt::Key key, Qt::KeyboardModifiers modifiers);
        static void ungrabKey(quint64 grab);

        static QString displayName(int uid);
        static QString userDisplayName();

    signals:
        void windowAdded(DesktopWmWindowPtr window);
        void windowRemoved(DesktopWmWindowPtr window);
        void desktopCountChanged();
        void currentDesktopChanged();
        void activeWindowChanged();
        void grabbedKeyPressed(quint64 grab);

    public slots:

    private:
        explicit DesktopWm();
        static DesktopWmPrivate* d;
};

#endif // DESKTOPWM_H
