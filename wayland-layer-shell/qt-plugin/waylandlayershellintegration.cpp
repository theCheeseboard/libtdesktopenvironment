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
#include <layershellwindow.h>
#include <private/layershellsurface.h>
#include <private/qwaylanddisplay_p.h>
#include <private/qwaylandshellintegrationfactory_p.h>
#include <private/qwaylandwindow_p.h>
#include <tlogger.h>

WaylandLayerShellIntegration::WaylandLayerShellIntegration() :
    QtWaylandClient::QWaylandShellIntegration() {
}

bool WaylandLayerShellIntegration::initialize(QtWaylandClient::QWaylandDisplay* display) {
    display->addRegistryListener([](void* data, wl_registry* registry, quint32 name, const QString& interface, quint32 version) {
        WaylandLayerShellIntegration* integration = static_cast<WaylandLayerShellIntegration*>(data);
        if (interface == "zwlr_layer_shell_v1") {
            integration->layershellShell = new LayerShellShell(new QtWayland::zwlr_layer_shell_v1(registry, name, version));
        }
    },
        this);

    xdgShellIntegration = QtWaylandClient::QWaylandShellIntegrationFactory::create("xdg-shell", display);
    return layershellShell != nullptr;
}

QtWaylandClient::QWaylandShellSurface* WaylandLayerShellIntegration::createShellSurface(QtWaylandClient::QWaylandWindow* window) {
    if (layershellShell && shouldBeLayerShell(window)) {
        return new LayerShellSurface(layershellShell, window);
    }

    auto parent = window->transientParent();
    if (!parent) {
        tWarn("WaylandLayerShellIntegration") << "Found a popup window with no parent. Creating as layer-shell window.";
        return new LayerShellSurface(layershellShell, window);
    }
    auto shellSurface = xdgShellIntegration->createShellSurface(window);
    auto layerShell = LayerShellWindow::forWindow(parent->window());
    if (layerShell) layerShell->getPopup(shellSurface->surfaceRole());
    return shellSurface;
}

bool WaylandLayerShellIntegration::shouldBeLayerShell(QtWaylandClient::QWaylandWindow* window) {
    auto windowType = window->window()->type();
    return !(windowType == Qt::Popup || windowType == Qt::ToolTip);
}
