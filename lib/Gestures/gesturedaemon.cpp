#include "gesturedaemon.h"

#include <QDebug>
#include "x11/x11gesturedaemon.h"

struct GestureDaemonPrivate {
    GestureBackend* backend = nullptr;
};

GestureDaemonPrivate* GestureDaemon::d = new GestureDaemonPrivate();

GestureDaemon* GestureDaemon::instance() {
    static GestureDaemon* instance = new GestureDaemon();
    return instance;
}

GestureDaemon::GestureDaemon(QObject* parent) : QObject(parent) {
    //Figure out the best backend to use
#ifdef HAVE_X11
    if (X11GestureDaemon::isSuitable()) {
        d->backend = new X11GestureDaemon();
    }
#endif

    if (d->backend) {
        connect(d->backend, &GestureBackend::gestureBegin, this, &GestureDaemon::gestureBegin);
    } else {
        //No suitable backend is available
        qWarning() << "No suitable backend for GestureDaemon";
    }
}
