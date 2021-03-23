#ifndef GESTUREBACKEND_H
#define GESTUREBACKEND_H

#include <QObject>

#include "../gestureinteraction.h"

class GestureBackend : public QObject {
        Q_OBJECT
    public:
        explicit GestureBackend(QObject* parent = nullptr);

        static void updateGesture(GestureInteractionPtr interaction, GestureTypes::GestureType type, GestureTypes::GestureDirection direction, double percentage, int fingers, GestureTypes::DeviceType deviceType, quint64 time);
        static void endGesture(GestureInteractionPtr interaction);

    signals:
        void gestureBegin(GestureInteractionPtr interaction);

};

#endif // GESTUREBACKEND_H
