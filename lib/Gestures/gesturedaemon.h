#ifndef GESTUREDAEMON_H
#define GESTUREDAEMON_H

#include <QObject>
#include "gestureinteraction.h"

struct GestureDaemonPrivate;
class GestureDaemon : public QObject {
        Q_OBJECT

    public:
        static GestureDaemon* instance();

    signals:
        void gestureBegin(GestureInteractionPtr interaction);

    private:
        explicit GestureDaemon(QObject* parent = nullptr);
        static GestureDaemonPrivate* d;
};

#endif // GESTUREDAEMON_H
