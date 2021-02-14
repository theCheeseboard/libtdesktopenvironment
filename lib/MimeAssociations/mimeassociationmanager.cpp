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
#include "mimeassociationmanager.h"

MimeAssociationManager::MimeAssociationManager(QObject* parent) : QObject(parent) {

}

ApplicationPointer MimeAssociationManager::defaultApplicationForMimeType(QString mimeType) {
    QSettings defaults("/etc/thesuite/theDesk/associations.conf", QSettings::IniFormat);
    QSettings settings;

    defaults.beginGroup("mimetypes");
    settings.beginGroup("mimetypes");

    QString assoc;
    if (defaults.contains(mimeType)) assoc = defaults.value(mimeType).toString();
    if (settings.contains(mimeType)) assoc = settings.value(mimeType).toString();

    if (!assoc.isEmpty() && assoc != "[unset]") {
        //Ensure the application still exists
        if (Application::allApplications().contains(assoc)) {
            //Use this application
            return ApplicationPointer(new Application(assoc));
        } else {
            //Clear the file association
            settings.setValue(mimeType, "[unset]");
        }
    }

    //No default application
    return nullptr;
}

void MimeAssociationManager::setDefaultApplicationForMimeType(QString application, QString mimeType) {
    QSettings settings;
    settings.beginGroup("mimetypes");
    settings.setValue(mimeType, application);
}

void MimeAssociationManager::clearDefaultApplicationForMimeType(QString mimeType) {
    QSettings settings;
    settings.beginGroup("mimetypes");
    settings.setValue(mimeType, "[unset]");
}

QList<ApplicationPointer> MimeAssociationManager::applicationsForMimeType(QString mimeType) {
    if (mimeType == "application/octet-stream") return {};
    QList<ApplicationPointer> apps;
    for (const QString& application : Application::allApplications()) {
        ApplicationPointer app(new Application(application));
        QStringList acceptableMimeTypes = app->getStringList("MimeType");
        if (acceptableMimeTypes.contains(mimeType)) apps.append(app);
    }

    //Sort the applications alphabetically
    std::sort(apps.begin(), apps.end(), [ = ](const ApplicationPointer & first, const ApplicationPointer & second) {
        return first->getProperty("Name").toString().localeAwareCompare(second->getProperty("Name").toString()) < 0;
    });

    return apps;
}
