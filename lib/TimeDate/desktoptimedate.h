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
#ifndef DESKTOPTIMEDATE_H
#define DESKTOPTIMEDATE_H

#include <QCoreApplication>
#include <QString>

class QLabel;
class QTimer;
struct DesktopTimeDatePrivate;
class DesktopTimeDate {
        Q_DECLARE_TR_FUNCTIONS(DesktopTimeDate)

    public:
        enum StringType {
            FullTime,
            Time,
            AmPm,
            StandardDate
        };

        static QString timeString(QDateTime d, StringType type);
        static QString timeString(StringType type);
        static void makeTimeLabel(QLabel* label, StringType type);
        static QTimer* timeUpdateTimer();

    private:
        static DesktopTimeDatePrivate* d;

        static void updateClocks();
};

#endif // DESKTOPTIMEDATE_H
