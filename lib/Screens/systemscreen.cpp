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
#include "systemscreen.h"

#include <QJsonObject>

SystemScreen::SystemScreen(QObject* parent) :
    QObject(parent) {
}

QJsonObject SystemScreen::serialise() {
    QJsonObject object;
    auto currentMode = this->availableModes().at(this->currentMode());

    QJsonObject mode;
    mode.insert("width", static_cast<int>(currentMode.width));
    mode.insert("height", static_cast<int>(currentMode.height));
    mode.insert("framerate", currentMode.framerate);
    mode.insert("interlaced", currentMode.isInterlaced);

    object.insert("mode", mode);

    return object;
}

void SystemScreen::load(QJsonObject config) {
    // Locate a viable mode
    auto mode = config.value("mode").toObject();
    auto width = mode.value("width").toInt();
    auto height = mode.value("height").toInt();
    auto framerate = mode.value("height").toDouble();
    auto interlaced = mode.value("interlaced").toBool();
    for (auto availableMode : this->availableModes()) {
        if (availableMode.width == width && availableMode.height == height && availableMode.framerate == framerate && availableMode.isInterlaced == interlaced) {
            this->setCurrentMode(availableMode.id);
            break;
        }
    }
}

QJsonObject SystemScreen::serialiseGeo() {
    QJsonObject geo;
    geo.insert("left", this->geometry().left());
    geo.insert("top", this->geometry().top());
    geo.insert("rotation", this->currentRotation());

    return geo;
}

void SystemScreen::loadGeo(QJsonObject geo) {
    this->setRotation(static_cast<Rotation>(geo.value("rotation").toInt()));

    QPoint location(geo.value("left").toInt(), geo.value("top").toInt());
    this->move(location);
}
