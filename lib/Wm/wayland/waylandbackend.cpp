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
#include <QRandomGenerator64>
#include <qpa/qplatformnativeinterface.h>

#include <wayland-client.h>

#include <layershellwindow.h>
#include <tlogger.h>

#include "waylandkeyboardtables.h"
#include "waylandwindow.h"

struct WaylandBackendPrivate {
    WaylandBackend* parent;
    WaylandAccessibility* accessibility;

    wl_display* display;
    wl_seat* seat;
    QMap<zwlr_foreign_toplevel_handle_v1*, WaylandWindowPtr> windows;

    quint64 nextKeygrabId = 0;
    QMap<quint64, quint64> extKeygrab;
    QMap<quint64, quint64> keygrabs;
    QMap<quint64, std::function<void(quint32)>> keygrabFunctions;
};

WaylandBackend::WaylandBackend() : WmBackend() {
    d = new WaylandBackendPrivate();
    d->parent = this;
    d->accessibility = new WaylandAccessibility(this);

//    LayerShellQt::Shell::useLayerShell();
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "tdesktopenvironment-layer-shell");

    d->display = reinterpret_cast<wl_display*>(qApp->platformNativeInterface()->nativeResourceForIntegration("display"));

    wl_registry_listener listener = {
        [](void* data, wl_registry * registry, quint32 name, const char* interface, quint32 version) {
            WaylandBackendPrivate* backend = static_cast<WaylandBackendPrivate*>(data);
            if (strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
                backend->parent->QtWayland::zwlr_foreign_toplevel_manager_v1::init(registry, name, qMin<quint32>(version, 3));
            } else if (strcmp(interface, tdesktopenvironment_keygrab_manager_v1_interface.name) == 0) {
                backend->parent->QtWayland::tdesktopenvironment_keygrab_manager_v1::init(registry, name, 1);
            } else if (strcmp(interface, wl_seat_interface.name) == 0) {
                wl_seat* seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, std::min(version, static_cast<quint32>(1))));
                backend->seat = seat;
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

    if (!this->QtWayland::zwlr_foreign_toplevel_manager_v1::isInitialized()) {
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
    widget->show();
    LayerShellWindow* layerWindow = LayerShellWindow::forWindow(widget->windowHandle());
    layerWindow->setKeyboardInteractivity(LayerShellWindow::OnDemand);

    switch (windowType) {
        case DesktopWm::SystemWindowTypeSkipTaskbarOnly:
            break;
        case DesktopWm::SystemWindowTypeDesktop:
            layerWindow->setLayer(LayerShellWindow::Background);
            layerWindow->setExclusiveZone(-1);
            layerWindow->setKeyboardInteractivity(LayerShellWindow::None);
            break;
        case DesktopWm::SystemWindowTypeTaskbar:
            layerWindow->setLayer(LayerShellWindow::Top);
            break;
        case DesktopWm::SystemWindowTypeNotification:
            layerWindow->setLayer(LayerShellWindow::Overlay);
            layerWindow->setExclusiveZone(0);
            break;
        case DesktopWm::SystemWindowTypeMenu:
            layerWindow->setLayer(LayerShellWindow::Overlay);
            layerWindow->setExclusiveZone(-1);
//            layerWindow->setAnchors(static_cast<LayerShellWindow::Anchors>(LayerShellWindow::AnchorLeft | LayerShellWindow::AnchorTop | LayerShellWindow::AnchorBottom));
            layerWindow->setAnchors(LayerShellWindow::AnchorRight);
            break;
        case DesktopWm::SystemWindowTypeLockScreen:
            layerWindow->setLayer(LayerShellWindow::Overlay);
            layerWindow->setExclusiveZone(-1);
            layerWindow->setAnchors(static_cast<LayerShellWindow::Anchors>(LayerShellWindow::AnchorLeft | LayerShellWindow::AnchorRight | LayerShellWindow::AnchorTop | LayerShellWindow::AnchorBottom));
            layerWindow->setKeyboardInteractivity(LayerShellWindow::Exclusive);
            break;
    }
}

void WaylandBackend::blurWindow(QWidget* widget) {
    //TODO: Implement
}

void WaylandBackend::setScreenMarginForWindow(QWidget* widget, QScreen* screen, Qt::Edge edge, int width) {
    LayerShellWindow* layerWindow = LayerShellWindow::forWindow(widget->windowHandle());

    layerWindow->setExclusiveZone(width);
    LayerShellWindow::Anchors anchors;
    switch (edge) {
        case Qt::TopEdge:
            anchors = LayerShellWindow::AnchorTop;
            break;
        case Qt::LeftEdge:
            anchors = LayerShellWindow::AnchorLeft;
            break;
        case Qt::RightEdge:
            anchors = LayerShellWindow::AnchorRight;
            break;
        case Qt::BottomEdge:
            anchors = LayerShellWindow::AnchorBottom;
            break;
    }
    layerWindow->setAnchors(anchors);
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
    if (!QtWayland::tdesktopenvironment_keygrab_manager_v1::isInitialized()) return 0;

    quint64 keygrabId = d->nextKeygrabId;
    d->nextKeygrabId++;

    QList<quint32> codes = TWayland::toEvdevCodes(key);
    quint32 mod = TWayland::toEvdevMod(modifiers);
    for (quint32 code : codes) {
        //Create a new keygrab
        quint64 id = QRandomGenerator64::system()->generate();
        while (d->keygrabs.contains(id)) id = QRandomGenerator64::system()->generate();

        d->keygrabs.insert(id, TWayland::evdevDescriptor(mod, code));
        d->keygrabFunctions.insert(id, [ = ](quint32 type) {
            emit grabbedKeyPressed(keygrabId);
        });
        QtWayland::tdesktopenvironment_keygrab_manager_v1::grab_key(mod, code);

        d->extKeygrab.insert(id, keygrabId);
    }
    return keygrabId;
}

void WaylandBackend::ungrabKey(quint64 grab) {
    if (!d->extKeygrab.values().contains(grab)) return;
    quint64 id = d->extKeygrab.key(grab);

    quint32 mod, key;
    TWayland::breakoutEvdevDescriptor(d->keygrabs.value(id), &mod, &key);

    QtWayland::tdesktopenvironment_keygrab_manager_v1::ungrab_key(mod, key);

    d->keygrabFunctions.remove(id);
    d->keygrabs.remove(id);
    d->extKeygrab.remove(id);
}

void WaylandBackend::registerAsPrimaryProvider() {
//noop
}

void WaylandBackend::zwlr_foreign_toplevel_manager_v1_toplevel(::zwlr_foreign_toplevel_handle_v1* toplevel) {
    WaylandWindowPtr window = new WaylandWindow(toplevel, this);
    d->windows.insert(toplevel, window);

    emit windowAdded(window.data());
}


void WaylandBackend::tdesktopenvironment_keygrab_manager_v1_activated(uint32_t mod, uint32_t key, uint32_t type) {
    quint64 descriptor = TWayland::evdevDescriptor(mod, key);
    if (!d->keygrabs.values().contains(descriptor)) return;

    quint64 id = d->keygrabs.key(descriptor);
    d->keygrabFunctions.value(id)(type);

}


QStringList WaylandBackend::availableKeyboardLayouts() {
    //TODO: Implement
    return {"us"};
}

QString WaylandBackend::currentKeyboardLayout() {
    //TODO: Implement
    return "us";
}

QString WaylandBackend::keyboardLayoutDescription(QString layout) {
    //TODO: Implement
    return "US";
}

void WaylandBackend::setCurrentKeyboardLayout(QString layout) {
    //TODO: Implement
}
