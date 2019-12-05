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
#ifndef DESKTOPUPOWERDEVICE_H
#define DESKTOPUPOWERDEVICE_H

#include <QObject>
#include <QDBusObjectPath>

class DesktopUPower;
struct DesktopUPowerDevicePrivate;
class DesktopUPowerDevice : public QObject
{
        Q_OBJECT
    public:
        enum DeviceType : uint {
            UnknownType = 0,
            LinePower = 1,
            Battery = 2,
            Ups = 3,
            Monitor = 4,
            Mouse = 5,
            Keyboard = 6,
            Pda = 7,
            Phone = 8
        };

        enum DeviceState : uint {
            UnknownState = 0,
            Charging = 1,
            Discharging = 2,
            Empty = 3,
            FullyCharged = 4,
            PendingCharge = 5,
            PendingDischarge = 6
        };

        ~DesktopUPowerDevice();

        DeviceType type();
        QString typeString();

        DeviceState state();
        QString stateString();

        int percentage();
        bool online();
        QString iconName();

        qint64 timeToEmpty();
        qint64 timeToFull();

    signals:
        void propertiesUpdated();
        void lowBatteryNotification(QString description);
        void chargingNotification();
        void dischargingNotification();
        void fullNotification();

    protected:
        friend DesktopUPower;
        explicit DesktopUPowerDevice(QDBusObjectPath path, QObject *parent = nullptr);

    public slots:

    private:
        DesktopUPowerDevicePrivate* d;
};

#endif // DESKTOPUPOWERDEVICE_H
