#include "gesturedaemon.h"

#include <QDebug>
#include "TdePlugin/tdepluginmanager.h"
#include "private/gesturebackend.h"

struct GestureDaemonPrivate {
    GestureBackend* backend = nullptr;
};

GestureDaemonPrivate* GestureDaemon::d = new GestureDaemonPrivate();

GestureDaemon* GestureDaemon::instance() {
    static GestureDaemon* instance = new GestureDaemon();
    return instance;
}

GestureDaemon::GestureDaemon(QObject* parent) : QObject(parent) {
    d->backend = TdePluginManager::loadedInterface()->gestureBackend();

    if (d->backend) {
        connect(d->backend, &GestureBackend::gestureBegin, this, &GestureDaemon::gestureBegin);
    } else {
        //No suitable backend is available
        qWarning() << "No suitable backend for GestureDaemon";
    }
}
