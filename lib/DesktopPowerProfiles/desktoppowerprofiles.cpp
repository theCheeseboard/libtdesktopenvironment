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
#include "desktoppowerprofiles.h"

#include <QDBusInterface>
#include <QDBusArgument>

struct DesktopPowerProfilesPrivate {
    QDBusInterface* powerProfilesDaemon;
};

DesktopPowerProfiles::DesktopPowerProfiles(QObject* parent) : QObject(parent) {
    d = new DesktopPowerProfilesPrivate();
    d->powerProfilesDaemon = new QDBusInterface("net.hadess.PowerProfiles", "/net/hadess/PowerProfiles", "net.hadess.PowerProfiles", QDBusConnection::systemBus());

    QDBusConnection::systemBus().connect("net.hadess.PowerProfiles", "/net/hadess/PowerProfiles", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(dbusPropertyChanged(QString, QMap<QString, QVariant>, QStringList)));
}

DesktopPowerProfiles::~DesktopPowerProfiles() {
    d->powerProfilesDaemon->deleteLater();
    delete d;
}

DesktopPowerProfiles::PowerProfile DesktopPowerProfiles::currentPowerProfile() {
    QString currentProfile = d->powerProfilesDaemon->property("ActiveProfile").toString();
    if (currentProfile == "power-saver") return PowerStretch;
    if (currentProfile == "balanced") return Balanced;
    if (currentProfile == "performance") return Performance;
    return Unknown;
}

void DesktopPowerProfiles::setCurrentPowerProfile(PowerProfile profile) {
    QString profileString;
    switch (profile) {
        case DesktopPowerProfiles::Unknown:
            break;
        case DesktopPowerProfiles::PowerStretch:
            profileString = "power-saver";
            break;
        case DesktopPowerProfiles::Balanced:
            profileString = "balanced";
            break;
        case DesktopPowerProfiles::Performance:
            profileString = "performance";
            break;
    }

    if (profileString.isEmpty()) return;
    d->powerProfilesDaemon->setProperty("ActiveProfile", profileString);
}

bool DesktopPowerProfiles::isPerformanceAvailable() {
    QDBusMessage message = QDBusMessage::createMethodCall(d->powerProfilesDaemon->service(), d->powerProfilesDaemon->path(), "org.freedesktop.DBus.Properties", "Get");
    message.setArguments({
        d->powerProfilesDaemon->interface(),
            "Profiles"
        });
    QDBusMessage reply = d->powerProfilesDaemon->connection().call(message);

    QDBusArgument propertyValue = reply.arguments().first().value<QDBusVariant>().variant().value<QDBusArgument>();
    QList<QVariantMap> profiles;
    propertyValue >> profiles;

    for (const QVariantMap& profile : qAsConst(profiles)) {
        if (profile.value("Profile") == "performance") return true;
    }
    return false;
}

QString DesktopPowerProfiles::performanceDegraded() {
    return d->powerProfilesDaemon->property("PerformanceDegraded").toString();
}

QString DesktopPowerProfiles::performanceInhibited() {
    return d->powerProfilesDaemon->property("PerformanceInhibited").toString();
}

void DesktopPowerProfiles::dbusPropertyChanged(QString interfaceName, QMap<QString, QVariant> changedProperties, QStringList invalidatedProperties) {
    if (interfaceName == "net.hadess.PowerProfiles") {
        emit powerProfileChanged();
    }
}
