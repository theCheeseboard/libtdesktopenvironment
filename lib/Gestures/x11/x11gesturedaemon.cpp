#include "x11gesturedaemon.h"

#include <QDBusConnection>
#include <QX11Info>
#include <QMap>

struct X11GestureDaemonPrivate {
    GestureInteractionPtr currentInteraction;
};

X11GestureDaemon::X11GestureDaemon(QObject* parent) : GestureBackend(parent) {
    d = new X11GestureDaemonPrivate;

    QDBusConnection conn = QDBusConnection::connectToPeer("unix:abstract=touchegg", "touchegg");
    conn.connect("", "/io/github/joseexposito/Touchegg", "io.github.joseexposito.Touchegg", "OnGestureBegin", this, SLOT(gestureBegin(quint32, quint32, double, qint32, quint32, quint64)));
    conn.connect("", "/io/github/joseexposito/Touchegg", "io.github.joseexposito.Touchegg", "OnGestureUpdate", this, SLOT(gestureUpdate(quint32, quint32, double, qint32, quint32, quint64)));
    conn.connect("", "/io/github/joseexposito/Touchegg", "io.github.joseexposito.Touchegg", "OnGestureEnd", this, SLOT(gestureEnd(quint32, quint32, double, qint32, quint32, quint64)));
}

X11GestureDaemon::~X11GestureDaemon() {
    delete d;
}

bool X11GestureDaemon::isSuitable() {
    return QX11Info::isPlatformX11();
}

void X11GestureDaemon::gestureBegin(quint32 type, quint32 direction, double percentage, qint32 fingers, quint32 deviceType, quint64 time) {
    d->currentInteraction = GestureInteractionPtr(new GestureInteraction());
    gestureUpdate(type, direction, percentage, fingers, deviceType, time);

    emit GestureBackend::gestureBegin(d->currentInteraction);
}

void X11GestureDaemon::gestureUpdate(quint32 type, quint32 direction, double percentage, qint32 fingers, quint32 deviceType, quint64 time) {
    const QMap<quint32, GestureTypes::GestureType> types = {
        {1, GestureTypes::Swipe},
        {2, GestureTypes::Pinch}
    };
    const QMap<quint32, GestureTypes::GestureDirection> directions = {
        {1, GestureTypes::Up},
        {2, GestureTypes::Down},
        {3, GestureTypes::Left},
        {4, GestureTypes::Right},
        {5, GestureTypes::In},
        {6, GestureTypes::Out}
    };
    const QMap<quint32, GestureTypes::DeviceType> devices = {
        {1, GestureTypes::Touchpad},
        {2, GestureTypes::TouchScreen}
    };
    updateGesture(d->currentInteraction, types.value(type), directions.value(direction), percentage / 100, fingers, devices.value(deviceType), time);
}

void X11GestureDaemon::gestureEnd(quint32 type, quint32 direction, double percentage, qint32 fingers, quint32 deviceType, quint64 time) {
    endGesture(d->currentInteraction);
}
