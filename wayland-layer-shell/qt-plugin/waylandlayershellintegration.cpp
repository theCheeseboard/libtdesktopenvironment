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
#include "waylandlayershellintegration.h"

#include "qwayland-wlr-layer-shell-unstable-v1.h"
#include <private/layershellsurface.h>

WaylandLayerShellIntegration::WaylandLayerShellIntegration() : QtWaylandClient::QWaylandShellIntegration() {

}

bool WaylandLayerShellIntegration::initialize(QtWaylandClient::QWaylandDisplay* display) {
    QWaylandShellIntegration::initialize(display);
    display->addRegistryListener([](void* data, wl_registry * registry, quint32 name, const QString & interface, quint32 version) {
        WaylandLayerShellIntegration* integration = static_cast<WaylandLayerShellIntegration*>(data);
        if (interface == "zwlr_layer_shell_v1") {
            integration->layershellShell = new LayerShellShell(new QtWayland::zwlr_layer_shell_v1(registry, name, version));
        }

    }, this);

    return layershellShell != nullptr;
}

QtWaylandClient::QWaylandShellSurface* WaylandLayerShellIntegration::createShellSurface(QtWaylandClient::QWaylandWindow* window) {
    return new LayerShellSurface(layershellShell, window);
}
