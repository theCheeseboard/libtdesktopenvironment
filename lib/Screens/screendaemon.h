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
#ifndef SCREENDAEMON_H
#define SCREENDAEMON_H

#include <QObject>

struct ScreenDaemonPrivate;
class SystemScreen;
class ScreenDaemon : public QObject {
        Q_OBJECT
    public:
        static ScreenDaemon* instance();

        QList<SystemScreen*> screens();
        SystemScreen* primayScreen();

        int dpi() const;
        void setDpi(int dpi);
        bool supportsPerScreenDpi();
        void saveCurrentConfiguration();
        void tryRestoreConfiguration();
        void enableAutomaticRestore();

    signals:
        void screensUpdated();
        void dpiChanged();

    private:
        explicit ScreenDaemon();
        static ScreenDaemonPrivate* d;

        QString geoKey();
};

#endif // SCREENDAEMON_H
