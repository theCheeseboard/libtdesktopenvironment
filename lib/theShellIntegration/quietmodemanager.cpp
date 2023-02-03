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
#include "quietmodemanager.h"

#include <QCoroDBusPendingCall>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <texception.h>

struct QuietModeManagerPrivate {
        QDBusInterface* interface;

        static QMap<QuietModeManager::QuietMode, QString> quietModeEnumToString;
};

QMap<QuietModeManager::QuietMode, QString> QuietModeManagerPrivate::quietModeEnumToString = {
    {QuietModeManager::None,          "None"           },
    {QuietModeManager::Critical,      "CriticalOnly"   },
    {QuietModeManager::Notifications, "NoNotifications"},
    {QuietModeManager::Mute,          "Mute"           }
};

QuietModeManager::QuietModeManager(QObject* parent) :
    QObject(parent) {
    d = new QuietModeManagerPrivate();
    d->interface = new QDBusInterface("com.vicr123.theshell", "/com/vicr123/theshell/QuietMode", "com.vicr123.theshell.QuietMode");
    QDBusConnection::sessionBus().connect(d->interface->service(), d->interface->path(), d->interface->interface(), "quietModeChanged", this, SLOT(quietModeChangedDBus(QString, QString)));
}

QuietModeManager::~QuietModeManager() {
    d->interface->deleteLater();
    delete d;
}

QCoro::Task<> QuietModeManager::setQuietMode(QuietModeManager::QuietMode quietMode) {
    co_await d->interface->asyncCall("setQuietMode", d->quietModeEnumToString.value(quietMode));
}

QCoro::Task<QuietModeManager::QuietMode> QuietModeManager::quietMode() {
    auto reply = co_await d->interface->asyncCall("quietMode");
    if (reply.type() == QDBusMessage::ErrorMessage) {
        throw tDBusException(reply.errorName());
    }
    auto quietMode = reply.arguments().first().toString();
    co_return d->quietModeEnumToString.key(quietMode);
}

void QuietModeManager::quietModeChangedDBus(QString newMode, QString oldMode) {
    emit quietModeChanged(d->quietModeEnumToString.key(newMode), d->quietModeEnumToString.key(oldMode));
}
