#include "x11plugin.h"

#include "Gestures/x11gesturedaemon.h"
#include "Screens/x11screenbackend.h"
#include "Wm/x11backend.h"
#include <tx11info.h>

struct X11PluginPrivate {
        WmBackend* wmBackend;
        ScreenBackend* screenBackend;
        GestureBackend* gestureBackend;
};

X11Plugin::X11Plugin(QObject* parent) :
    QObject{parent} {
    d = new X11PluginPrivate();
}

void X11Plugin::activate() {
    d->wmBackend = new X11Backend();
    d->screenBackend = new X11ScreenBackend();
    d->gestureBackend = new X11GestureDaemon();
}

void X11Plugin::deactivate() {
}

bool X11Plugin::supportedOnThisPlatform() {
    return tX11Info::isPlatformX11();
}

WmBackend* X11Plugin::wmBackend() {
    return d->wmBackend;
}

ScreenBackend* X11Plugin::screenBackend() {
    return d->screenBackend;
}

GestureBackend* X11Plugin::gestureBackend() {
    return d->gestureBackend;
}
