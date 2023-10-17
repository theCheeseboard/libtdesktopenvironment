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
#include "screendaemon.h"

#include "TdePlugin/tdepluginmanager.h"
#include "private/screenbackend.h"
#include "systemscreen.h"
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <ranges/trange.h>

struct ScreenDaemonPrivate {
        ScreenDaemon* instance = nullptr;
        ScreenBackend* backend = nullptr;

        QJsonObject screenConfigs;
        QJsonObject geoConfigs;

        bool loading = false;
        bool autorestore = false;
};

ScreenDaemonPrivate* ScreenDaemon::d = new ScreenDaemonPrivate();

ScreenDaemon* ScreenDaemon::instance() {
    if (!d->instance) d->instance = new ScreenDaemon();
    return d->instance;
}

QList<SystemScreen*> ScreenDaemon::screens() {
    return d->backend->screens();
}

SystemScreen* ScreenDaemon::primayScreen() {
    return d->backend->primaryScreen();
}

int ScreenDaemon::dpi() const {
    return d->backend->dpi();
}

void ScreenDaemon::setDpi(int dpi) {
    d->backend->setDpi(dpi);
    emit dpiChanged();
}

bool ScreenDaemon::supportsPerScreenDpi() {
    return d->backend->supportsPerScreenDpi();
}

void ScreenDaemon::saveCurrentConfiguration() {
    if (d->loading) return;

    QJsonObject thisConfigGeo;
    for (auto screen : this->screens()) {
        d->screenConfigs.insert(screen->restoreKey(), screen->serialise());
        thisConfigGeo.insert(screen->restoreKey(), screen->serialiseGeo());
    }
    d->geoConfigs.insert(this->geoKey(), thisConfigGeo);

    QJsonObject rootObject;
    rootObject.insert("screens", d->screenConfigs);
    rootObject.insert("geos", d->geoConfigs);

    QDir appdata(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QDir::root().mkpath(appdata.absolutePath());
    QFile configFile(appdata.absoluteFilePath("screens"));
    configFile.open(QFile::WriteOnly);
    configFile.write(QJsonDocument(rootObject).toJson());
    configFile.close();
}

void ScreenDaemon::tryRestoreConfiguration() {
    d->loading = true;

    // Start by restoring screen resolution, etc.
    for (auto screen : this->screens()) {
        if (!d->screenConfigs.contains(screen->restoreKey())) continue;
        screen->load(d->screenConfigs.value(screen->restoreKey()).toObject());
    }

    // Restore geospatial settings
    auto geoKey = this->geoKey();
    if (d->geoConfigs.contains(geoKey)) {
        auto geoConfig = d->geoConfigs.value(geoKey).toObject();
        for (auto screen : this->screens()) {
            screen->loadGeo(geoConfig.value(screen->restoreKey()).toObject());
        }
    }

    d->loading = false;
}

void ScreenDaemon::enableAutomaticRestore() {
    d->autorestore = true;
    this->tryRestoreConfiguration();
}

ScreenDaemon::ScreenDaemon() :
    QObject(nullptr) {
    QDir appdata(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QDir::root().mkpath(appdata.absolutePath());
    QFile configFile(appdata.absoluteFilePath("screens"));
    configFile.open(QFile::ReadOnly);
    auto rootObject = QJsonDocument::fromJson(configFile.readAll()).object();
    d->screenConfigs = rootObject.value("screens").toObject();
    d->geoConfigs = rootObject.value("geos").toObject();
    configFile.close();

    d->backend = TdePluginManager::loadedInterface()->screenBackend();

    if (d->backend) {
        connect(d->backend, &ScreenBackend::screensUpdated, this, [this] {
            emit this->screensUpdated();
            if (d->autorestore) {
                this->tryRestoreConfiguration();
            }
        });
    } else {
        // No suitable backend is available
        qWarning() << "No suitable backend for ScreenDaemon";
    }
}

QString ScreenDaemon::geoKey() {
    auto screenNames = tRange(this->screens()).map<QString>([](SystemScreen* screen) {
                                                  return screen->restoreKey();
                                              })
                           .toList();
    std::sort(screenNames.begin(), screenNames.end());
    return screenNames.join(";");
}
