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
#ifndef LAYERSHELLSURFACE_H
#define LAYERSHELLSURFACE_H

#include <QObject>
#include <private/qwaylandshellsurface_p.h>
#include "qwayland-wlr-layer-shell-unstable-v1.h"

struct LayerShellSurfacePrivate;

class LayerShellShell;
class LayerShellSurface : public QtWaylandClient::QWaylandShellSurface, public QtWayland::zwlr_layer_surface_v1 {
        Q_OBJECT
    public:
        explicit LayerShellSurface(LayerShellShell* shell, QtWaylandClient::QWaylandWindow* window);
        ~LayerShellSurface();

        void setAnchor(quint32 anchor);
        void setExclusiveZone(quint32 exclusiveZone);
        void setKeyboardInteractivity(quint32 interactivity);
        void setLayer(quint32 layer);

    signals:

    private:
        LayerShellSurfacePrivate* d;


        // zwlr_layer_surface_v1 interface
    protected:
        void zwlr_layer_surface_v1_configure(uint32_t serial, uint32_t width, uint32_t height);
        void zwlr_layer_surface_v1_closed();

        // QWaylandShellSurface interface
    public:
        void applyConfigure();
        bool isExposed() const;
        void setWindowGeometry(const QRect& rect);
};

#endif // LAYERSHELLSURFACE_H
