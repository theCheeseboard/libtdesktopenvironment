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
#include "desktopupowerdevicesmodel.h"

#include "desktopupower.h"
#include "desktopupowerdevice.h"

struct DesktopUPowerDevicesModelPrivate {
    DesktopUPower* daemon;
};

DesktopUPowerDevicesModel::DesktopUPowerDevicesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    d = new DesktopUPowerDevicesModelPrivate();
    d->daemon = new DesktopUPower();
}

DesktopUPowerDevicesModel::~DesktopUPowerDevicesModel()
{
    delete d;
}

int DesktopUPowerDevicesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return d->daemon->devices().count();
}

QVariant DesktopUPowerDevicesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    DesktopUPowerDevice* device = d->daemon->devices().at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return device->typeString();
        case Qt::UserRole:
            return QVariant::fromValue(device);
    }

    return QVariant();
}
