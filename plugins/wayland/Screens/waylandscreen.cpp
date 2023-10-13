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

#include "Screens/screendaemon.h"
#include "waylandmode.h"
#include "waylandscreenbackend.h"
#include <QApplication>
#include <QMap>

struct WaylandScreenPrivate {
        WaylandScreenBackend* backend;

        QString name;
        QString description;
        QString manufacturer;
        QString model;
        bool powered;
        QPoint position;

        wl_output_transform transform;

        QList<WaylandMode*> modes;
        int currentMode = -1;

        bool initialPowered;
        int initialMode;
        QPoint initialPoisition;
};

WaylandScreen::WaylandScreen(::zwlr_output_head_v1* head, WaylandScreenBackend* backend) :
    SystemScreen(), QtWayland::zwlr_output_head_v1(head) {
    d = new WaylandScreenPrivate();
    d->backend = backend;
}

WaylandScreen::~WaylandScreen() {
    delete d;
}

void WaylandScreen::normaliseScreens() {
    QRect bounds(0, 0, 0, 0);

    // Find out how far we should offset all of the screens
    for (SystemScreen* screen : ScreenDaemon::instance()->screens()) {
        auto* scr = static_cast<WaylandScreen*>(screen);
        if (!scr->powered()) continue;

        bounds = bounds.united(scr->geometry());
    }

    // Offset all of the screens
    for (SystemScreen* screen : ScreenDaemon::instance()->screens()) {
        auto* scr = static_cast<WaylandScreen*>(screen);
        if (!scr->powered()) continue;

        scr->d->position -= bounds.topLeft();
        emit scr->geometryChanged(scr->geometry());
    }
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
    d->initialPowered = enabled;
    emit poweredChanged(d->powered);
}

void WaylandScreen::zwlr_output_head_v1_current_mode(zwlr_output_mode_v1* mode) {
    for (int i = 0; i < d->modes.length(); i++) {
        if (d->modes.at(i)->object() == mode) {
            d->currentMode = i;
            d->initialMode = i;
            emit currentModeChanged(d->currentMode);
            return;
        }
    }
    d->currentMode = -1;
    d->initialMode = -1;
    emit currentModeChanged(d->currentMode);
    emit geometryChanged(geometry());
}

void WaylandScreen::zwlr_output_head_v1_position(int32_t x, int32_t y) {
    d->position = QPoint(x, y);
    d->initialPoisition = d->position;
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
    d->powered = powered;
    emit poweredChanged(powered);
}

QRect WaylandScreen::geometry() const {
    return QRect(d->position, d->modes.at(d->currentMode)->size());
}

void WaylandScreen::move(QPoint topLeft) {
    d->position = topLeft;
    emit geometryChanged(geometry());
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
    switch (rotation) {
        case Landscape:
            d->transform = WL_OUTPUT_TRANSFORM_NORMAL;
            break;
        case Portrait:
            d->transform = WL_OUTPUT_TRANSFORM_90;
            break;
        case UpsideDown:
            d->transform = WL_OUTPUT_TRANSFORM_180;
            break;
        case UpsideDownPortrait:
            d->transform = WL_OUTPUT_TRANSFORM_270;
            break;
        default:
            d->transform = WL_OUTPUT_TRANSFORM_NORMAL;
    }
    emit rotationChanged(currentRotation());
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
    return qApp->primaryScreen();
}

void WaylandScreen::set() {
    auto config = zwlr_output_manager_v1_create_configuration(d->backend->QtWayland::zwlr_output_manager_v1::object(), d->backend->serial());

    for (auto systemScreen : d->backend->screens()) {
        auto screen = qobject_cast<WaylandScreen*>(systemScreen);
        auto screenProperties = screen->d;

        if (screen->d->powered) {
            auto configHead = zwlr_output_configuration_v1_enable_head(config, screen->QtWayland::zwlr_output_head_v1::object());
            zwlr_output_configuration_head_v1_set_mode(configHead, screenProperties->modes.at(screenProperties->currentMode)->object());
            zwlr_output_configuration_head_v1_set_position(configHead, screenProperties->position.x(), screenProperties->position.y());
            zwlr_output_configuration_head_v1_set_transform(configHead, screenProperties->transform);
        } else {
            zwlr_output_configuration_v1_disable_head(config, screen->QtWayland::zwlr_output_head_v1::object());
        }
    }

    zwlr_output_configuration_v1_apply(config);

    for (auto systemScreen : d->backend->screens()) {
        auto screen = qobject_cast<WaylandScreen*>(systemScreen);
        emit screen->currentModeChanged(screen->d->currentMode);
    }
}

void WaylandScreen::reset() {
    move(d->initialPoisition);
    setPowered(d->initialPowered);
    setCurrentMode(d->initialMode);
}
