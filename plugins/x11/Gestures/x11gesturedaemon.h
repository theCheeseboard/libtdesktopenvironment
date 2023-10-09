#ifndef X11GESTUREDAEMON_H
#define X11GESTUREDAEMON_H

#include "Gestures/private/gesturebackend.h"

struct X11GestureDaemonPrivate;
class X11GestureDaemon : public GestureBackend {
        Q_OBJECT
    public:
        explicit X11GestureDaemon(QObject* parent = nullptr);
        ~X11GestureDaemon();

        static bool isSuitable();

    signals:

    public slots:
        void gestureBegin(quint32 type, quint32 direction, double percentage, qint32 fingers, quint32 deviceType, quint64 time);
        void gestureUpdate(quint32 type, quint32 direction, double percentage, qint32 fingers, quint32 deviceType, quint64 time);
        void gestureEnd(quint32 type, quint32 direction, double percentage, qint32 fingers, quint32 deviceType, quint64 time);

    private:
        X11GestureDaemonPrivate* d;
};

#endif // X11GESTUREDAEMON_H
