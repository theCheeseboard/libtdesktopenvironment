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
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QGuiApplication>
#include <QRandomGenerator64>
#include <QTimer>
#include <QWidget>
#include <qpa/qplatformnativeinterface.h>

#include <wayland-client.h>

#include "layershellwindow.h"
#include <tlogger.h>

#include "twaylandregistry.h"
#include "waylandkeyboardtables.h"
#include "waylandwindow.h"
#include "waylandwmconstants.h"
#include <ranges/trange.h>

struct WaylandBackendPrivate {
        WaylandAccessibility* accessibility;

        tWaylandRegistry registry;
        wl_display* display;
        wl_seat* seat;

        quint64 nextKeygrabId = 0;
        QMap<quint64, quint64> extKeygrab;
        QMap<quint64, quint64> keygrabs;
        QMap<quint64, std::function<void(quint32)>> keygrabFunctions;

        QMap<uint, DesktopWmWindowPtr> windows;
};

WaylandBackend::WaylandBackend() :
    WmBackend() {
    d = new WaylandBackendPrivate();
    d->accessibility = new WaylandAccessibility(this);

    d->display = reinterpret_cast<wl_display*>(qApp->platformNativeInterface()->nativeResourceForIntegration("display"));

    d->seat = d->registry.seat();
    if (!d->registry.init<QtWayland::tdesktopenvironment_keygrab_manager_v1>(this)) {
        tWarn("WaylandBackend") << "The compositor doesn't support the tdesktopenvironment_keygrab_manager_v1 protocol";
    }

    QDBusConnection::sessionBus().connect(WaylandWmConstants::serviceName, WaylandWmConstants::servicePath, "wayland.compositor", "ViewAdded", this, SLOT(viewAdded(uint)));
    QDBusConnection::sessionBus().connect(WaylandWmConstants::serviceName, WaylandWmConstants::servicePath, "wayland.compositor", "ViewRemoved", this, SLOT(viewRemoved(uint)));
    QDBusConnection::sessionBus().connect(WaylandWmConstants::serviceName, WaylandWmConstants::servicePath, "wayland.compositor", "ViewFocusChanged", this, SIGNAL(activeWindowChanged()));

    QDBusInterface outputInterface(WaylandWmConstants::serviceName, WaylandWmConstants::servicePath, "wayland.compositor.output", QDBusConnection::sessionBus(), this);
    auto outputIdsArg = outputInterface.call("QueryOutputIds").arguments().first().value<QDBusArgument>();
    QList<uint> outputIds;
    outputIdsArg >> outputIds;
    for (auto id : outputIds) {
        auto viewsArg = outputInterface.call("QueryOutputViews", id).arguments().first().value<QDBusArgument>();
        QList<uint> viewIds;
        viewsArg >> viewIds;
        for (auto viewId : viewIds) {
            this->viewAdded(viewId);
        }
    }

    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "tdesktopenvironment-layer-shell");
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

void WaylandBackend::viewAdded(uint viewId) {
    DesktopWmWindowPtr window(new WaylandWindow(viewId, this));
    d->windows.insert(viewId, window);
    emit windowAdded(window);
}

void WaylandBackend::viewRemoved(uint viewId) {
    auto window = d->windows.value(viewId);
    if (!window) return;
    emit windowRemoved(window);
    d->windows.remove(viewId);
    window->deleteLater();
}

DesktopAccessibility* WaylandBackend::accessibility() {
    return d->accessibility;
}

QList<DesktopWmWindowPtr> WaylandBackend::openWindows() {
    return d->windows.values();
}

DesktopWmWindowPtr WaylandBackend::activeWindow() {
    for (auto window : d->windows.values()) {
        if (static_cast<WaylandWindow*>(window.data())->isActive()) return window;
    }
    return nullptr;
}

QStringList WaylandBackend::desktops() {
    // TODO: Implement
    return {"Desktop 1", "Desktop 2"};
}

uint WaylandBackend::currentDesktop() {
    // TODO: Implement
    return 0;
}

void WaylandBackend::setCurrentDesktop(uint desktopNumber) {
    // TODO: Implement
}

void WaylandBackend::setNumDesktops(uint numDesktops) {
    // TODO: Implement
}

void WaylandBackend::setShowDesktop(bool showDesktop) {
    // TODO: Implement
}

bool WaylandBackend::supportsSetNumDesktops() {
    return false;
}

void WaylandBackend::setSystemWindow(QWidget* widget) {
    // TODO: Implement
}

void WaylandBackend::setSystemWindow(QWidget* widget, DesktopWm::SystemWindowType windowType) {
    widget->show();
    auto* layerWindow = LayerShellWindow::forWindow(widget->windowHandle());
    if (!layerWindow) return;
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
            layerWindow->setAnchors(static_cast<LayerShellWindow::Anchors>(LayerShellWindow::AnchorLeft | LayerShellWindow::AnchorTop | LayerShellWindow::AnchorBottom));
            layerWindow->setExclusiveZone(0);
            break;
        case DesktopWm::SystemWindowTypeMenu:
            layerWindow->setLayer(LayerShellWindow::Top);
            layerWindow->setAnchors(static_cast<LayerShellWindow::Anchors>(LayerShellWindow::AnchorLeft | LayerShellWindow::AnchorTop | LayerShellWindow::AnchorBottom));
            layerWindow->setExclusiveZone(-1);
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
    // TODO: Implement
}

void WaylandBackend::setScreenMarginForWindow(QWidget* widget, QScreen* screen, Qt::Edge edge, int width) {
    LayerShellWindow* layerWindow = LayerShellWindow::forWindow(widget->windowHandle());
    if (!layerWindow) return;

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
    // TODO: Implement
}

bool WaylandBackend::isScreenOff() {
    // TODO: Implement
    return false;
}

quint64 WaylandBackend::msecsIdle() {
    // TODO: Implement
    return 0;
}

quint64 WaylandBackend::grabKey(Qt::Key key, Qt::KeyboardModifiers modifiers) {
    if (!QtWayland::tdesktopenvironment_keygrab_manager_v1::isInitialized()) return 0;

    quint64 keygrabId = d->nextKeygrabId;
    d->nextKeygrabId++;

    QList<quint32> codes = TWayland::toEvdevCodes(key);
    quint32 mod = TWayland::toEvdevMod(modifiers);
    for (quint32 code : codes) {
        // Create a new keygrab
        quint64 id = QRandomGenerator64::system()->generate();
        while (d->keygrabs.contains(id)) id = QRandomGenerator64::system()->generate();

        d->keygrabs.insert(id, TWayland::evdevDescriptor(mod, code));
        d->keygrabFunctions.insert(id, [this, keygrabId](quint32 type) {
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
    // noop
}

void WaylandBackend::tdesktopenvironment_keygrab_manager_v1_activated(uint32_t mod, uint32_t key, uint32_t type) {
    quint64 descriptor = TWayland::evdevDescriptor(mod, key);
    if (!d->keygrabs.values().contains(descriptor)) return;

    quint64 id = d->keygrabs.key(descriptor);
    d->keygrabFunctions.value(id)(type);
}

QStringList WaylandBackend::availableKeyboardLayouts() {
    // TODO: Implement
    return {"us"};
}

QString WaylandBackend::currentKeyboardLayout() {
    // TODO: Implement
    return "us";
}

QString WaylandBackend::keyboardLayoutDescription(QString layout) {
    // TODO: Implement
    return "US";
}

void WaylandBackend::setCurrentKeyboardLayout(QString layout) {
    // TODO: Implement
}
