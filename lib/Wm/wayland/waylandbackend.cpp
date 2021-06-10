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
#include "waylandbackend.h"

#include "waylandaccessibility.h"
#include <QGuiApplication>
#include <QWidget>
#include <QTimer>
#include <qpa/qplatformnativeinterface.h>

#include <wayland-client.h>
#include "wlr-foreign-toplevel-management-unstable-v1-proto.h"

#include <Shell>
#include <tlogger.h>

#include "waylandwindow.h"

struct WaylandBackendPrivate {
    WaylandAccessibility* accessibility;

    wl_display* display;
    wl_seat* seat;
    zwlr_foreign_toplevel_manager_v1* toplevelManager = nullptr;
    QMap<zwlr_foreign_toplevel_handle_v1*, WaylandWindowPtr> windows;
};

WaylandBackend::WaylandBackend() : WmBackend() {
    d = new WaylandBackendPrivate();
    d->accessibility = new WaylandAccessibility(this);

    LayerShellQt::Shell::useLayerShell();

    d->display = reinterpret_cast<wl_display*>(qApp->platformNativeInterface()->nativeResourceForIntegration("display"));


    wl_registry_listener listener = {
        [](void* data, wl_registry * registry, quint32 name, const char* interface, quint32 version) {
            if (strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
                zwlr_foreign_toplevel_manager_v1* toplevelManager = static_cast<zwlr_foreign_toplevel_manager_v1*>(wl_registry_bind(registry, name, &zwlr_foreign_toplevel_manager_v1_interface, std::min(version, static_cast<quint32>(3))));
                static_cast<WaylandBackendPrivate*>(data)->toplevelManager = toplevelManager;
            } else if (strcmp(interface, wl_seat_interface.name) == 0) {
                wl_seat* seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, std::min(version, static_cast<quint32>(1))));
                static_cast<WaylandBackendPrivate*>(data)->seat = seat;
            }
        },
        [](void* data, wl_registry * registry, quint32 name) {
            Q_UNUSED(data)
            Q_UNUSED(registry)
            Q_UNUSED(name)
        }
    };

    wl_registry* registry = wl_display_get_registry(d->display);
    wl_registry_add_listener(registry, &listener, d);
    wl_display_roundtrip(d->display);

    if (d->toplevelManager) {
        zwlr_foreign_toplevel_manager_v1_listener* toplevelEvents = new zwlr_foreign_toplevel_manager_v1_listener();
        toplevelEvents->toplevel = [](void* data, struct zwlr_foreign_toplevel_manager_v1 * zwlr_foreign_toplevel_manager_v1, struct zwlr_foreign_toplevel_handle_v1 * toplevel) {
            static_cast<WaylandBackend*>(data)->newToplevel(toplevel);
        };
        toplevelEvents->finished = [](void* data, struct zwlr_foreign_toplevel_manager_v1 * zwlr_foreign_toplevel_manager_v1) {

        };

        zwlr_foreign_toplevel_manager_v1_add_listener(d->toplevelManager, toplevelEvents, this);
    } else {
        tWarn("WaylandBackend") << "The compositor doesn't support the wlr-foreign-toplevel-management protocol";
    }
    wl_registry_destroy(registry);
}

bool WaylandBackend::isSuitable() {
    return QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive);
}

QString WaylandBackend::windowSystemName() {
    return QStringLiteral("Wayland");
}

wl_display* WaylandBackend::display() {
    return d->display;
}

wl_seat* WaylandBackend::seat() {
    return d->seat;
}

void WaylandBackend::newToplevel(zwlr_foreign_toplevel_handle_v1* toplevel) {
    WaylandWindowPtr window = new WaylandWindow(toplevel, this);
    d->windows.insert(toplevel, window);

    emit windowAdded(window.data());
}

void WaylandBackend::signalToplevelClosed(zwlr_foreign_toplevel_handle_v1* toplevel) {
    WaylandWindowPtr window = d->windows.value(toplevel);
    emit windowRemoved(window.data());
    window->deleteLater();
    d->windows.remove(toplevel);
}

DesktopAccessibility* WaylandBackend::accessibility() {
    return d->accessibility;
}

QList<DesktopWmWindowPtr> WaylandBackend::openWindows() {
    //TODO: Implement
    return {};
}

DesktopWmWindowPtr WaylandBackend::activeWindow() {
    //TODO: Implement
    for (WaylandWindow* window : d->windows.values()) {
        if (window->isActive()) return window;
    }
    return nullptr;
}

QStringList WaylandBackend::desktops() {
    //TODO: Implement
    return {"Desktop 1", "Desktop 2"};
}

uint WaylandBackend::currentDesktop() {
    //TODO: Implement
    return 0;
}

void WaylandBackend::setCurrentDesktop(uint desktopNumber) {
    //TODO: Implement
}

void WaylandBackend::setNumDesktops(uint numDesktops) {
    //TODO: Implement
}

void WaylandBackend::setShowDesktop(bool showDesktop) {
    //TODO: Implement
}

void WaylandBackend::setSystemWindow(QWidget* widget) {
    //TODO: Implement
}

void WaylandBackend::setSystemWindow(QWidget* widget, DesktopWm::SystemWindowType windowType) {
    //TODO: Implement
    widget->show();
    LayerShellQt::Window* layerWindow = LayerShellQt::Window::get(widget->windowHandle());
    layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);

    switch (windowType) {
        case DesktopWm::SystemWindowTypeSkipTaskbarOnly:
            break;
        case DesktopWm::SystemWindowTypeDesktop:
            layerWindow->setLayer(LayerShellQt::Window::LayerBackground);
//            layerWindow->setAnchors(static_cast<LayerShellQt::Window::Anchors>(LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorBottom | LayerShellQt::Window::AnchorRight | LayerShellQt::Window::AnchorTop));
            break;
        case DesktopWm::SystemWindowTypeTaskbar:
            layerWindow->setLayer(LayerShellQt::Window::LayerTop);
//            layerWindow->setAnchors(LayerShellQt::Window::AnchorTop);
            break;
        case DesktopWm::SystemWindowTypeNotification:
            layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
            break;
        case DesktopWm::SystemWindowTypeMenu:
            layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
            layerWindow->setAnchors(static_cast<LayerShellQt::Window::Anchors>(LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom));
            break;
    }
}

void WaylandBackend::blurWindow(QWidget* widget) {
    //TODO: Implement
}

void WaylandBackend::setScreenMarginForWindow(QWidget* widget, QScreen* screen, Qt::Edge edge, int width) {
    //TODO: Implement

//    QTimer::singleShot(1000, [ = ] {
//    LayerShellQt::Window::get(widget->windowHandle())->setExclusiveZone(width);
//        LayerShellQt::Window::Anchors anchors;
//        switch (edge) {
//            case Qt::TopEdge:
//                anchors = static_cast<LayerShellQt::Window::Anchors>(LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight);
//                break;
//            case Qt::LeftEdge:
//                anchors = static_cast<LayerShellQt::Window::Anchors>(LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom);
//                break;
//            case Qt::RightEdge:
//                anchors = LayerShellQt::Window::AnchorRight;
//                break;
//            case Qt::BottomEdge:
//                anchors = LayerShellQt::Window::AnchorBottom;
//                break;
//        }
//        LayerShellQt::Window::get(widget->windowHandle())->setAnchors(anchors);
//    });

}

void WaylandBackend::setScreenOff(bool screenOff) {
    //TODO: Implement
}

bool WaylandBackend::isScreenOff() {
    //TODO: Implement
    return false;
}

quint64 WaylandBackend::msecsIdle() {
    //TODO: Implement
    return 0;
}

quint64 WaylandBackend::grabKey(Qt::Key key, Qt::KeyboardModifiers modifiers) {
    //TODO: Implement
    return 0;
}

void WaylandBackend::ungrabKey(quint64 grab) {
    //TODO: Implement
}
