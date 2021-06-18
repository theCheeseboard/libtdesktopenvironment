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
#include "waylandmode.h"

struct WaylandModePrivate {
    SystemScreen::Mode mode;
};

WaylandMode::WaylandMode(::zwlr_output_mode_v1* mode, QObject* parent) : QObject(parent), QtWayland::zwlr_output_mode_v1(mode) {
    d = new WaylandModePrivate();
    d->mode.isInterlaced = false;
}

WaylandMode::~WaylandMode() {
    delete d;
}

QSize WaylandMode::size() {
    return QSize(d->mode.width, d->mode.height);
}

SystemScreen::Mode WaylandMode::mode(int id) {
    d->mode.id = id;
    return d->mode;
}

void WaylandMode::zwlr_output_mode_v1_size(int32_t width, int32_t height) {
    d->mode.width = width;
    d->mode.height = height;
}

void WaylandMode::zwlr_output_mode_v1_refresh(int32_t refresh) {
    d->mode.framerate = static_cast<double>(refresh) / 1000;
}

void WaylandMode::zwlr_output_mode_v1_preferred() {
}

void WaylandMode::zwlr_output_mode_v1_finished() {
}
