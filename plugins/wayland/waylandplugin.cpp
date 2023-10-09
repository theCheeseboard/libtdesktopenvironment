#include "waylandplugin.h"

//#include "Gestures/x11gesturedaemon.h"
#include "Screens/waylandscreenbackend.h"
#include "Wm/waylandbackend.h"
#include <QGuiApplication>

struct WaylandPluginPrivate {
        WmBackend* wmBackend;
        ScreenBackend* screenBackend;
        GestureBackend* gestureBackend = nullptr;
};

WaylandPlugin::WaylandPlugin(QObject* parent) :
    QObject{parent} {
    d = new WaylandPluginPrivate();
}

void WaylandPlugin::activate() {
    d->wmBackend = new WaylandBackend();
    d->screenBackend = new WaylandScreenBackend();
//    d->gestureBackend = new X11GestureDaemon();
}

void WaylandPlugin::deactivate() {
}

bool WaylandPlugin::supportedOnThisPlatform() {
    return QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive);
}

WmBackend* WaylandPlugin::wmBackend() {
    return d->wmBackend;
}

ScreenBackend* WaylandPlugin::screenBackend() {
    return d->screenBackend;
}

GestureBackend* WaylandPlugin::gestureBackend() {
    return d->gestureBackend;
}
