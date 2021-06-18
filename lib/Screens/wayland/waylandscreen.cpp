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
#include "waylandscreen.h"

#include <QMap>
#include "waylandmode.h"

struct WaylandScreenPrivate {
    QString name;
    QString description;
    QString manufacturer;
    QString model;
    bool powered;
    QPoint position;

    wl_output_transform transform;

    QList<WaylandMode*> modes;
    int currentMode = -1;
};

WaylandScreen::WaylandScreen(::zwlr_output_head_v1* head) : SystemScreen(), QtWayland::zwlr_output_head_v1(head) {
    d = new WaylandScreenPrivate();
}

WaylandScreen::~WaylandScreen() {
    delete d;
}


void WaylandScreen::zwlr_output_head_v1_name(const QString& name) {
    d->name = name;
}

void WaylandScreen::zwlr_output_head_v1_description(const QString& description) {
    d->description = description;
}

void WaylandScreen::zwlr_output_head_v1_physical_size(int32_t width, int32_t height) {
}

void WaylandScreen::zwlr_output_head_v1_mode(zwlr_output_mode_v1* mode) {
    d->modes.append(new WaylandMode(mode, this));
}

void WaylandScreen::zwlr_output_head_v1_enabled(int32_t enabled) {
    d->powered = enabled;
    emit poweredChanged(d->powered);
}

void WaylandScreen::zwlr_output_head_v1_current_mode(zwlr_output_mode_v1* mode) {
    for (int i = 0; i < d->modes.length(); i++) {
        if (d->modes.at(i)->object() == mode) {
            d->currentMode = i;
            emit currentModeChanged(d->currentMode);
            return;
        }
    }
    d->currentMode = -1;
    emit currentModeChanged(d->currentMode);
}

void WaylandScreen::zwlr_output_head_v1_position(int32_t x, int32_t y) {
    d->position = QPoint(x, y);
}

void WaylandScreen::zwlr_output_head_v1_transform(int32_t transform) {
    d->transform = static_cast<wl_output_transform>(transform);
}

void WaylandScreen::zwlr_output_head_v1_scale(wl_fixed_t scale) {
}

void WaylandScreen::zwlr_output_head_v1_finished() {
}

void WaylandScreen::zwlr_output_head_v1_make(const QString& make) {
    d->manufacturer = make;
}

void WaylandScreen::zwlr_output_head_v1_model(const QString& model) {
    d->model = model;
}

void WaylandScreen::zwlr_output_head_v1_serial_number(const QString& serial_number) {
}

bool WaylandScreen::isScreenBrightnessAvailable() {
    return false;
}

double WaylandScreen::screenBrightness() {
    return 1;
}

void WaylandScreen::setScreenBrightness(double screenBrightness) {
}

void WaylandScreen::adjustGammaRamps(QString adjustmentName, GammaRamps ramps) {
}

void WaylandScreen::removeGammaRamps(QString adjustmentName) {
}

bool WaylandScreen::powered() const {
    return d->powered;
}

bool WaylandScreen::isPrimary() const {
    return false;
}

void WaylandScreen::setPowered(bool powered) {
}

QRect WaylandScreen::geometry() const {
    return QRect(d->position, d->modes.at(d->currentMode)->size());
}

void WaylandScreen::move(QPoint topLeft) {
}

QList<WaylandScreen::Mode> WaylandScreen::availableModes() const {
    QList<WaylandScreen::Mode> modes;
    for (int i = 0; i < d->modes.length(); i++) {
        modes.append(d->modes.at(i)->mode(i));
    }
    return modes;
}

int WaylandScreen::currentMode() const {
    return d->currentMode;
}

void WaylandScreen::setCurrentMode(int mode) {
    d->currentMode = mode;
    emit currentModeChanged(d->currentMode);
}

void WaylandScreen::setAsPrimary() {
}

WaylandScreen::Rotation WaylandScreen::currentRotation() const {
    switch (d->transform) {
        case WL_OUTPUT_TRANSFORM_NORMAL:
            return Landscape;
        case WL_OUTPUT_TRANSFORM_90:
            return Portrait;
        case WL_OUTPUT_TRANSFORM_180:
            return UpsideDown;
        case WL_OUTPUT_TRANSFORM_270:
            return UpsideDownPortrait;
        default:
            return Landscape;
    }
}

void WaylandScreen::setRotation(Rotation rotation) {
}

QString WaylandScreen::displayName() const {
    if (d->manufacturer.isEmpty() && d->model.isEmpty()) {
        return d->name;
    }
    return d->manufacturer + " " + d->model;
}

QString WaylandScreen::physicalMonitorId() const {
    return "";
}

QByteArray WaylandScreen::edid() const {
    return QByteArray();
}

QScreen* WaylandScreen::qtScreen() const {
    return nullptr;
}

void WaylandScreen::set() {
}

void WaylandScreen::reset() {
}
