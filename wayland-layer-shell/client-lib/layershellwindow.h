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
#ifndef LAYERSHELLWINDOW_H
#define LAYERSHELLWINDOW_H

#include <QObject>
#include <QWindow>

#include "client-lib_global.h"

class LayerShellSurface;
struct LayerShellWindowPrivate;
class CLIENTLIB_EXPORT LayerShellWindow : public QObject {
        Q_OBJECT
    public:
        static LayerShellWindow* forWindow(QWindow* window);
        ~LayerShellWindow();

        enum Layer : quint32 {
            Overlay = 3,
            Top = 2,
            Bottom = 1,
            Background = 0
        };

        enum Anchor : quint32 {
            AnchorTop = 1,
            AnchorBottom = 2,
            AnchorLeft = 4,
            AnchorRight = 8
        };
        Q_DECLARE_FLAGS(Anchors, Anchor);

        enum KeyboardInteractivity : quint32 {
            None = 0,
            Exclusive = 1,
            OnDemand = 2
        };

        void setLayer(Layer layer);
        void setExclusiveZone(quint32 exclusiveZone);
        void setAnchors(Anchors anchors);
        void setKeyboardInteractivity(KeyboardInteractivity interactivity);

    signals:

    private:
        explicit LayerShellWindow(LayerShellSurface* surface);

        LayerShellWindowPrivate* d;
};

#endif // LAYERSHELLWINDOW_H
