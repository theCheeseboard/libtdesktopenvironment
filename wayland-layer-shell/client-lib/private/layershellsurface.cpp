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
#include "layershellsurface.h"

#include "layershellshell.h"
#include <private/qwaylandscreen_p.h>
#include <private/qwaylandsurface_p.h>
#include <private/qwaylandwindow_p.h>

struct LayerShellSurfacePrivate {
        LayerShellShell* shell;
        QPointer<QtWaylandClient::QWaylandWindow> window;

        QSize queuedSize;
        bool configured = false;
        bool fullAnchor = false;
};

LayerShellSurface::LayerShellSurface(LayerShellShell* shell, QtWaylandClient::QWaylandWindow* window) :
    QtWaylandClient::QWaylandShellSurface(window), QtWayland::zwlr_layer_surface_v1(shell->get_layer_surface(window->waylandSurface()->object(), window->waylandScreen()->output(), QtWayland::zwlr_layer_shell_v1::layer_top, "window")) {
    d = new LayerShellSurfacePrivate();
    set_anchor(anchor_top | anchor_bottom | anchor_left | anchor_right);
    set_keyboard_interactivity(keyboard_interactivity_on_demand);
    this->window()->waylandSurface()->commit();

    d->fullAnchor = true;
    this->setWindowGeometry(this->window()->geometry());
}

LayerShellSurface::~LayerShellSurface() {
    this->destroy();
    delete d;
}

void LayerShellSurface::setAnchor(quint32 anchor) {
    d->fullAnchor = anchor == (anchor_top | anchor_bottom | anchor_left | anchor_right);
    if (d->fullAnchor) {
        this->setWindowGeometry(QRect());
    } else {
        this->setWindowGeometry(this->window()->geometry());
    }
    set_anchor(anchor);
    this->window()->waylandSurface()->commit();
}

void LayerShellSurface::setExclusiveZone(quint32 exclusiveZone) {
    set_exclusive_zone(exclusiveZone);
    this->window()->waylandSurface()->commit();
}

void LayerShellSurface::setKeyboardInteractivity(quint32 interactivity) {
    set_keyboard_interactivity(interactivity);
    this->window()->waylandSurface()->commit();
}

void LayerShellSurface::setLayer(quint32 layer) {
    set_layer(layer);
    this->window()->waylandSurface()->commit();
}

void LayerShellSurface::zwlr_layer_surface_v1_configure(uint32_t serial, uint32_t width, uint32_t height) {
    ack_configure(serial);
    d->queuedSize = QSize(width, height);

    if (!d->configured) {
        d->configured = true;
        window()->resizeFromApplyConfigure(d->queuedSize);
        window()->handleExpose(QRect(QPoint(0, 0), d->queuedSize));
    } else {
        window()->applyConfigureWhenPossible();
    }
}

void LayerShellSurface::zwlr_layer_surface_v1_closed() {
    if (d->window) d->window->close();
}

void LayerShellSurface::applyConfigure() {
    window()->resizeFromApplyConfigure(d->queuedSize);
}

bool LayerShellSurface::isExposed() const {
    return d->configured;
}

void LayerShellSurface::setWindowGeometry(const QRect& rect) {
    auto newRect = rect;
    if (!d->fullAnchor) {
        if (newRect.width() == 0) newRect.setWidth(1);
        if (newRect.height() == 0) newRect.setHeight(1);
    }
    this->set_size(newRect.width(), newRect.height());
}
