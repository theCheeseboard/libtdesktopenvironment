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
#ifndef DESKTOPPOWERPROFILES_H
#define DESKTOPPOWERPROFILES_H

#include <QMap>
#include <QObject>
#include <QVariant>

struct DesktopPowerProfilesPrivate;
class DesktopPowerProfiles : public QObject {
        Q_OBJECT
    public:
        explicit DesktopPowerProfiles(QObject* parent = nullptr);
        ~DesktopPowerProfiles();

        enum PowerProfile {
            PowerStretch,
            Balanced,
            Performance,
            Unknown
        };

        PowerProfile currentPowerProfile();
        void setCurrentPowerProfile(PowerProfile profile);

        bool powerProfilesAvailable();

        bool isPerformanceAvailable();

        QString performanceDegraded();
        QString performanceInhibited();

    private slots:
        void dbusPropertyChanged(QString interfaceName, QMap<QString, QVariant> changedProperties, QStringList invalidatedProperties);

    signals:
        void powerProfileChanged();

    private:
        DesktopPowerProfilesPrivate* d;
};

#endif // DESKTOPPOWERPROFILES_H
