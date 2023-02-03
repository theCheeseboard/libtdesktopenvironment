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
#ifndef QUIETMODEMANAGER_H
#define QUIETMODEMANAGER_H

#include <QObject>
#include <QCoroTask>

struct QuietModeManagerPrivate;
class QuietModeManager : public QObject {
        Q_OBJECT

    public:
        explicit QuietModeManager(QObject* parent = nullptr);
        ~QuietModeManager();

        enum QuietMode {
            None = 0,
            Critical = 1,
            Notifications = 2,
            Mute = 3
        }; //Keep in sync with libtdesktopenvironment

        bool isAvailable();

        QCoro::Task<> setQuietMode(QuietMode quietMode);
        QCoro::Task<QuietMode> quietMode();

    private Q_SLOTS:
        void quietModeChangedDBus(QString newMode, QString oldMode);

    signals:
        void quietModeChanged(QuietMode newMode, QuietMode oldMode);

    private:
        QuietModeManagerPrivate* d;
};

#endif // QUIETMODEMANAGER_H
